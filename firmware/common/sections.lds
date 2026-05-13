ENTRY(_start)

MEMORY {
	mem : ORIGIN = 0x80000000, LENGTH = 0x00018000
}

SECTIONS {
	.memory : {
		. = ORIGIN(mem) + 0x80;
		*(.text.start);
		*(.text);
		*(.text*);
		*(.rodata*);
		*(.srodata*);
		*(.data*);
		*(.sdata*);
		. = ALIGN(4);
		_bss_start = .;
		*(.bss*);
		*(.sbss*);
		*(COMMON);
		. = ALIGN(4);
		_bss_end = .;
		end = .;
	} > mem

	_stack_top = ORIGIN(mem) + LENGTH(mem);
}
