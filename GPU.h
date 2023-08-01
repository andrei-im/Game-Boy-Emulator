#ifndef GBEMU_GPU_H
#define GBEMU_GPU_H

#include "utils.h"

static const int LINES_PER_FRAME = 154;
static const int TICKS_PER_LINE = 456;
static const int YRES = 144;
static const int XRES = 160;

typedef enum {
    FS_TILE,
    FS_DATA0,
    FS_DATA1,
    FS_IDLE,
    FS_PUSH
} FetchState;

typedef struct _FifoEntry {
    struct _FifoEntry* next;
    uint32_t value; //32 bit color value
} FifoEntry;

typedef struct {
    FifoEntry* head;
    FifoEntry* tail;
    uint32_t size;
} Fifo;

typedef struct {
    FetchState currFetchState;
    Fifo pixelFifo;
    uint8_t lineX;
    uint8_t pushedX;
    uint8_t fetchX;
    uint8_t bgwFetchData[3];
    uint8_t fetchEntryData[6]; //oam data
    uint8_t mapY;
    uint8_t mapX;
    uint8_t tileY;
    uint8_t fifoX;
} PixelFifoState;

typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile;

    uint8_t flagPaletteNumber : 1;
    uint8_t flagXFlip : 1;
    uint8_t flagYFlip : 1;
    uint8_t flagBgPriority : 1;

} OAM_entry;


typedef struct _OAM_lineEntry {
    OAM_entry entry;
    struct _OAM_lineEntry* next;
} OAM_lineEntry;

typedef struct {
    OAM_entry OAM_ram[40];
    uint8_t VRAM[0x2000];

    PixelFifoState pixelFifoState;

    uint8_t lineSpriteCount; //0 to 10 sprites.
    OAM_lineEntry* lineSprites; //linked list of current sprites on line.
    OAM_lineEntry lineEntryArray[10]; //memory to use for list.

    uint8_t fetchedEntryCount;
    OAM_entry fetchedEntries[3]; //entries fetched during pipeline.
    uint8_t windowLine;

    uint32_t currentFrame;
    uint32_t lineTicks;
    uint32_t* videoBuffer;
} GPUState;

void initGPU();
void gpuTick();

void updateWindowTitle(uint32_t fps);

void writeOAM(uint16_t address, uint8_t value);
uint8_t readOAM(uint16_t address);

void writeVRAM(uint16_t address, uint8_t value);
uint8_t readVRAM(uint16_t address);

GPUState* getGPUState();

#endif //GBEMU_GPU_H
