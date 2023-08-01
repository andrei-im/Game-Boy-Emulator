#ifndef GBEMU_CARTRIDGE_H
#define GBEMU_CARTRIDGE_H

#include "utils.h"

typedef struct {
    uint8_t entry[4];
    uint8_t logo[0x30];

    char title[16];
    uint16_t newLicCode;
    uint8_t superGBFlag;
    uint8_t type;
    uint8_t romSize;
    uint8_t ramSize;
    uint8_t destCode;
    uint8_t licenseCode;
    uint8_t version;
    uint8_t checksum;
    uint16_t globalChecksum;
} romHeader;

bool LoadCartridge(char *cart);

uint8_t readCartridge(uint16_t address);
void writeCartridge(uint16_t address, uint8_t value);

bool needSaveCartridge();
void batteryLoadCartridge();
void batterySaveCartridge();

#endif //GBEMU_CARTRIDGE_H
