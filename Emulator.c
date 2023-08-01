#include <stdio.h>
#include "Emulator.h"
#include "Cartridge.h"
#include "CPU.h"
#include "GUI.h"
#include "Timer.h"
#include "DirectMemoryAccess.h"
#include "GPU.h"

// mingw-w64 handles the POSIX threads and provides a windows alternative
#include <pthread.h>
#include <unistd.h>

static EmulatorState emulatorState;

EmulatorState* getEmulatorState() {
    return &emulatorState;
}

void *cpu_run(void *p) {
    initTimer();
    InitCPU();
    initGPU();

    emulatorState.running = true;
    emulatorState.paused = false;
    emulatorState.ticks = 0;

    while(emulatorState.running) {
        if (emulatorState.paused) {
            delay(10);
            continue;
        }

        if (!CPUStep()) {
            printf("CPU Stopped\n");
            return 0;
        }
    }

    return 0;
}

int runEmulator(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: emu <rom_file>\n");
        return -1;
    }

    if (!LoadCartridge(argv[1])) {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }

    initGUI();

    pthread_t t1;

    if (pthread_create(&t1, NULL, cpu_run, NULL)) {
        fprintf(stderr, "FAILED TO START MAIN CPU THREAD!\n");
        return -1;
    }

    uint32_t prev_frame = 0;

    while(!emulatorState.die) {
        // sleep the thread
        usleep(1000);
        handleEventsGUI();

        if (prev_frame != getGPUState()->currentFrame) {
            updateEmuWindow();
        }

        prev_frame = getGPUState()->currentFrame;
    }

    return 0;
}

void updateEmulatorCycles(int cpu_cycles) {

    for (int i=0; i<cpu_cycles; i++) {
        for (int n=0; n<4; n++) {
            emulatorState.ticks++;
            tickTimer();
            gpuTick();
        }

        tickDMA();
    }
}