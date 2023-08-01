#include "CPU.h"
#include "Emulator.h"
#include "MemoryBus.h"
#include "Stack.h"


//processes CPU instructions

void setFlags(CPUState* cpuState, int8_t z, int8_t n, int8_t h, int8_t c) {
    if (z != -1) {
        SET_OR_CLEAR_BIT(cpuState->registers.F, 7, z);
    }

    if (n != -1) {
        SET_OR_CLEAR_BIT(cpuState->registers.F, 6, n);
    }

    if (h != -1) {
        SET_OR_CLEAR_BIT(cpuState->registers.F, 5, h);
    }

    if (c != -1) {
        SET_OR_CLEAR_BIT(cpuState->registers.F, 4, c);
    }
}

static void processNone(CPUState* cpuState) {
    printf("INVALID INSTRUCTION!\n");
    exit(-7);
}

static void NOP(CPUState* cpuState) {
    // no operation
}

RegisterType RegisterType_lookup[] = {
        RegisterType_B,
        RegisterType_C,
        RegisterType_D,
        RegisterType_E,
        RegisterType_H,
        RegisterType_L,
        RegisterType_HL,
        RegisterType_A
};

RegisterType decodeRegister(uint8_t reg) {
    if (reg > 0b111) {
        return RegisterType_NONE;
    }

    return RegisterType_lookup[reg];
}

static void processCB(CPUState* cpuState) {
    uint8_t op = cpuState->fetchedData;
    RegisterType reg = decodeRegister(op & 0b111);
    uint8_t bit = (op >> 3) & 0b111;
    uint8_t bit_op = (op >> 6) & 0b11;
    uint8_t reg_val = read8BitRegister(reg);

    updateEmulatorCycles(1);

    if (reg == RegisterType_HL) {
        updateEmulatorCycles(2);
    }

    switch(bit_op) {
        case 1:
            //IS_BIT_SET
            setFlags(cpuState, !(reg_val & (1 << bit)), 0, 1, -1);
            return;

        case 2:
            //RST
            reg_val &= ~(1 << bit);
            set8BitRegister(reg, reg_val);
            return;

        case 3:
            //SET
            reg_val |= (1 << bit);
            set8BitRegister(reg, reg_val);
            return;
    }

    bool flagC = CPU_FLAG_C;

    switch(bit) {
        case 0: {
            //RLC
            bool setC = false;
            uint8_t result = (reg_val << 1) & 0xFF;

            if ((reg_val & (1 << 7)) != 0) {
                result |= 1;
                setC = true;
            }

            set8BitRegister(reg, result);
            setFlags(cpuState, result == 0, false, false, setC);
        } return;

        case 1: {
            //RRC
            uint8_t old = reg_val;
            reg_val >>= 1;
            reg_val |= (old << 7);

            set8BitRegister(reg, reg_val);
            setFlags(cpuState, !reg_val, false, false, old & 1);
        } return;

        case 2: {
            //RL
            uint8_t old = reg_val;
            reg_val <<= 1;
            reg_val |= flagC;

            set8BitRegister(reg, reg_val);
            setFlags(cpuState, !reg_val, false, false, !!(old & 0x80));
        } return;

        case 3: {
            //RR
            uint8_t old = reg_val;
            reg_val >>= 1;

            reg_val |= (flagC << 7);

            set8BitRegister(reg, reg_val);
            setFlags(cpuState, !reg_val, false, false, old & 1);
        } return;

        case 4: {
            //SLA
            uint8_t old = reg_val;
            reg_val <<= 1;

            set8BitRegister(reg, reg_val);
            setFlags(cpuState, !reg_val, false, false, !!(old & 0x80));
        } return;

        case 5: {
            //SRA
            uint8_t u = (int8_t)reg_val >> 1;
            set8BitRegister(reg, u);
            setFlags(cpuState, !u, 0, 0, reg_val & 1);
        } return;

        case 6: {
            //SWAP
            reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
            set8BitRegister(reg, reg_val);
            setFlags(cpuState, reg_val == 0, false, false, false);
        } return;

        case 7: {
            //SRL
            uint8_t u = reg_val >> 1;
            set8BitRegister(reg, u);
            setFlags(cpuState, !u, 0, 0, reg_val & 1);
        } return;
    }

    fprintf(stderr, "ERROR: INVALID CB: %02X", op);
    NO_IMPL
}

