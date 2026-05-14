#include "print.h"

int main(void)
{
	volatile unsigned int *p0 = (volatile unsigned int *)0x80010000u;
	volatile unsigned int *p1 = (volatile unsigned int *)0x80010004u;
	volatile unsigned int *p2 = (volatile unsigned int *)0x80010008u;

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

	if (*p0 != 0x11223344u || *p1 != 0xa5a55a5au || *p2 != (0x11223344u ^ 0xa5a55a5au)) {
		print_str("FAIL\n");
		return 1;
	}

	print_str("PASS\n");
	return 0;
}
