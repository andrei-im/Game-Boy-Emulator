#include "GPU.h"
#include "LCD.h"
#include "Interrupts.h"
#include <string.h>
#include "Cartridge.h"



void pipelineResetFIFO();
void processPipeline();
bool isWindowVisible();

void incrementLY() {
    if (isWindowVisible() && getLCDState()->ly >= getLCDState()->winY &&
        getLCDState()->ly < getLCDState()->winY + YRES) {
        getGPUState()->windowLine++;
    }

    getLCDState()->ly++;

    if (getLCDState()->ly == getLCDState()->lyCompare) {
        LCDS_LYC_SET(1);

        if (LCDS_STAT_INT(SS_LYC)) {
            cpuRequestInterrupt(IT_LCD_STAT);
        }
    } else {
        LCDS_LYC_SET(0);
    }
}

void loadLineSprites() {
    int cur_y = getLCDState()->ly;

    uint8_t sprite_height = LCDC_OBJ_HEIGHT;
    memset(getGPUState()->lineEntryArray, 0,
           sizeof(getGPUState()->lineEntryArray));

    for (int i=0; i<40; i++) {
        OAM_entry e = getGPUState()->OAM_ram[i];

        if (!e.x) {
            //x = 0 means not visible
            continue;
        }

        if (getGPUState()->lineSpriteCount >= 10) {
            //max 10 sprites per line
            break;
        }

        if (e.y <= cur_y + 16 && e.y + sprite_height > cur_y + 16) {
            //this sprite is on the current line.

            OAM_lineEntry *entry = &getGPUState()->lineEntryArray[
                    getGPUState()->lineSpriteCount++
            ];

            entry->entry = e;
            entry->next = NULL;

            if (!getGPUState()->lineSprites ||
                getGPUState()->lineSprites->entry.x > e.x) {
                entry->next = getGPUState()->lineSprites;
                getGPUState()->lineSprites = entry;
                continue;
            }

            //sort
            OAM_lineEntry *le = getGPUState()->lineSprites;
            OAM_lineEntry *prev = le;

            while(le) {
                if (le->entry.x > e.x) {
                    prev->next = entry;
                    entry->next = le;
                    break;
                }

                if (!le->next) {
                    le->next = entry;
                    break;
                }

                prev = le;
                le = le->next;
            }
        }
    }
}

void gpuMode_OAM() {
    if (getGPUState()->lineTicks >= 80) {
        LCDS_MODE_SET(MODE_XFER);

        getGPUState()->pixelFifoState.currFetchState = FS_TILE;
        getGPUState()->pixelFifoState.lineX = 0;
        getGPUState()->pixelFifoState.fetchX = 0;
        getGPUState()->pixelFifoState.pushedX = 0;
        getGPUState()->pixelFifoState.fifoX = 0;
    }

    if (getGPUState()->lineTicks == 1) {
        //read oam only on the first tick
        getGPUState()->lineSprites = 0;
        getGPUState()->lineSpriteCount = 0;
        loadLineSprites();
    }
}

void gpuMode_XFER() {
    processPipeline();

    if (getGPUState()->pixelFifoState.pushedX >= XRES) {
        pipelineResetFIFO();

        LCDS_MODE_SET(MODE_HBLANK);

        if (LCDS_STAT_INT(SS_HBLANK)) {
            cpuRequestInterrupt(IT_LCD_STAT);
        }
    }
}

void gpuMode_VBLANK() {
    if (getGPUState()->lineTicks >= TICKS_PER_LINE) {
        incrementLY();

        if (getLCDState()->ly >= LINES_PER_FRAME) {
            LCDS_MODE_SET(MODE_OAM);
            getLCDState()->ly = 0;
            getGPUState()->windowLine = 0;
        }

        getGPUState()->lineTicks = 0;
    }
}

static uint32_t targetFrameTime = 1000 / 60;
static long prevFrameTime = 0;
static long staRegisterType_timer = 0;
static long frameCount = 0;

void gpuMode_HBLANK() {
    if (getGPUState()->lineTicks >= TICKS_PER_LINE) {
        incrementLY();

        if (getLCDState()->ly >= YRES) {
            LCDS_MODE_SET(MODE_VBLANK);

            cpuRequestInterrupt(IT_VBLANK);

            if (LCDS_STAT_INT(SS_VBLANK)) {
                cpuRequestInterrupt(IT_LCD_STAT);
            }

            getGPUState()->currentFrame++;

            //calculate the FPS
            uint32_t end = getTicks();
            uint32_t frame_time = end - prevFrameTime;

            if (frame_time < targetFrameTime) {
                delay((targetFrameTime - frame_time));
            }

            if (end - staRegisterType_timer >= 1000) {
                uint32_t fps = frameCount;
                staRegisterType_timer = end;
                frameCount = 0;

                updateWindowTitle(fps);

                if (needSaveCartridge()) {
                    batterySaveCartridge();
                }
            }

            frameCount++;
            prevFrameTime = getTicks();

        } else {
            LCDS_MODE_SET(MODE_OAM);
        }

        getGPUState()->lineTicks = 0;
    }
}