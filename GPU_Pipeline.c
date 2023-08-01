#include "GPU.h"
#include "LCD.h"
#include "MemoryBus.h"


bool isWindowVisible() {
    return LCDC_WIN_ENABLE && getLCDState()->winX >= 0 &&
            getLCDState()->winX <= 166 && getLCDState()->winY >= 0 &&
            getLCDState()->winY < YRES;
}

void pushPixelFIFO(uint32_t value) {
    FifoEntry* next = malloc(sizeof(FifoEntry));
    next->next = NULL;
    next->value = value;

    if (!getGPUState()->pixelFifoState.pixelFifo.head) {
        //first entry...
        getGPUState()->pixelFifoState.pixelFifo.head = getGPUState()->pixelFifoState.pixelFifo.tail = next;
    } else {
        getGPUState()->pixelFifoState.pixelFifo.tail->next = next;
        getGPUState()->pixelFifoState.pixelFifo.tail = next;
    }

    getGPUState()->pixelFifoState.pixelFifo.size++;
}

uint32_t popPixelFIFO() {
    if (getGPUState()->pixelFifoState.pixelFifo.size <= 0) {
        fprintf(stderr, "!!!ERROR IN PIXEL FIFO!\n");
        exit(-8);
    }

    FifoEntry* popped = getGPUState()->pixelFifoState.pixelFifo.head;
    getGPUState()->pixelFifoState.pixelFifo.head = popped->next;
    getGPUState()->pixelFifoState.pixelFifo.size--;

    uint32_t val = popped->value;
    free(popped);

    return val;
}

uint32_t fetchSpritePixels(int bit, uint32_t color, uint8_t bgColor) {
    for (int i=0; i < getGPUState()->fetchedEntryCount; i++) {
        int sp_x = (getGPUState()->fetchedEntries[i].x - 8) +
                   ((getLCDState()->scrollX % 8));

        if (sp_x + 8 < getGPUState()->pixelFifoState.fifoX) {
            //past pixel point already
            continue;
        }

        int offset = getGPUState()->pixelFifoState.fifoX - sp_x;

        if (offset < 0 || offset > 7) {
            //out of bounds
            continue;
        }

        bit = (7 - offset);

        if (getGPUState()->fetchedEntries[i].flagXFlip) {
            bit = offset;
        }

        uint8_t hi = !!(getGPUState()->pixelFifoState.fetchEntryData[i * 2] & (1 << bit));
        uint8_t lo = !!(getGPUState()->pixelFifoState.fetchEntryData[(i * 2) + 1] & (1 << bit)) << 1;

        bool bg_priority = getGPUState()->fetchedEntries[i].flagBgPriority;

        if (!(hi|lo)) {
            //transparent
            continue;
        }

        if (!bg_priority || bgColor == 0) {
            color = (getGPUState()->fetchedEntries[i].flagPaletteNumber) ?
                    getLCDState()->sp2Colors[hi | lo] : getLCDState()->sp1Colors[hi | lo];

            if (hi|lo) {
                break;
            }
        }
    }

    return color;
}

bool canAddToFifo() {
    if (getGPUState()->pixelFifoState.pixelFifo.size > 8) {
        //Fifo is full
        return false;
    }

    int x = getGPUState()->pixelFifoState.fetchX - (8 - (getLCDState()->scrollX % 8));

    for (int i=0; i<8; i++) {
        int bit = 7 - i;
        uint8_t hi = !!(getGPUState()->pixelFifoState.bgwFetchData[1] & (1 << bit));
        uint8_t lo = !!(getGPUState()->pixelFifoState.bgwFetchData[2] & (1 << bit)) << 1;
        uint32_t color = getLCDState()->bgColors[hi | lo];

        if (!LCDC_BGW_ENABLE) {
            color = getLCDState()->bgColors[0];
        }

        if (LCDC_OBJ_ENABLE) {
            color = fetchSpritePixels(bit, color, hi | lo);
        }

        if (x >= 0) {
            pushPixelFIFO(color);
            getGPUState()->pixelFifoState.fifoX++;
        }
    }

    return true;
}

void loadSpriteTile() {
    OAM_lineEntry* lineEntry = getGPUState()->lineSprites;

    while(lineEntry) {
        int sp_x = (lineEntry->entry.x - 8) + (getLCDState()->scrollX % 8);

        if ((sp_x >= getGPUState()->pixelFifoState.fetchX && sp_x < getGPUState()->pixelFifoState.fetchX + 8) ||
            ((sp_x + 8) >= getGPUState()->pixelFifoState.fetchX && (sp_x + 8) < getGPUState()->pixelFifoState.fetchX + 8)) {
            //need to add entry
            getGPUState()->fetchedEntries[getGPUState()->fetchedEntryCount++] = lineEntry->entry;
        }

        lineEntry = lineEntry->next;

        if (!lineEntry || getGPUState()->fetchedEntryCount >= 3) {
            //max checking 3 sprites on pixels
            break;
        }
    }
}

