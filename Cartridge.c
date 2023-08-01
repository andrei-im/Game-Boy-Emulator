#include "Cartridge.h"

typedef struct {
    char filename[1024];
    uint32_t romSize;
    uint8_t* romData;
    romHeader* header;

    //mbc1 related data
    bool ramEnabled;
    bool ramBanking;

    uint8_t* romBankX;
    uint8_t bankingMode;

    uint8_t romBankValue;
    uint8_t ramBankValue;

    uint8_t* ramBank; //currently selected ram bank
    uint8_t* ramBanks[16]; //all ram banks

    bool battery; // cart contains battery
    bool needSave; //should save battery backup.
} CartridgeState;

static CartridgeState cartridgeState;

bool needSaveCartridge() {
    return cartridgeState.needSave;
}

bool isMBC1() {
    return IS_IN_RANGE(cartridgeState.header->type, 1, 3);
}

bool caRegisterType_battery() {
    //mbc1 only
    return cartridgeState.header->type == 3;
}

static const char* ROM_TYPES[] = {
        "ROM ONLY",
        "MBC1",
        "MBC1+RAM",
        "MBC1+RAM+BATTERY",
        "0x04 ???",
        "MBC2",
        "MBC2+BATTERY",
        "0x07 ???",
        "ROM+RAM 1",
        "ROM+RAM+BATTERY 1",
        "0x0A ???",
        "MMM01",
        "MMM01+RAM",
        "MMM01+RAM+BATTERY",
        "0x0E ???",
        "MBC3+TIMER+BATTERY",
        "MBC3+TIMER+RAM+BATTERY 2",
        "MBC3",
        "MBC3+RAM 2",
        "MBC3+RAM+BATTERY 2",
        "0x14 ???",
        "0x15 ???",
        "0x16 ???",
        "0x17 ???",
        "0x18 ???",
        "MBC5",
        "MBC5+RAM",
        "MBC5+RAM+BATTERY",
        "MBC5+RUMBLE",
        "MBC5+RUMBLE+RAM",
        "MBC5+RUMBLE+RAM+BATTERY",
        "0x1F ???",
        "MBC6",
        "0x21 ???",
        "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
};

static const char* LIC_CODE[0xA5] = {
        [0x00] = "None",
        [0x01] = "Nintendo R&D1",
        [0x08] = "Capcom",
        [0x13] = "Electronic Arts",
        [0x18] = "Hudson Soft",
        [0x19] = "b-ai",
        [0x20] = "kss",
        [0x22] = "pow",
        [0x24] = "PCM Complete",
        [0x25] = "san-x",
        [0x28] = "Kemco Japan",
        [0x29] = "seta",
        [0x30] = "Viacom",
        [0x31] = "Nintendo",
        [0x32] = "Bandai",
        [0x33] = "Ocean/Acclaim",
        [0x34] = "Konami",
        [0x35] = "Hector",
        [0x37] = "Taito",
        [0x38] = "Hudson",
        [0x39] = "Banpresto",
        [0x41] = "Ubi Soft",
        [0x42] = "Atlus",
        [0x44] = "Malibu",
        [0x46] = "angel",
        [0x47] = "Bullet-Proof",
        [0x49] = "irem",
        [0x50] = "Absolute",
        [0x51] = "Acclaim",
        [0x52] = "Activision",
        [0x53] = "American sammy",
        [0x54] = "Konami",
        [0x55] = "Hi tech entertainment",
        [0x56] = "LJN",
        [0x57] = "Matchbox",
        [0x58] = "Mattel",
        [0x59] = "Milton Bradley",
        [0x60] = "Titus",
        [0x61] = "Virgin",
        [0x64] = "LucasArts",
        [0x67] = "Ocean",
        [0x69] = "Electronic Arts",
        [0x70] = "Infogrames",
        [0x71] = "Interplay",
        [0x72] = "Broderbund",
        [0x73] = "sculptured",
        [0x75] = "sci",
        [0x78] = "THQ",
        [0x79] = "Accolade",
        [0x80] = "misawa",
        [0x83] = "lozc",
        [0x86] = "Tokuma Shoten Intermedia",
        [0x87] = "Tsukuda Original",
        [0x91] = "Chunsoft",
        [0x92] = "Video system",
        [0x93] = "Ocean/Acclaim",
        [0x95] = "Varie",
        [0x96] = "Yonezawa/s’pal",
        [0x97] = "Kaneko",
        [0x99] = "Pack in soft",
        [0xA4] = "Konami (Yu-Gi-Oh!)"
};

const char* caRegisterType_lic_name() {
    if (cartridgeState.header->newLicCode <= 0xA4) {
        return LIC_CODE[cartridgeState.header->licenseCode];
    }

    return "UNKNOWN";
}

const char* caRegisterType_type_name() {
    if (cartridgeState.header->type <= 0x22) {
        return ROM_TYPES[cartridgeState.header->type];
    }

    return "UNKNOWN";
}

