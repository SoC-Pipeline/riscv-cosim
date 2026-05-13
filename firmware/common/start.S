	.section .text.start
	.global _start
	.global reset_vec
	.global main

reset_vec:
_start:
	lui sp, %hi(_stack_top)
	addi sp, sp, %lo(_stack_top)

	jal ra, main

	li a0, 0x20000000
	li a1, 123456789
	sw a1, 0(a0)

1:	j 1b
