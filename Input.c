#include "Input.h"

typedef struct {
    bool buttonSel;
    bool dirSel;
    GamepadButtons controller;
} GamepadState;

static GamepadState gamepadState = {0};

bool buttonSelect() {
    return gamepadState.buttonSel;
}

bool directionSelection() {
    return gamepadState.dirSel;
}

void setGamepadSelection(uint8_t value) {
    gamepadState.buttonSel = value & 0x20;
    gamepadState.dirSel = value & 0x10;
}

GamepadButtons* getGamepadButtonState() {
    return &gamepadState.controller;
}

uint8_t getGamepadOutput() {
    uint8_t output = 0xCF;

    if (!buttonSelect()) {
        if (getGamepadButtonState()->start) {
            output &= ~(1 << 3);
        }
        if (getGamepadButtonState()->select) {
            output &= ~(1 << 2);
        }
        if (getGamepadButtonState()->a) {
            output &= ~(1 << 0);
        }
        if (getGamepadButtonState()->b) {
            output &= ~(1 << 1);
        }
    }

    if (!directionSelection()) {
        if (getGamepadButtonState()->left) {
            output &= ~(1 << 1);
        }
        if (getGamepadButtonState()->right) {
            output &= ~(1 << 0);
        }
        if (getGamepadButtonState()->up) {
            output &= ~(1 << 2);
        }
        if (getGamepadButtonState()->down) {
            output &= ~(1 << 3);
        }
    }

    return output;
}