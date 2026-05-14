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
	volatile unsigned int value = 1234u;
	volatile unsigned int hex_value = 0xdeadbeefu;
	int pass = (value == 1234u) && (hex_value == 0xdeadbeefu);

	print_str("\n-------------------\n");
	print_str("hello world\n");
	print_str("value=");
	print_dec(value);
	print_chr('\n');
	print_str("hex=");
	print_hex(hex_value, 8);
	print_str("\n-------------------\n");

	finish_case(pass);
}
