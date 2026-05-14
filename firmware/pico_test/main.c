#include "print.h"

int main(void) {

	int a = 0xDEAD;
	int b = 0xBEAD;
	int c = a+b;

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
	print_str("PASS\n");

	return 0;
}