static void RLCA(CPUState* cpuState) {
    uint8_t u = cpuState->registers.A;
    bool c = (u >> 7) & 1;
    u = (u << 1) | c;
    cpuState->registers.A = u;

    setFlags(cpuState, 0, 0, 0, c);
}

static void RRCA(CPUState* cpuState) {
    uint8_t b = cpuState->registers.A & 1;
    cpuState->registers.A >>= 1;
    cpuState->registers.A |= (b << 7);

    setFlags(cpuState, 0, 0, 0, b);
}


static void RLA(CPUState* cpuState) {
    uint8_t u = cpuState->registers.A;
    uint8_t cf = CPU_FLAG_C;
    uint8_t c = (u >> 7) & 1;

    cpuState->registers.A = (u << 1) | cf;
    setFlags(cpuState, 0, 0, 0, c);
}

static void processSTOP(CPUState* cpuState) {
    fprintf(stderr, "STOPPING!\n");
}

static void DAA(CPUState* cpuState) {
    uint8_t u = 0;
    int fc = 0;

    if (CPU_FLAG_H || (!CPU_FLAG_N && (cpuState->registers.A & 0xF) > 9)) {
        u = 6;
    }

    if (CPU_FLAG_C || (!CPU_FLAG_N && cpuState->registers.A > 0x99)) {
        u |= 0x60;
        fc = 1;
    }

    cpuState->registers.A += CPU_FLAG_N ? -u : u;

    setFlags(cpuState, cpuState->registers.A == 0, -1, 0, fc);
}

static void CPL(CPUState* cpuState) {
    cpuState->registers.A = ~cpuState->registers.A;
    setFlags(cpuState, -1, 1, 1, -1);
}

static void SCF(CPUState* cpuState) {
    setFlags(cpuState, -1, 0, 0, 1);
}

static void CCF(CPUState* cpuState) {
    setFlags(cpuState, -1, 0, 0, CPU_FLAG_C ^ 1);
}

static void HALT(CPUState* cpuState) {
    cpuState->halted = true;
}

static void RRA(CPUState* cpuState) {
    uint8_t carry = CPU_FLAG_C;
    uint8_t new_c = cpuState->registers.A & 1;

    cpuState->registers.A >>= 1;
    cpuState->registers.A |= (carry << 7);

    setFlags(cpuState, 0, 0, 0, new_c);
}

static void AND(CPUState* cpuState) {
    cpuState->registers.A &= cpuState->fetchedData;
    setFlags(cpuState, cpuState->registers.A == 0, 0, 1, 0);
}

static void XOR(CPUState* cpuState) {
    cpuState->registers.A ^= cpuState->fetchedData & 0xFF;
    setFlags(cpuState, cpuState->registers.A == 0, 0, 0, 0);
}

static void OR(CPUState* cpuState) {
    cpuState->registers.A |= cpuState->fetchedData & 0xFF;
    setFlags(cpuState, cpuState->registers.A == 0, 0, 0, 0);
}

static void CP(CPUState* cpuState) {
    int n = (int)cpuState->registers.A - (int)cpuState->fetchedData;

    setFlags(cpuState, n == 0, 1,
             ((int) cpuState->registers.A & 0x0F) - ((int) cpuState->fetchedData & 0x0F) < 0, n < 0);
}

static void DI(CPUState* cpuState) {
    cpuState->interruptMasterEnabled = false;
}

