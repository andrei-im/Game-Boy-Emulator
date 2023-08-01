#include "CPU.h"
#include "MemoryBus.h"
#include "Emulator.h"


extern CPUState cpuState;

void fetchData() {
    cpuState.memDest = 0;
    cpuState.destIsMem = false;

    if (cpuState.currentInstruction == NULL) {
        return;
    }

    switch(cpuState.currentInstruction->mode) {
        case AM_IMP: return;

        case AM_R:
            cpuState.fetchedData = readCPURegister(cpuState.currentInstruction->reg_1);
            return;
        case AM_R_R:
            cpuState.fetchedData = readCPURegister(cpuState.currentInstruction->reg_2);
            return;
        case AM_R_D8:
            cpuState.fetchedData = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);
            cpuState.registers.PC++;
            return;
        case AM_R_D16:
        case AM_D16: {
            uint16_t lo = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);

            uint16_t hi = ReadByteMemBus(cpuState.registers.PC + 1);
            updateEmulatorCycles(1);

            cpuState.fetchedData = lo | (hi << 8);

            cpuState.registers.PC += 2;

            return;
        }
        case AM_MR_R:
            cpuState.fetchedData = readCPURegister(cpuState.currentInstruction->reg_2);
            cpuState.memDest = readCPURegister(cpuState.currentInstruction->reg_1);
            cpuState.destIsMem = true;

            if (cpuState.currentInstruction->reg_1 == RegisterType_C) {
                cpuState.memDest |= 0xFF00;
            }
            return;
        case AM_R_MR: {
            uint16_t addr = readCPURegister(cpuState.currentInstruction->reg_2);

            if (cpuState.currentInstruction->reg_2 == RegisterType_C) {
                addr |= 0xFF00;
            }

            cpuState.fetchedData = ReadByteMemBus(addr);
            updateEmulatorCycles(1);

        } return;
        case AM_R_HLI:
            cpuState.fetchedData = ReadByteMemBus(readCPURegister(cpuState.currentInstruction->reg_2));
            updateEmulatorCycles(1);
            setCPURegister(RegisterType_HL, readCPURegister(RegisterType_HL) + 1);
            return;
        case AM_R_HLD:
            cpuState.fetchedData = ReadByteMemBus(readCPURegister(cpuState.currentInstruction->reg_2));
            updateEmulatorCycles(1);
            setCPURegister(RegisterType_HL, readCPURegister(RegisterType_HL) - 1);
            return;
        case AM_HLI_R:
            cpuState.fetchedData = readCPURegister(cpuState.currentInstruction->reg_2);
            cpuState.memDest = readCPURegister(cpuState.currentInstruction->reg_1);
            cpuState.destIsMem = true;
            setCPURegister(RegisterType_HL, readCPURegister(RegisterType_HL) + 1);
            return;
        case AM_HLD_R:
            cpuState.fetchedData = readCPURegister(cpuState.currentInstruction->reg_2);
            cpuState.memDest = readCPURegister(cpuState.currentInstruction->reg_1);
            cpuState.destIsMem = true;
            setCPURegister(RegisterType_HL, readCPURegister(RegisterType_HL) - 1);
            return;
        case AM_R_A8:
            cpuState.fetchedData = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);
            cpuState.registers.PC++;
            return;
        case AM_A8_R:
            cpuState.memDest = ReadByteMemBus(cpuState.registers.PC) | 0xFF00;
            cpuState.destIsMem = true;
            updateEmulatorCycles(1);
            cpuState.registers.PC++;
            return;
        case AM_HL_SPR:
            cpuState.fetchedData = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);
            cpuState.registers.PC++;
            return;
        case AM_D8:
            cpuState.fetchedData = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);
            cpuState.registers.PC++;
            return;
        case AM_A16_R:
        case AM_D16_R: {
            uint16_t lo = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);

            uint16_t hi = ReadByteMemBus(cpuState.registers.PC + 1);
            updateEmulatorCycles(1);

            cpuState.memDest = lo | (hi << 8);
            cpuState.destIsMem = true;

            cpuState.registers.PC += 2;
            cpuState.fetchedData = readCPURegister(cpuState.currentInstruction->reg_2);

        } return;
        case AM_MR_D8:
            cpuState.fetchedData = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);
            cpuState.registers.PC++;
            cpuState.memDest = readCPURegister(cpuState.currentInstruction->reg_1);
            cpuState.destIsMem = true;
            return;

        case AM_MR:
            cpuState.memDest = readCPURegister(cpuState.currentInstruction->reg_1);
            cpuState.destIsMem = true;
            cpuState.fetchedData = ReadByteMemBus(readCPURegister(cpuState.currentInstruction->reg_1));
            updateEmulatorCycles(1);
            return;

        case AM_R_A16: {
            uint16_t lo = ReadByteMemBus(cpuState.registers.PC);
            updateEmulatorCycles(1);

            uint16_t hi = ReadByteMemBus(cpuState.registers.PC + 1);
            updateEmulatorCycles(1);

            uint16_t addr = lo | (hi << 8);

            cpuState.registers.PC += 2;
            cpuState.fetchedData = ReadByteMemBus(addr);
            updateEmulatorCycles(1);

            return;
        }

        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", cpuState.currentInstruction->mode, cpuState.currentOpcode);
            exit(-7);
    }
}