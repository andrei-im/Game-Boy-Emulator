#ifndef GBEMU_IO_H
#define GBEMU_IO_H

#include "utils.h"

uint8_t io_read(uint16_t address);
void io_write(uint16_t address, uint8_t value);

#endif //GBEMU_IO_H