static void EI(CPUState* cpuState) {
    cpuState->enablingIME = true;
}

static bool is16Bit(RegisterType rt) {
    return rt >= RegisterType_AF;
}

static void LD(CPUState* cpuState) {
    if (cpuState->destIsMem) {
        //LD (BC), A for instance...

        if (is16Bit(cpuState->currentInstruction->reg_2)) {
            //if 16 bit register...
            updateEmulatorCycles(1);
            WriteWordMemBus(cpuState->memDest, cpuState->fetchedData);
        } else {
            WriteByteMemBus(cpuState->memDest, cpuState->fetchedData);
        }

        updateEmulatorCycles(1);

        return;
    }

    if (cpuState->currentInstruction->mode == AM_HL_SPR) {
        uint8_t hflag = (readCPURegister(cpuState->currentInstruction->reg_2) & 0xF) +
                        (cpuState->fetchedData & 0xF) >= 0x10;

        uint8_t cflag = (readCPURegister(cpuState->currentInstruction->reg_2) & 0xFF) +
                        (cpuState->fetchedData & 0xFF) >= 0x100;

        setFlags(cpuState, 0, 0, hflag, cflag);
        setCPURegister(cpuState->currentInstruction->reg_1,
                       readCPURegister(cpuState->currentInstruction->reg_2) + (int8_t) cpuState->fetchedData);

        return;
    }

    setCPURegister(cpuState->currentInstruction->reg_1, cpuState->fetchedData);
}

static void LDH(CPUState* cpuState) {
    if (cpuState->currentInstruction->reg_1 == RegisterType_A) {
        setCPURegister(cpuState->currentInstruction->reg_1, ReadByteMemBus(0xFF00 | cpuState->fetchedData));
    } else {
        WriteByteMemBus(cpuState->memDest, cpuState->registers.A);
    }

    updateEmulatorCycles(1);
}


static bool checkCondition(CPUState* cpuState) {
    bool z = CPU_FLAG_Z;
    bool c = CPU_FLAG_C;

    switch(cpuState->currentInstruction->cond) {
        case CT_NONE: return true;
        case CT_C: return c;
        case CT_NC: return !c;
        case CT_Z: return z;
        case CT_NZ: return !z;
    }

    return false;
}

static void gotoAddress(CPUState* cpuState, uint16_t addr, bool pushpc) {
    if (checkCondition(cpuState)) {
        if (pushpc) {
            updateEmulatorCycles(2);
            pushStack16(cpuState->registers.PC);
        }

        cpuState->registers.PC = addr;
        updateEmulatorCycles(1);
    }
}

static void JP(CPUState* cpuState) {
    gotoAddress(cpuState, cpuState->fetchedData, false);
}

static void JR(CPUState* cpuState) {
    int8_t rel = (int8_t)(cpuState->fetchedData & 0xFF);
    uint16_t addr = cpuState->registers.PC + rel;
    gotoAddress(cpuState, addr, false);
}

static void CALL(CPUState* cpuState) {
    gotoAddress(cpuState, cpuState->fetchedData, true);
}

static void RST(CPUState* cpuState) {
    gotoAddress(cpuState, cpuState->currentInstruction->param, true);
}

static void RET(CPUState* cpuState) {
    if (cpuState->currentInstruction->cond != CT_NONE) {
        updateEmulatorCycles(1);
    }

    if (checkCondition(cpuState)) {
        uint16_t lo = popStack();
        updateEmulatorCycles(1);
        uint16_t hi = popStack();
        updateEmulatorCycles(1);

        uint16_t n = (hi << 8) | lo;
        cpuState->registers.PC = n;

        updateEmulatorCycles(1);
    }
}

static void RETI(CPUState* cpuState) {
    cpuState->interruptMasterEnabled = true;
    RET(cpuState);
}

