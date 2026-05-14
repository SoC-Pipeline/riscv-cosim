#ifndef FIRMWARE_COMMON_SIM_SHOW_H
#define FIRMWARE_COMMON_SIM_SHOW_H

#include "sim.h"

#define LED_REG_BASE SIM_MMIO_ADDR

void print(char string_val[]);
void printf(const char *fmt, ...);

#endif
