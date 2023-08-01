#ifndef GBEMU_TIMER_H
#define GBEMU_TIMER_H

#include "utils.h"

typedef struct {
    uint16_t div;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;
} TimerState;

void initTimer();
void tickTimer();

void writeTimer(uint16_t address, uint8_t value);
uint8_t readTimer(uint16_t address);

TimerState* getTimerState();

#endif //GBEMU_TIMER_H