static void POP(CPUState* cpuState) {
    uint16_t lo = popStack();
    updateEmulatorCycles(1);
    uint16_t hi = popStack();
    updateEmulatorCycles(1);

    uint16_t n = (hi << 8) | lo;

    setCPURegister(cpuState->currentInstruction->reg_1, n);

    if (cpuState->currentInstruction->reg_1 == RegisterType_AF) {
        setCPURegister(cpuState->currentInstruction->reg_1, n & 0xFFF0);
    }
}

static void PUSH(CPUState* cpuState) {
    uint16_t hi = (readCPURegister(cpuState->currentInstruction->reg_1) >> 8) & 0xFF;
    updateEmulatorCycles(1);
    pushStack(hi);

    uint16_t lo = readCPURegister(cpuState->currentInstruction->reg_1) & 0xFF;
    updateEmulatorCycles(1);
    pushStack(lo);

    updateEmulatorCycles(1);
}

static void INC(CPUState* cpuState) {
    uint16_t val = readCPURegister(cpuState->currentInstruction->reg_1) + 1;

    if (is16Bit(cpuState->currentInstruction->reg_1)) {
        updateEmulatorCycles(1);
    }

    if (cpuState->currentInstruction->reg_1 == RegisterType_HL && cpuState->currentInstruction->mode == AM_MR) {
        val = ReadByteMemBus(readCPURegister(RegisterType_HL)) + 1;
        val &= 0xFF;
        WriteByteMemBus(readCPURegister(RegisterType_HL), val);
    } else {
        setCPURegister(cpuState->currentInstruction->reg_1, val);
        val = readCPURegister(cpuState->currentInstruction->reg_1);
    }

    if ((cpuState->currentOpcode & 0x03) == 0x03) {
        return;
    }

    setFlags(cpuState, val == 0, 0, (val & 0x0F) == 0, -1);
}

static void DEC(CPUState* cpuState) {
    uint16_t val = readCPURegister(cpuState->currentInstruction->reg_1) - 1;

    if (is16Bit(cpuState->currentInstruction->reg_1)) {
        updateEmulatorCycles(1);
    }

    if (cpuState->currentInstruction->reg_1 == RegisterType_HL && cpuState->currentInstruction->mode == AM_MR) {
        val = ReadByteMemBus(readCPURegister(RegisterType_HL)) - 1;
        WriteByteMemBus(readCPURegister(RegisterType_HL), val);
    } else {
        setCPURegister(cpuState->currentInstruction->reg_1, val);
        val = readCPURegister(cpuState->currentInstruction->reg_1);
    }

    if ((cpuState->currentOpcode & 0x0B) == 0x0B) {
        return;
    }

    setFlags(cpuState, val == 0, 1, (val & 0x0F) == 0x0F, -1);
}

static void SUB(CPUState* cpuState) {
    uint16_t val = readCPURegister(cpuState->currentInstruction->reg_1) - cpuState->fetchedData;

    int z = val == 0;
    int h = ((int) readCPURegister(cpuState->currentInstruction->reg_1) & 0xF) - ((int)cpuState->fetchedData & 0xF) < 0;
    int c = ((int) readCPURegister(cpuState->currentInstruction->reg_1)) - ((int)cpuState->fetchedData) < 0;

    setCPURegister(cpuState->currentInstruction->reg_1, val);
    setFlags(cpuState, z, 1, h, c);
}

static void SBC(CPUState* cpuState) {
    uint8_t val = cpuState->fetchedData + CPU_FLAG_C;

    int z = readCPURegister(cpuState->currentInstruction->reg_1) - val == 0;

    int h = ((int) readCPURegister(cpuState->currentInstruction->reg_1) & 0xF)
            - ((int)cpuState->fetchedData & 0xF) - ((int)CPU_FLAG_C) < 0;
    int c = ((int) readCPURegister(cpuState->currentInstruction->reg_1))
            - ((int)cpuState->fetchedData) - ((int)CPU_FLAG_C) < 0;

    setCPURegister(cpuState->currentInstruction->reg_1, readCPURegister(cpuState->currentInstruction->reg_1) - val);
    setFlags(cpuState, z, 1, h, c);
}

