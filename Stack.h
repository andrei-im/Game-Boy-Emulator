#ifndef GBEMU_STACK_H
#define GBEMU_STACK_H

#include "utils.h"

void pushStack(uint8_t data);
void pushStack16(uint16_t data);

uint8_t popStack();
uint16_t popStack16();

#endif //GBEMU_STACK_H
