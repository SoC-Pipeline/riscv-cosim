#include "print.h"
#include "sim.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void print_chr(char ch)
{
	sim_putc((unsigned int)ch);
}

void print_str(const char *p)
{
	while (*p != 0)
		print_chr(*(p++));
}

void print(char string_val[])
{
	print_str(string_val);
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

static void print_signed_dec(int val)
{
	unsigned int uval;
	if (val < 0) {
		print_chr('-');
		uval = (unsigned int)(-val);
	} else {
		uval = (unsigned int)val;
	}
	print_dec(uval);
}

void printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	while (*fmt != 0) {
		if (*fmt != '%') {
			print_chr(*fmt++);
			continue;
		}

		fmt++;
		switch (*fmt) {
		case 0:
			fmt--;
			break;
		case '%':
			print_chr('%');
			break;
		case 'c':
			print_chr((char)va_arg(ap, int));
			break;
		case 's': {
			const char *s = va_arg(ap, const char *);
			print_str(s ? s : "(null)");
			break;
		}
		case 'd':
			print_signed_dec(va_arg(ap, int));
			break;
		case 'u':
			print_dec(va_arg(ap, unsigned int));
			break;
		case 'x':
			print_hex(va_arg(ap, unsigned int), 8);
			break;
		case 'p':
			print_str("0x");
			print_hex((uintptr_t)va_arg(ap, void *), 8);
			break;
		default:
			print_chr('%');
			print_chr(*fmt);
			break;
		}

		if (*fmt != 0)
			fmt++;
	}
	va_end(ap);
}
