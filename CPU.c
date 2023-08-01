#include "CPU.h"
#include "MemoryBus.h"
#include "Emulator.h"
#include "Interrupts.h"
#include "Timer.h"


CPUState cpuState = {0};

void InitCPU() {
    cpuState.registers.PC = 0x100;
    cpuState.registers.SP = 0xFFFE;
    *((short *)&cpuState.registers.A) = 0xB001;
    *((short *)&cpuState.registers.B) = 0x1300;
    *((short *)&cpuState.registers.D) = 0xD800;
    *((short *)&cpuState.registers.H) = 0x4D01;
    cpuState.InterruptEnableRegister = 0;
    cpuState.interruptFlags = 0;
    cpuState.interruptMasterEnabled = false;
    cpuState.enablingIME = false;

    getTimerState()->div = 0xABCC;
}

static void getInstruction() {
    cpuState.currentOpcode = ReadByteMemBus(cpuState.registers.PC++);
    cpuState.currentInstruction = getInstructionFromOpcode(cpuState.currentOpcode);
}

void fetchData();

static void execute() {
    IN_PROC proc = getInstructionFunction(cpuState.currentInstruction->type);

    if (!proc) {
        NO_IMPL
    }

    proc(&cpuState);
}

bool CPUStep() {
    if (!cpuState.halted) {
        getInstruction();
        updateEmulatorCycles(1);
        fetchData();

        if (cpuState.currentInstruction == NULL) {
            printf("Unknown Instruction! %02X\n", cpuState.currentOpcode);
            exit(-7);
        }
        execute();
    } else {
        //is halted
        updateEmulatorCycles(1);

        if (cpuState.interruptFlags) {
            cpuState.halted = false;
        }
    }
    if (cpuState.interruptMasterEnabled) {
        cpuHandleInterrupts(&cpuState);
        cpuState.enablingIME = false;
    }
    if (cpuState.enablingIME) {
        cpuState.interruptMasterEnabled = true;
    }

    return true;
}

uint8_t getIERegister() {
    return cpuState.InterruptEnableRegister;
}

void setIERegister(uint8_t n) {
    cpuState.InterruptEnableRegister = n;
}

void cpuRequestInterrupt(InterruptType t) {
    cpuState.interruptFlags |= t;
}