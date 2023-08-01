#ifndef GBEMU_MEMORYBUS_H
#define GBEMU_MEMORYBUS_H

#include "utils.h"

uint8_t ReadByteMemBus(uint16_t address);
void WriteByteMemBus(uint16_t address, uint8_t value);

uint16_t ReadWordMemBus(uint16_t address);
void WriteWordMemBus(uint16_t address, uint16_t value);

#endif //GBEMU_MEMORYBUS_H
