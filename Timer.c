#include "Timer.h"
#include "Interrupts.h"


static TimerState timerState = {0};

TimerState* getTimerState() {
    return &timerState;
}

void initTimer() {
    timerState.div = 0xAC00;
}

void tickTimer() {
    uint16_t prev_div = timerState.div;

    timerState.div++;

    bool doUpdateTimer = false;

    switch(timerState.tac & (0b11)) {
        case 0b00:
            doUpdateTimer = (prev_div & (1 << 9)) && (!(timerState.div & (1 << 9)));
            break;
        case 0b01:
            doUpdateTimer = (prev_div & (1 << 3)) && (!(timerState.div & (1 << 3)));
            break;
        case 0b10:
            doUpdateTimer = (prev_div & (1 << 5)) && (!(timerState.div & (1 << 5)));
            break;
        case 0b11:
            doUpdateTimer = (prev_div & (1 << 7)) && (!(timerState.div & (1 << 7)));
            break;
    }

    if (doUpdateTimer && timerState.tac & (1 << 2)) {
        timerState.tima++;

        if (timerState.tima == 0xFF) {
            timerState.tima = timerState.tma;

            cpuRequestInterrupt(IT_TIMER);
        }
    }
}

void writeTimer(uint16_t address, uint8_t value) {
    switch(address) {
        case 0xFF04:
            //DIV
            timerState.div = 0;
            break;

        case 0xFF05:
            //TIMA
            timerState.tima = value;
            break;

        case 0xFF06:
            //TMA
            timerState.tma = value;
            break;

        case 0xFF07:
            //TAC
            timerState.tac = value;
            break;
    }
}

uint8_t readTimer(uint16_t address) {
    switch(address) {
        case 0xFF04:
            return timerState.div >> 8;
        case 0xFF05:
            return timerState.tima;
        case 0xFF06:
            return timerState.tma;
        case 0xFF07:
            return timerState.tac;
    }
}