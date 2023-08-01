#include "Stack.h"
#include "CPU.h"
#include "MemoryBus.h"


void pushStack(uint8_t data) {
    getCPURegisters()->SP--;
    WriteByteMemBus(getCPURegisters()->SP, data);
}

void pushStack16(uint16_t data) {
    pushStack((data >> 8) & 0xFF);
    pushStack(data & 0xFF);
}

uint8_t popStack() {
    return ReadByteMemBus(getCPURegisters()->SP++);
}

uint16_t popStack16() {
    uint16_t lo = popStack();
    uint16_t hi = popStack();
    return (hi << 8) | lo;
}