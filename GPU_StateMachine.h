#ifndef GBEMU_GPU_STATEMACHINE_H
#define GBEMU_GPU_STATEMACHINE_H

#include "utils.h"

void gpuMode_OAM();
void gpuMode_XFER();
void gpuMode_VBLANK();
void gpuMode_HBLANK();

#endif //GBEMU_GPU_STATEMACHINE_H
