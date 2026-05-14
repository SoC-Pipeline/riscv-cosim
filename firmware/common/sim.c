#include "sim.h"

void sim_putc(unsigned int value)
{
	*((volatile unsigned int *)SIM_MMIO_ADDR) = value;
}

static void sim_mmio_barrier(void)
{
	__asm__ volatile ("fence iorw, iorw" ::: "memory");
}

void sim_start(void)
{
	sim_putc(RISCV_START);
}

void sim_pass(void)
{
	sim_putc(RISCV_PASS);
	sim_mmio_barrier();
	sim_putc(RISCV_QUIT);
}

void sim_fail(void)
{
	sim_putc(RISCV_FAIL);
	sim_mmio_barrier();
	sim_putc(RISCV_QUIT);
}

void sim_finish(void)
{
	sim_putc(RISCV_FINISH);
	sim_mmio_barrier();
	sim_putc(RISCV_QUIT);
}

void sim_exit(int status)
{
	if (status == 0)
		sim_pass();
	else
		sim_fail();

	while (1) {
		__asm__ volatile ("");
	}
}
