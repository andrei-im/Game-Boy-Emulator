#ifndef GBEMU_EMULATOR_H
#define GBEMU_EMULATOR_H

#include "utils.h"

typedef struct {
    bool paused;
    bool running;
    bool die;
    uint64_t ticks;
} EmulatorState;

int runEmulator(int argc, char **argv);

EmulatorState* getEmulatorState();

void updateEmulatorCycles(int cpu_cycles);

#endif //GBEMU_EMULATOR_H
