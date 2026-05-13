#include "print.h"

int main(void)
{
	print_str("\n-------------------\n");
	print_str("hello world\n");
	print_str("value=");
	print_dec(1234);
	print_chr('\n');
	print_str("hex=");
	print_hex(0xdeadbeef, 8);
	print_str("\n-------------------\n");
	return 0;
}
