#include "print.h"
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
	volatile unsigned int *p0 = (volatile unsigned int *)0x80010000u;
	volatile unsigned int *p1 = (volatile unsigned int *)0x80010004u;
	volatile unsigned int *p2 = (volatile unsigned int *)0x80010008u;
	unsigned int expected = 0x11223344u ^ 0xa5a55a5au;

	*p0 = 0x11223344u;
	*p1 = 0xa5a55a5au;
	*p2 = *p0 ^ *p1;

	print_str("mem0=");
	print_hex(*p0, 8);
	print_chr('\n');
	print_str("mem1=");
	print_hex(*p1, 8);
	print_chr('\n');
	print_str("mem2=");
	print_hex(*p2, 8);
	print_chr('\n');

	finish_case(*p0 == 0x11223344u && *p1 == 0xa5a55a5au && *p2 == expected);
}
