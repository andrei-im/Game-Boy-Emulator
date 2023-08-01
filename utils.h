#ifndef GBEMU_UTILS_H
#define GBEMU_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

#define IS_BIT_SET(a, n) ((a & (1 << n)) ? 1 : 0)

#define SET_OR_CLEAR_BIT(a, n, on) { if (on) (a |= (1 << n)); else (a &= ~(1 << n));}

#define IS_IN_RANGE(a, b, c) ((a >= b) && (a <= c))


uint32_t getTicks();
void delay(uint32_t ms);

#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }

#endif //GBEMU_UTILS_H