static void ADC(CPUState* cpuState) {
    uint16_t u = cpuState->fetchedData;
    uint16_t a = cpuState->registers.A;
    uint16_t c = CPU_FLAG_C;

    cpuState->registers.A = (a + u + c) & 0xFF;

    setFlags(cpuState, cpuState->registers.A == 0, 0,
             (a & 0xF) + (u & 0xF) + c > 0xF,
             a + u + c > 0xFF);
}

static void ADD(CPUState* cpuState) {
    uint32_t val = readCPURegister(cpuState->currentInstruction->reg_1) + cpuState->fetchedData;

    bool is_16bit = is16Bit(cpuState->currentInstruction->reg_1);

    if (is_16bit) {
        updateEmulatorCycles(1);
    }

    if (cpuState->currentInstruction->reg_1 == RegisterType_SP) {
        val = readCPURegister(cpuState->currentInstruction->reg_1) + (int8_t)cpuState->fetchedData;
    }

    int z = (val & 0xFF) == 0;
    int h = (readCPURegister(cpuState->currentInstruction->reg_1) & 0xF) + (cpuState->fetchedData & 0xF) >= 0x10;
    int c = (int)(readCPURegister(cpuState->currentInstruction->reg_1) & 0xFF) + (int)(cpuState->fetchedData & 0xFF) >= 0x100;

    if (is_16bit) {
        z = -1;
        h = (readCPURegister(cpuState->currentInstruction->reg_1) & 0xFFF) + (cpuState->fetchedData & 0xFFF) >= 0x1000;
        uint32_t n = ((uint32_t) readCPURegister(cpuState->currentInstruction->reg_1)) + ((uint32_t)cpuState->fetchedData);
        c = n >= 0x10000;
    }

    if (cpuState->currentInstruction->reg_1 == RegisterType_SP) {
        z = 0;
        h = (readCPURegister(cpuState->currentInstruction->reg_1) & 0xF) + (cpuState->fetchedData & 0xF) >= 0x10;
        c = (int)(readCPURegister(cpuState->currentInstruction->reg_1) & 0xFF) + (int)(cpuState->fetchedData & 0xFF) >= 0x100;
    }

    setCPURegister(cpuState->currentInstruction->reg_1, val & 0xFFFF);
    setFlags(cpuState, z, 0, h, c);
}

static IN_PROC instructionFunctionMap[] = {
        [IN_NONE] = processNone,
        [IN_NOP] = NOP,
        [IN_LD] = LD,
        [IN_LDH] = LDH,
        [IN_JP] = JP,
        [IN_DI] = DI,
        [IN_POP] = POP,
        [IN_PUSH] = PUSH,
        [IN_JR] = JR,
        [IN_CALL] = CALL,
        [IN_RET] = RET,
        [IN_RST] = RST,
        [IN_DEC] = DEC,
        [IN_INC] = INC,
        [IN_ADD] = ADD,
        [IN_ADC] = ADC,
        [IN_SUB] = SUB,
        [IN_SBC] = SBC,
        [IN_AND] = AND,
        [IN_XOR] = XOR,
        [IN_OR] = OR,
        [IN_CP] = CP,
        [IN_CB] = processCB,
        [IN_RRCA] = RRCA,
        [IN_RLCA] = RLCA,
        [IN_RRA] = RRA,
        [IN_RLA] = RLA,
        [IN_STOP] = processSTOP,
        [IN_HALT] = HALT,
        [IN_DAA] = DAA,
        [IN_CPL] = CPL,
        [IN_SCF] = SCF,
        [IN_CCF] = CCF,
        [IN_EI] = EI,
        [IN_RETI] = RETI
};

IN_PROC getInstructionFunction(InstructionType type) {
    return instructionFunctionMap[type];
}