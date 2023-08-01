#include "LCD.h"
#include "GPU.h"
#include "DirectMemoryAccess.h"


static LCDState lcdState;

static unsigned long colors_default[4] = {
        0xFF9bbc0f, // lightest shade
        0xFF8bac0f, // light shade
        0xFF306230, // dark shade
        0xFF0F380F  // darkest shade
};


void initLCD() {
    lcdState.lcdc = 0x91;
    lcdState.scrollX = 0;
    lcdState.scrollY = 0;
    lcdState.ly = 0;
    lcdState.lyCompare = 0;
    lcdState.bgPalette = 0xFC;
    lcdState.objPalette[0] = 0xFF;
    lcdState.objPalette[1] = 0xFF;
    lcdState.winY = 0;
    lcdState.winX = 0;

    for (int i=0; i<4; i++) {
        lcdState.bgColors[i] = colors_default[i];
        lcdState.sp1Colors[i] = colors_default[i];
        lcdState.sp2Colors[i] = colors_default[i];
    }
}

LCDState* getLCDState() {
    return &lcdState;
}

uint8_t readLCD(uint16_t address) {
    uint8_t offset = (address - 0xFF40);
    uint8_t *p = (uint8_t *)&lcdState;

    return p[offset];
}

void updatePalette(uint8_t palette_data, uint8_t pal) {
    uint32_t *p_colors = lcdState.bgColors;

    switch(pal) {
        case 1:
            p_colors = lcdState.sp1Colors;
            break;
        case 2:
            p_colors = lcdState.sp2Colors;
            break;
    }

    p_colors[0] = colors_default[palette_data & 0b11];
    p_colors[1] = colors_default[(palette_data >> 2) & 0b11];
    p_colors[2] = colors_default[(palette_data >> 4) & 0b11];
    p_colors[3] = colors_default[(palette_data >> 6) & 0b11];
}

void writeLCD(uint16_t address, uint8_t value) {

    uint8_t offset = (address - 0xFF40);
    uint8_t *p = (uint8_t *)&lcdState;
    p[offset] = value;

    if (offset == 6) {
        //0xFF46 = DMA
        startDMA(value);
    }

    if (address == 0xFF47) {
        updatePalette(value, 0);
    } else if (address == 0xFF48) {
        updatePalette(value & 0b11111100, 1);
    } else if (address == 0xFF49) {
        updatePalette(value & 0b11111100, 2);
    }
}