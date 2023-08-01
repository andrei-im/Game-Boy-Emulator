#ifndef GBEMU_DIRECTMEMORYACCESS_H
#define GBEMU_DIRECTMEMORYACCESS_H

#include "utils.h"

void startDMA(uint8_t start);
void tickDMA();

bool transferringDMA();

#endif //GBEMU_DIRECTMEMORYACCESS_H
