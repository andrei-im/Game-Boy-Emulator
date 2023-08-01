#ifndef GBEMU_INPUT_H
#define GBEMU_INPUT_H

#include "utils.h"

typedef struct {
    bool start;
    bool select;
    bool a;
    bool b;
    bool up;
    bool down;
    bool left;
    bool right;
} GamepadButtons;

GamepadButtons* getGamepadButtonState();
uint8_t getGamepadOutput();

bool buttonSelect();
bool directionSelection();
void setGamepadSelection(uint8_t value);

#endif //GBEMU_INPUT_H
