#include "GPU.h"
#include "LCD.h"
#include <string.h>
#include "GPU_StateMachine.h"


void pipelineResetFIFO();
void processPipeline();

static GPUState gpuState;

GPUState* getGPUState() {
    return &gpuState;
}

void initGPU() {
    gpuState.currentFrame = 0;
    gpuState.lineTicks = 0;
    gpuState.videoBuffer = malloc(YRES * XRES * sizeof(32));

    gpuState.pixelFifoState.lineX = 0;
    gpuState.pixelFifoState.pushedX = 0;
    gpuState.pixelFifoState.fetchX = 0;
    gpuState.pixelFifoState.pixelFifo.size = 0;
    gpuState.pixelFifoState.pixelFifo.head = gpuState.pixelFifoState.pixelFifo.tail = NULL;
    gpuState.pixelFifoState.currFetchState = FS_TILE;

    gpuState.lineSprites = 0;
    gpuState.fetchedEntryCount = 0;
    gpuState.windowLine = 0;

    initLCD();
    LCDS_MODE_SET(MODE_OAM);

    memset(gpuState.OAM_ram, 0, sizeof(gpuState.OAM_ram));
    memset(gpuState.videoBuffer, 0, YRES * XRES * sizeof(uint32_t));
}

void gpuTick() {
    gpuState.lineTicks++;

    switch(LCDS_MODE) {
        case MODE_OAM:
            gpuMode_OAM();
            break;
        case MODE_XFER:
            gpuMode_XFER();
            break;
        case MODE_VBLANK:
            gpuMode_VBLANK();
            break;
        case MODE_HBLANK:
            gpuMode_HBLANK();
            break;
    }
}


void writeOAM(uint16_t address, uint8_t value) {
    if (address >= 0xFE00) {
        address -= 0xFE00;
    }

    uint8_t* p = (uint8_t *)gpuState.OAM_ram;
    p[address] = value;
}

uint8_t readOAM(uint16_t address) {
    if (address >= 0xFE00) {
        address -= 0xFE00;
    }

    uint8_t* p = (uint8_t *)gpuState.OAM_ram;
    return p[address];
}

void writeVRAM(uint16_t address, uint8_t value) {
    gpuState.VRAM[address - 0x8000] = value;
}

uint8_t readVRAM(uint16_t address) {
    return gpuState.VRAM[address - 0x8000];
}