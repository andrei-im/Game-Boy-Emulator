#include "MemoryBus.h"
#include "Cartridge.h"
#include "RAM.h"
#include "CPU.h"
#include <io.h>
#include "GPU.h"
#include "DirectMemoryAccess.h"

uint8_t io_read(uint16_t address);
void io_write(uint16_t address, uint8_t value);

uint8_t ReadByteMemBus(uint16_t address) {
    if (address < 0x8000) {
        //ROM Data
        return readCartridge(address);
    } else if (address < 0xA000) {
        //Char/Map Data
        return readVRAM(address);
    } else if (address < 0xC000) {
        //Cartridge RAM
        return readCartridge(address);
    } else if (address < 0xE000) {
        //WRAM (Working RAM)
        return readWRAM(address);
    } else if (address < 0xFE00) {
        //reserved echo ram...
        return 0;
    } else if (address < 0xFEA0) {
        //OAM
        if (transferringDMA()) {
            return 0xFF;
        }

        return readOAM(address);
    } else if (address < 0xFF00) {
        //reserved unusable...
        return 0;
    } else if (address < 0xFF80) {
        //IO Registers...
        return io_read(address);
    } else if (address == 0xFFFF) {
        //CPU ENABLE REGISTER...
        return getIERegister();
    }

    //NO_IMPL
    return readHRAM(address);
}

void WriteByteMemBus(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        //ROM Data
        writeCartridge(address, value);
    } else if (address < 0xA000) {
        //Char/Map Data
        writeVRAM(address, value);
    } else if (address < 0xC000) {
        //EXT-RAM
        writeCartridge(address, value);
    } else if (address < 0xE000) {
        //WRAM
        writeWRAM(address, value);
    } else if (address < 0xFE00) {
        //reserved echo ram
    } else if (address < 0xFEA0) {
        //OAM
        if (transferringDMA()) {
            return;
        }

        writeOAM(address, value);
    } else if (address < 0xFF00) {
        //unusable reserved
    } else if (address < 0xFF80) {
        //IO Registers...
        io_write(address, value);
    } else if (address == 0xFFFF) {
        //CPU SET ENABLE REGISTER

        setIERegister(value);
    } else {
        writeHRAM(address, value);
    }
}

uint16_t ReadWordMemBus(uint16_t address) {
    uint16_t lo = ReadByteMemBus(address);
    uint16_t hi = ReadByteMemBus(address + 1);

    return lo | (hi << 8);
}

void WriteWordMemBus(uint16_t address, uint16_t value) {
    WriteByteMemBus(address + 1, (value >> 8) & 0xFF);
    WriteByteMemBus(address, value & 0xFF);
}