#ifndef FIRMWARE_COMMON_SIM_H
#define FIRMWARE_COMMON_SIM_H

#define SIM_MMIO_ADDR 0x10000000u

#define RISCV_QUIT   0x00u
#define RISCV_START  0x80u
#define RISCV_FINISH 0x81u
#define RISCV_FAIL   0x82u
#define RISCV_PASS   0x83u

void sim_putc(unsigned int value);
void sim_start(void);
void sim_pass(void);
void sim_fail(void);
void sim_finish(void);
void sim_exit(int status) __attribute__((noreturn));

#endif
