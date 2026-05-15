#include "sim.h"

static void finish_case(int pass) __attribute__((noreturn));

static void finish_case(int pass)
{
	if (pass)
		sim_pass();
	else
		sim_fail();

	while (1) {
		__asm__ volatile ("");
	}
}

int main(void)
{
	__asm__ volatile ("rdcycle x0");
	__asm__ volatile ("rdinstret x0");
	finish_case(1);
}
