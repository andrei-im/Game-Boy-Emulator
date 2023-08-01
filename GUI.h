#ifndef GBEMU_GUI_H
#define GBEMU_GUI_H

#include "utils.h"

static const int SCREEN_WIDTH = 160;
static const int SCREEN_HEIGHT = 144;

void initGUI();
void handleExtraKeyInputs(uint32_t key_code);
void handleEventsGUI();
void updateEmuWindow();

#endif //GBEMU_GUI_H
