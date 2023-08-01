#ifndef GBEMU_INSTRUCTIONS_H
#define GBEMU_INSTRUCTIONS_H

#include "utils.h"


typedef enum {
    AM_IMP,
    AM_R_D16,
    AM_R_R,
    AM_MR_R,
    AM_R,
    AM_R_D8,
    AM_R_MR,
    AM_R_HLI,
    AM_R_HLD,
    AM_HLI_R,
    AM_HLD_R,
    AM_R_A8,
    AM_A8_R,
    AM_HL_SPR,
    AM_D16,
    AM_D8,
    AM_D16_R,
    AM_MR_D8,
    AM_MR,
    AM_A16_R,
    AM_R_A16
} AddressingMode;

typedef enum {
    RegisterType_NONE,
    RegisterType_A,
    RegisterType_F,
    RegisterType_B,
    RegisterType_C,
    RegisterType_D,
    RegisterType_E,
    RegisterType_H,
    RegisterType_L,
    RegisterType_AF,
    RegisterType_BC,
    RegisterType_DE,
    RegisterType_HL,
    RegisterType_SP,
    RegisterType_PC
} RegisterType;

typedef enum {
    IN_NONE,
    IN_NOP,
    IN_LD,
    IN_INC,
    IN_DEC,
    IN_RLCA,
    IN_ADD,
    IN_RRCA,
    IN_STOP,
    IN_RLA,
    IN_JR,
    IN_RRA,
    IN_DAA,
    IN_CPL,
    IN_SCF,
    IN_CCF,
    IN_HALT,
    IN_ADC,
    IN_SUB,
    IN_SBC,
    IN_AND,
    IN_XOR,
    IN_OR,
    IN_CP,
    IN_POP,
    IN_JP,
    IN_PUSH,
    IN_RET,
    IN_CB,
    IN_CALL,
    IN_RETI,
    IN_LDH,
    IN_JPHL,
    IN_DI,
    IN_EI,
    IN_RST,
    IN_ERR,
    //CB instructions
    IN_RLC,
    IN_RRC,
    IN_RL,
    IN_RR,
    IN_SLA,
    IN_SRA,
    IN_SWAP,
    IN_SRL,
    IN_BIT,
    IN_RES,
    IN_SET
} InstructionType;

typedef enum {
    CT_NONE, CT_NZ, CT_Z, CT_NC, CT_C
} ConditionType;

typedef struct {
    InstructionType type;
    AddressingMode mode;
    RegisterType reg_1;
    RegisterType reg_2;
    ConditionType cond;
    uint8_t param;
} instruction;

instruction* getInstructionFromOpcode(uint8_t opcode);

char* inst_name(InstructionType t);


#endif //GBEMU_INSTRUCTIONS_H
