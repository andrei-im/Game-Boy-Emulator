#include "DirectMemoryAccess.h"
#include "GPU.h"
#include "MemoryBus.h"


typedef struct {
    bool active;
    uint8_t byte;
    uint8_t value;
    uint8_t transferDelay;
} DMAStatus;

static DMAStatus dmaStatus;

void startDMA(uint8_t start) {
    dmaStatus.active = true;
    dmaStatus.byte = 0;
    dmaStatus.transferDelay = 2;
    dmaStatus.value = start;
}

void tickDMA() {
    if (!dmaStatus.active) {
        return;
    }

    if (dmaStatus.transferDelay) {
        dmaStatus.transferDelay--;
        return;
    }

    writeOAM(dmaStatus.byte, ReadByteMemBus((dmaStatus.value * 0x100) + dmaStatus.byte));

    dmaStatus.byte++;

    dmaStatus.active = dmaStatus.byte < 0xA0;
}

bool transferringDMA() {
    return dmaStatus.active;
}