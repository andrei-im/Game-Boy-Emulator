#ifndef GBEMU_INTERRUPTS_H
#define GBEMU_INTERRUPTS_H

#include "CPU.h"

typedef enum {
    IT_VBLANK = 1,
    IT_LCD_STAT = 2,
    IT_TIMER = 4,
    IT_SERIAL = 8,
    IT_JOYPAD = 16
} InterruptType;

void cpuRequestInterrupt(InterruptType t);
void cpuHandleInterrupts(CPUState* ctx);

#endif //GBEMU_INTERRUPTS_H