void loadSpriteData(uint8_t offset) {
    int cur_y = getLCDState()->ly;
    uint8_t sprite_height = LCDC_OBJ_HEIGHT;

    for (int i=0; i < getGPUState()->fetchedEntryCount; i++) {
        uint8_t ty = ((cur_y + 16) - getGPUState()->fetchedEntries[i].y) * 2;

        if (getGPUState()->fetchedEntries[i].flagYFlip) {
            //flipped upside down
            ty = ((sprite_height * 2) - 2) - ty;
        }

        uint8_t tile_index = getGPUState()->fetchedEntries[i].tile;

        if (sprite_height == 16) {
            tile_index &= ~(1); //remove last bit
        }

        getGPUState()->pixelFifoState.fetchEntryData[(i * 2) + offset] =
                ReadByteMemBus(0x8000 + (tile_index * 16) + ty + offset);
    }
}

void loadWindowTile() {
    if (!isWindowVisible()) {
        return;
    }

    uint8_t window_y = getLCDState()->winY;

    if (getGPUState()->pixelFifoState.fetchX + 7 >= getLCDState()->winX &&
        getGPUState()->pixelFifoState.fetchX + 7 < getLCDState()->winX + YRES + 14) {
        if (getLCDState()->ly >= window_y && getLCDState()->ly < window_y + XRES) {
            uint8_t w_tile_y = getGPUState()->windowLine / 8;

            getGPUState()->pixelFifoState.bgwFetchData[0] = ReadByteMemBus(LCDC_WIN_MAP_AREA +
                                                                           ((getGPUState()->pixelFifoState.fetchX + 7 -
                                                                                   getLCDState()->winX) / 8) +
                                                                           (w_tile_y * 32));

            if (LCDC_BGW_DATA_AREA == 0x8800) {
                getGPUState()->pixelFifoState.bgwFetchData[0] += 128;
            }
        }
    }
}

void pipelineFetch() {
    switch(getGPUState()->pixelFifoState.currFetchState) {
        case FS_TILE: {
            getGPUState()->fetchedEntryCount = 0;

            if (LCDC_BGW_ENABLE) {
                getGPUState()->pixelFifoState.bgwFetchData[0] = ReadByteMemBus(LCDC_BG_MAP_AREA +
                                                                               (getGPUState()->pixelFifoState.mapX / 8) +
                                                                               (((getGPUState()->pixelFifoState.mapY / 8)) * 32));

                if (LCDC_BGW_DATA_AREA == 0x8800) {
                    getGPUState()->pixelFifoState.bgwFetchData[0] += 128;
                }

                loadWindowTile();
            }

            if (LCDC_OBJ_ENABLE && getGPUState()->lineSprites) {
                loadSpriteTile();
            }

            getGPUState()->pixelFifoState.currFetchState = FS_DATA0;
            getGPUState()->pixelFifoState.fetchX += 8;
        } break;

        case FS_DATA0: {
            getGPUState()->pixelFifoState.bgwFetchData[1] = ReadByteMemBus(LCDC_BGW_DATA_AREA +
                                                                           (getGPUState()->pixelFifoState.bgwFetchData[0] * 16) +
                                                                           getGPUState()->pixelFifoState.tileY);

            loadSpriteData(0);

            getGPUState()->pixelFifoState.currFetchState = FS_DATA1;
        } break;

        case FS_DATA1: {
            getGPUState()->pixelFifoState.bgwFetchData[2] = ReadByteMemBus(LCDC_BGW_DATA_AREA +
                                                                           (getGPUState()->pixelFifoState.bgwFetchData[0] * 16) +
                                                                           getGPUState()->pixelFifoState.tileY + 1);

            loadSpriteData(1);

            getGPUState()->pixelFifoState.currFetchState = FS_IDLE;

        } break;

        case FS_IDLE: {
            getGPUState()->pixelFifoState.currFetchState = FS_PUSH;
        } break;

        case FS_PUSH: {
            if (canAddToFifo()) {
                getGPUState()->pixelFifoState.currFetchState = FS_TILE;
            }

        } break;

    }
}

void pipelinePushPixel() {
    if (getGPUState()->pixelFifoState.pixelFifo.size > 8) {
        uint32_t pixel_data = popPixelFIFO();

        if (getGPUState()->pixelFifoState.lineX >= (getLCDState()->scrollX % 8)) {
            getGPUState()->videoBuffer[getGPUState()->pixelFifoState.pushedX +
                                       (getLCDState()->ly * XRES)] = pixel_data;

            getGPUState()->pixelFifoState.pushedX++;
        }

        getGPUState()->pixelFifoState.lineX++;
    }
}

void processPipeline() {
    getGPUState()->pixelFifoState.mapY = (getLCDState()->ly + getLCDState()->scrollY);
    getGPUState()->pixelFifoState.mapX = (getGPUState()->pixelFifoState.fetchX + getLCDState()->scrollX);
    getGPUState()->pixelFifoState.tileY = ((getLCDState()->ly + getLCDState()->scrollY) % 8) * 2;

    if (!(getGPUState()->lineTicks & 1)) {
        pipelineFetch();
    }

    pipelinePushPixel();
}

void pipelineResetFIFO() {
    while(getGPUState()->pixelFifoState.pixelFifo.size) {
        popPixelFIFO();
    }

    getGPUState()->pixelFifoState.pixelFifo.head = 0;
}