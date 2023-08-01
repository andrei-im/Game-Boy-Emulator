#include "Interrupts.h"
#include "CPU.h"
#include "Stack.h"


void handleInterrupt(CPUState* ctx, uint16_t address) {
    pushStack16(ctx->registers.PC);
    ctx->registers.PC = address;
}

bool checkInterrupt(CPUState* ctx, uint16_t address, InterruptType it) {
    if (ctx->interruptFlags & it && ctx->InterruptEnableRegister & it) {
        handleInterrupt(ctx, address);
        ctx->interruptFlags &= ~it;
        ctx->halted = false;
        ctx->interruptMasterEnabled = false;

        return true;
    }

    return false;
}

void cpuHandleInterrupts(CPUState* ctx) {
    if (checkInterrupt(ctx, 0x40, IT_VBLANK)) {

    } else if (checkInterrupt(ctx, 0x48, IT_LCD_STAT)) {

    } else if (checkInterrupt(ctx, 0x50, IT_TIMER)) {

    }  else if (checkInterrupt(ctx, 0x58, IT_SERIAL)) {

    }  else if (checkInterrupt(ctx, 0x60, IT_JOYPAD)) {

    }
}