#include "CPU.h"
#include "MemoryBus.h"


extern CPUState cpuState;

uint16_t swapBytes(uint16_t n) {
    uint8_t highByte = (n & 0xFF00) >> 8;
    uint8_t lowByte = (n & 0x00FF);
    return (lowByte << 8) | highByte;
}

uint16_t readCPURegister(RegisterType rt) {
    switch(rt) {
        case RegisterType_A: return cpuState.registers.A;
        case RegisterType_F: return cpuState.registers.F;
        case RegisterType_B: return cpuState.registers.B;
        case RegisterType_C: return cpuState.registers.C;
        case RegisterType_D: return cpuState.registers.D;
        case RegisterType_E: return cpuState.registers.E;
        case RegisterType_H: return cpuState.registers.H;
        case RegisterType_L: return cpuState.registers.L;

        case RegisterType_AF: return swapBytes(*((uint16_t *) &cpuState.registers.A));
        case RegisterType_BC: return swapBytes(*((uint16_t *) &cpuState.registers.B));
        case RegisterType_DE: return swapBytes(*((uint16_t *) &cpuState.registers.D));
        case RegisterType_HL: return swapBytes(*((uint16_t *) &cpuState.registers.H));

        case RegisterType_PC: return cpuState.registers.PC;
        case RegisterType_SP: return cpuState.registers.SP;
        default: return 0;
    }
}

void setCPURegister(RegisterType rt, uint16_t val) {
    switch(rt) {
        case RegisterType_A: cpuState.registers.A = val & 0x00FF; break;
        case RegisterType_F: cpuState.registers.F = val & 0x00FF; break;
        case RegisterType_B: cpuState.registers.B = val & 0x00FF; break;
        case RegisterType_C: {
            cpuState.registers.C = val & 0xFF;
        } break;
        case RegisterType_D: cpuState.registers.D = val & 0x00FF; break;
        case RegisterType_E: cpuState.registers.E = val & 0x00FF; break;
        case RegisterType_H: cpuState.registers.H = val & 0x00FF; break;
        case RegisterType_L: cpuState.registers.L = val & 0x00FF; break;

        case RegisterType_AF: *((uint16_t *)&cpuState.registers.A) = swapBytes(val); break;
        case RegisterType_BC: *((uint16_t *)&cpuState.registers.B) = swapBytes(val); break;
        case RegisterType_DE: *((uint16_t *)&cpuState.registers.D) = swapBytes(val); break;
        case RegisterType_HL: {
            *((uint16_t *)&cpuState.registers.H) = swapBytes(val);
            break;
        }

        case RegisterType_PC: cpuState.registers.PC = val; break;
        case RegisterType_SP: cpuState.registers.SP = val; break;
        case RegisterType_NONE: break;
    }
}


uint8_t read8BitRegister(RegisterType rt) {
    switch(rt) {
        case RegisterType_A: return cpuState.registers.A;
        case RegisterType_F: return cpuState.registers.F;
        case RegisterType_B: return cpuState.registers.B;
        case RegisterType_C: return cpuState.registers.C;
        case RegisterType_D: return cpuState.registers.D;
        case RegisterType_E: return cpuState.registers.E;
        case RegisterType_H: return cpuState.registers.H;
        case RegisterType_L: return cpuState.registers.L;
        case RegisterType_HL: {
            return ReadByteMemBus(readCPURegister(RegisterType_HL));
        }
        default:
            printf("!!!ERROR: INVALID REG8: %d\n", rt);
            NO_IMPL
    }
}

void set8BitRegister(RegisterType rt, uint8_t val) {
    switch(rt) {
        case RegisterType_A: cpuState.registers.A = val & 0xFF; break;
        case RegisterType_F: cpuState.registers.F = val & 0xFF; break;
        case RegisterType_B: cpuState.registers.B = val & 0xFF; break;
        case RegisterType_C: cpuState.registers.C = val & 0xFF; break;
        case RegisterType_D: cpuState.registers.D = val & 0xFF; break;
        case RegisterType_E: cpuState.registers.E = val & 0xFF; break;
        case RegisterType_H: cpuState.registers.H = val & 0xFF; break;
        case RegisterType_L: cpuState.registers.L = val & 0xFF; break;
        case RegisterType_HL:
            WriteByteMemBus(readCPURegister(RegisterType_HL), val); break;
        default:
            printf("!!!ERROR: INVALID REG8: %d\n", rt);
            NO_IMPL
    }
}

CPURegisters* getCPURegisters() {
    return& cpuState.registers;
}

uint8_t getInterruptFlags() {
    return cpuState.interruptFlags;
}

void setInterruptFlags(uint8_t value) {
    cpuState.interruptFlags = value;
}