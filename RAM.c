#include "RAM.h"


typedef struct {
    uint8_t wram[0x2000];
    uint8_t hram[0x80];
} RAMState;

static RAMState ramState;

uint8_t readWRAM(uint16_t address) {
    address -= 0xC000;

    if (address >= 0x2000) {
        printf("INVALID WRAM ADDRESS %08X\n", address + 0xC000);
        exit(-1);
    }
    return ramState.wram[address];
}

void writeWRAM(uint16_t address, uint8_t value) {
    address -= 0xC000;
    ramState.wram[address] = value;
}

uint8_t readHRAM(uint16_t address) {
    address -= 0xFF80;
    return ramState.hram[address];
}

void writeHRAM(uint16_t address, uint8_t value) {
    address -= 0xFF80;
    ramState.hram[address] = value;
}