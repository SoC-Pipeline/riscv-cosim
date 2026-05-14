	.section .text.start
	.global _start
	.global reset_vec
	.global main

reset_vec:
_start:
	lui sp, %hi(_stack_top)
	addi sp, sp, %lo(_stack_top)

	jal ra, sim_start
	jal ra, main

	j sim_exit

1:	j 1b
