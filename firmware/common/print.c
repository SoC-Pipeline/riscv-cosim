#include "print.h"

#define OUTPORT 0x10000000u

void print_chr(char ch)
{
	*((volatile unsigned int *)OUTPORT) = (unsigned int)ch;
}

void print_str(const char *p)
{
	while (*p != 0)
		print_chr(*(p++));
}

void print_dec(unsigned int val)
{
	char buffer[10];
	char *p = buffer;
	while (val || p == buffer) {
		*(p++) = val % 10;
		val = val / 10;
	}
	while (p != buffer)
		print_chr('0' + *(--p));
}

void print_hex(unsigned int val, int digits)
{
	for (int i = (4 * digits) - 4; i >= 0; i -= 4)
		print_chr("0123456789ABCDEF"[(val >> i) & 15]);
}
