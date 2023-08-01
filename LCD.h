#ifndef GBEMU_LCD_H
#define GBEMU_LCD_H

#include "utils.h"

typedef struct {
    //registers
    uint8_t lcdc;
    uint8_t lcds;
    uint8_t scrollY;
    uint8_t scrollX;
    uint8_t ly;
    uint8_t lyCompare;
    uint8_t dma;
    uint8_t bgPalette;
    uint8_t objPalette[2];
    uint8_t winY;
    uint8_t winX;

    uint32_t bgColors[4];
    uint32_t sp1Colors[4];
    uint32_t sp2Colors[4];

} LCDState;

typedef enum {
    MODE_HBLANK,
    MODE_VBLANK,
    MODE_OAM,
    MODE_XFER
} LCDMode;

LCDState* getLCDState();

#define LCDC_BGW_ENABLE (IS_BIT_SET(getLCDState()->lcdc, 0))
#define LCDC_OBJ_ENABLE (IS_BIT_SET(getLCDState()->lcdc, 1))
#define LCDC_OBJ_HEIGHT (IS_BIT_SET(getLCDState()->lcdc, 2) ? 16 : 8)
#define LCDC_BG_MAP_AREA (IS_BIT_SET(getLCDState()->lcdc, 3) ? 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA (IS_BIT_SET(getLCDState()->lcdc, 4) ? 0x8000 : 0x8800)
#define LCDC_WIN_ENABLE (IS_BIT_SET(getLCDState()->lcdc, 5))
#define LCDC_WIN_MAP_AREA (IS_BIT_SET(getLCDState()->lcdc, 6) ? 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE (IS_BIT_SET(getLCDState()->lcdc, 7))
#define LCDS_MODE ((LCDMode)(getLCDState()->lcds & 0b11))
#define LCDS_MODE_SET(mode) { getLCDState()->lcds &= ~0b11; getLCDState()->lcds |= mode; }
#define LCDS_LYC (IS_BIT_SET(getLCDState()->lcds, 2))
#define LCDS_LYC_SET(b) (SET_OR_CLEAR_BIT(getLCDState()->lcds, 2, b))

typedef enum {
    SS_HBLANK = (1 << 3),
    SS_VBLANK = (1 << 4),
    SS_OAM = (1 << 5),
    SS_LYC = (1 << 6),
} stat_src;

#define LCDS_STAT_INT(src) (getLCDState()->lcds & src)

void initLCD();

uint8_t readLCD(uint16_t address);
void writeLCD(uint16_t address, uint8_t value);

#endif //GBEMU_LCD_H