bool LoadCartridge(char* cart) {
    snprintf(cartridgeState.filename, sizeof(cartridgeState.filename), "%s", cart);

    FILE *fp = fopen(cart, "r");

    if (!fp) {
        printf("Failed to open: %s\n", cart);
        return false;
    }

    printf("Opened: %s\n", cartridgeState.filename);

    fseek(fp, 0, SEEK_END);
    cartridgeState.romSize = ftell(fp);

    rewind(fp);

    cartridgeState.romData = malloc(cartridgeState.romSize);
    fread(cartridgeState.romData, cartridgeState.romSize, 1, fp);
    fclose(fp);

    cartridgeState.header = (romHeader*)(cartridgeState.romData + 0x100);
    cartridgeState.header->title[15] = 0;
    cartridgeState.battery = caRegisterType_battery();
    cartridgeState.needSave = false;

    printf("Cartridge Loaded:\n");
    printf("\t Title   : %s\n", cartridgeState.header->title);
    printf("\t Type    : %2.2X (%s)\n", cartridgeState.header->type, caRegisterType_type_name());
    printf("\t ROM Size: %d KB\n", 32 << cartridgeState.header->romSize);
    printf("\t RAM Size: %2.2X\n", cartridgeState.header->ramSize);
    printf("\t LIC Code: %2.2X (%s)\n", cartridgeState.header->licenseCode, caRegisterType_lic_name());
    printf("\t ROM Vers: %2.2X\n", cartridgeState.header->version);


    uint16_t x = 0;
    for (uint16_t i=0x0134; i<=0x014C; i++) {
        x = x - cartridgeState.romData[i] - 1;
    }

    printf("\t Checksum : %2.2X (%s)\n", cartridgeState.header->checksum, (x & 0xFF) ? "PASSED" : "FAILED");

    if (cartridgeState.battery) {
        batteryLoadCartridge();
    }

    return true;
}

void batteryLoadCartridge() {
    if (!cartridgeState.ramBank) {
        return;
    }

    char fn[1048];
    sprintf(fn, "%s.battery", cartridgeState.filename);
    FILE *fp = fopen(fn, "rb");

    if (!fp) {
        fprintf(stderr, "FAILED TO OPEN: %s\n", fn);
        return;
    }

    fread(cartridgeState.ramBank, 0x2000, 1, fp);
    fclose(fp);
}

void batterySaveCartridge() {
    if (!cartridgeState.ramBank) {
        return;
    }

    char fn[1048];
    sprintf(fn, "%s.battery", cartridgeState.filename);
    FILE *fp = fopen(fn, "wb");

    if (!fp) {
        fprintf(stderr, "FAILED TO OPEN: %s\n", fn);
        return;
    }

    fwrite(cartridgeState.ramBank, 0x2000, 1, fp);
    fclose(fp);
}

uint8_t readCartridge(uint16_t address) {

    if (!isMBC1() || address < 0x4000) {
        return cartridgeState.romData[address];
    }
    if ((address & 0xE000) == 0xA000) {
        if (!cartridgeState.ramEnabled) {
            return 0xFF;
        }
        if (!cartridgeState.ramBank) {
            return 0xFF;
        }

        return cartridgeState.ramBank[address - 0xA000];
    }

    // switchable ROM bank range
    return cartridgeState.romBankX[address - 0x4000];
}

void writeCartridge(uint16_t address, uint8_t value) {
    if (!isMBC1()) {
        return;
    }
    if (address < 0x2000) {
        cartridgeState.ramEnabled = ((value & 0xF) == 0xA);
    }
    if ((address & 0xE000) == 0x2000) {
        //rom bank number
        if (value == 0) {
            value = 1;
        }

        value &= 0b11111;

        cartridgeState.romBankValue = value;
        cartridgeState.romBankX = cartridgeState.romData + (0x4000 * cartridgeState.romBankValue);
    }

    if ((address & 0xE000) == 0x4000) {
        //ram bank number
        cartridgeState.ramBankValue = value & 0b11;

        if (cartridgeState.ramBanking) {
            if (needSaveCartridge()) {
                batterySaveCartridge();
            }

            cartridgeState.ramBank = cartridgeState.ramBanks[cartridgeState.ramBankValue];
        }
    }
    if ((address & 0xE000) == 0x6000) {
        //banking mode select
        cartridgeState.bankingMode = value & 1;

        cartridgeState.ramBanking = cartridgeState.bankingMode;

        if (cartridgeState.ramBanking) {
            if (needSaveCartridge()) {
                batterySaveCartridge();
            }

            cartridgeState.ramBank = cartridgeState.ramBanks[cartridgeState.ramBankValue];
        }
    }

    if ((address & 0xE000) == 0xA000) {
        if (!cartridgeState.ramEnabled) {
            return;
        }
        if (!cartridgeState.ramBank) {
            return;
        }

        cartridgeState.ramBank[address - 0xA000] = value;

        if (cartridgeState.battery) {
            cartridgeState.needSave = true;
        }
    }
}