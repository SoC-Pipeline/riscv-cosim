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
	volatile unsigned int a = 0xDEADu;
	volatile unsigned int b = 0xBEADu;
	unsigned int c = a + b;

	print_str("a=");
	print_hex(a, 8);
	print_chr('\n');
	print_str("b=");
	print_hex(b, 8);
	print_chr('\n');
	print_str("c=");
	print_hex(c, 8);
	print_chr('\n');

	print_chr('\n');
	finish_case(a == 0xDEADu && b == 0xBEADu && c == 0x19D5Au);
}
