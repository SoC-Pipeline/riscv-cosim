#ifndef FIRMWARE_COMMON_PRINT_H
#define FIRMWARE_COMMON_PRINT_H

void print_chr(char ch);
void print_str(const char *p);
void print(char string_val[]);
void print_dec(unsigned int val);
void print_hex(unsigned int val, int digits);
void printf(const char *fmt, ...);

#endif
