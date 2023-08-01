#ifndef GBEMU_RAM_H
#define GBEMU_RAM_H

#include "utils.h"

uint8_t readWRAM(uint16_t address);
void writeWRAM(uint16_t address, uint8_t value);

uint8_t readHRAM(uint16_t address);
void writeHRAM(uint16_t address, uint8_t value);

#endif //GBEMU_RAM_H
