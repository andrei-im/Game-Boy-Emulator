//
// Created by Manwol on 7/1/2023.
//

#ifndef GBEMU_CPU_H
#define GBEMU_CPU_H

#include "utils.h"
#include "Instructions.h"


typedef struct {
    uint8_t A;
    uint8_t F;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint16_t PC;
    uint16_t SP;
} CPURegisters;

typedef struct {
    CPURegisters registers;

    //current fetch...
    uint16_t fetchedData;
    uint16_t memDest;
    bool destIsMem;
    uint8_t currentOpcode;
    instruction* currentInstruction;

    bool halted;
    bool stepping;

    bool interruptMasterEnabled;
    bool enablingIME;
    uint8_t InterruptEnableRegister;
    uint8_t interruptFlags;

} CPUState;

CPURegisters* getCPURegisters();

void InitCPU();
bool CPUStep();

typedef void (*IN_PROC)(CPUState*);

IN_PROC getInstructionFunction(InstructionType type);

#define CPU_FLAG_Z IS_BIT_SET(cpuState->registers.F, 7)
#define CPU_FLAG_N IS_BIT_SET(cpuState->registers.F, 6)
#define CPU_FLAG_H IS_BIT_SET(cpuState->registers.F, 5)
#define CPU_FLAG_C IS_BIT_SET(cpuState->registers.F, 4)

uint16_t readCPURegister(RegisterType rt);
void setCPURegister(RegisterType rt, uint16_t val);

uint8_t getIERegister();
void setIERegister(uint8_t n);

uint8_t read8BitRegister(RegisterType rt);
void set8BitRegister(RegisterType rt, uint8_t val);

uint8_t getInterruptFlags();
void setInterruptFlags(uint8_t value);

void instructionToString(CPUState* cpuState, char *str);

#endif //GBEMU_CPU_H
