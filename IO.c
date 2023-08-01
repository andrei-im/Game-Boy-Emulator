#include "IO.h"
#include "LCD.h"
#include "Timer.h"
#include "CPU.h"
#include "Input.h"

static char serial_data[2];

uint8_t io_read(uint16_t address) {
    if (address == 0xFF00) {
        return getGamepadOutput();
    }

    if (address == 0xFF01) {
        return serial_data[0];
    }

    if (address == 0xFF02) {
        return serial_data[1];
    }

    if (IS_IN_RANGE(address, 0xFF04, 0xFF07)) {
        return readTimer(address);
    }

    if (address == 0xFF0F) {
        return getInterruptFlags();
    }

    if (IS_IN_RANGE(address, 0xFF10, 0xFF3F)) {
        //ignore sound
        return 0;
    }

    if (IS_IN_RANGE(address, 0xFF40, 0xFF4B)) {
        return readLCD(address);
    }

//    printf("UNSUPPORTED bus read(%04X)\n", address);
    return 0;
}

void io_write(uint16_t address, uint8_t value) {
    if (address == 0xFF00) {
        setGamepadSelection(value);
        return;
    }

    if (address == 0xFF01) {
        serial_data[0] = value;
        return;
    }

    if (address == 0xFF02) {
        serial_data[1] = value;
        return;
    }

    if (IS_IN_RANGE(address, 0xFF04, 0xFF07)) {
        writeTimer(address, value);
        return;
    }

    if (address == 0xFF0F) {
        setInterruptFlags(value);
        return;
    }

    if (IS_IN_RANGE(address, 0xFF10, 0xFF3F)) {
        //ignore sound
        return;
    }

    if (IS_IN_RANGE(address, 0xFF40, 0xFF4B)) {
        writeLCD(address, value);
        return;
    }

//    printf("UNSUPPORTED bus write(%04X)\n", address);
}