ENTRY(_start)

MEMORY
{
	RAM : ORIGIN = CONFIG_RAM_START, LENGTH = CONFIG_RAM_SIZE
	ROM : ORIGIN = CONFIG_ROM_START, LENGTH = CONFIG_ROM_SIZE
}

SECTIONS
{
	.text :
	{
		. = ALIGN(16);
		PROVIDE(__ld_program_start = .);
		KEEP (*(.vector));
		KEEP (*(.exception_handler));
		*(.text);
		*(.text*);
		KEEP (*(.init));
		KEEP (*(.fini));
		. = ALIGN(16);
	} > ROM

	.rodata :
	{
		. = ALIGN(16);
		PROVIDE(__ld_early_init_start = .);
		KEEP (*(.early_init));
		PROVIDE(__ld_early_init_end = .);

		PROVIDE(__ld_arch_init_start = .);
		KEEP (*(.arch_init));
		PROVIDE(__ld_arch_init_end = .);

		PROVIDE(__ld_plat_init_start = .);
		KEEP (*(.plat_init));
		PROVIDE(__ld_plat_init_end = .);

		PROVIDE(__ld_subsys_init_start = .);
		KEEP (*(.subsys_init));
		PROVIDE(__ld_subsys_init_end = .);

		PROVIDE(__ld_module_init_start = .);
		KEEP (*(.module_init));
		PROVIDE(__ld_module_init_end = .);

		PROVIDE(__ld_subsys_probe_start = .);
		KEEP (*(.subsys_probe));
		PROVIDE(__ld_subsys_probe_end = .);

		PROVIDE(__ld_late_init_start = .);
		KEEP (*(.late_init));
		PROVIDE(__ld_late_init_end = .);

		PROVIDE(__ld_cli_command_start = .);
		KEEP (*(.cli_command));
		PROVIDE(__ld_cli_command_end = .);

		PROVIDE(__ld_test_case_start = .);
		KEEP (*(.test_case));
		PROVIDE(__ld_test_case_end = .);

		. = ALIGN(16);
		*(.rodata);
		*(.rodata*);
		*(.srodata);
		*(.srodata*);
		. = ALIGN(16);
	} > ROM

	.preinit_array :
	{
		. = ALIGN(16);
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array*));
		PROVIDE_HIDDEN (__preinit_array_end = .);
		. = ALIGN(16);
	} > ROM

	.init_array :
	{
		. = ALIGN(16);
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)));
		KEEP (*(.init_array*));
		PROVIDE_HIDDEN (__init_array_end = .);
		. = ALIGN(16);
	} > ROM

	.fini_array :
	{
		. = ALIGN(16);
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)));
		KEEP (*(.fini_array*));
		PROVIDE_HIDDEN (__fini_array_end = .);
		. = ALIGN(16);
	} > ROM

	.note :
	{
		. = ALIGN(16);
		*(.note.*);
		. = ALIGN(16);
	} > ROM

	.data :
	{
		. = ALIGN(16);
		PROVIDE(__ld_data_start = .);
		*(.data);
		*(.data*);
		*(.sdata);
		*(.sdata*);
		. = ALIGN(16);
		PROVIDE(__ld_data_end = .);
		PROVIDE(__ld_program_end = .);
	} > RAM AT > ROM

	.bss :
	{
		. = ALIGN(16);
		PROVIDE(__ld_bss_start = .);
		*(.bss);
		*(.bss*);
		*(.sbss);
		*(.sbss*);
		*(COMMON);
		. = ALIGN(16);
		PROVIDE(__ld_bss_end = .);
	} > RAM

	PROVIDE(__ld_ram_start = ORIGIN(RAM));
	PROVIDE(__ld_ram_end = ORIGIN(RAM) + LENGTH(RAM));
	PROVIDE(__ld_data_load_start = LOADADDR(.data));
	PROVIDE(__ld_data_load_end = __ld_data_load_start + SIZEOF(.data));
	PROVIDE(__ld_stack_top = __ld_ram_end);
}


