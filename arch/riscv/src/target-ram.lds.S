ENTRY(_start)

MEMORY
{
	RAM : ORIGIN = CONFIG_RAM_START, LENGTH = CONFIG_RAM_SIZE
}

SECTIONS
{
	. = ALIGN(16);

	.text :
	{
		PROVIDE(_fw_start = .);
		__ld_program_start = .;
		KEEP (*(.vector));
		KEEP (*(.exception_handler));
		*(.text);
		*(.text*);
		KEEP (*(.init));
		KEEP (*(.fini));
		. = ALIGN(16);
	} > RAM

	. = ALIGN(16);
	.rodata :
	{
		__ld_early_init_start = .;
		KEEP (*(.early_init));
		__ld_early_init_end = .;

		__ld_arch_init_start = .;
		KEEP (*(.arch_init));
		__ld_arch_init_end = .;

		__ld_plat_init_start = .;
		KEEP (*(.plat_init));
		__ld_plat_init_end = .;

		__ld_subsys_init_start = .;
		KEEP (*(.subsys_init));
		__ld_subsys_init_end = .;

		__ld_module_init_start = .;
		KEEP (*(.module_init));
		__ld_module_init_end = .;

		__ld_subsys_probe_start = .;
		KEEP (*(.subsys_probe));
		__ld_subsys_probe_end = .;

		__ld_test_case_start = .;
		KEEP (*(.test_case));
		__ld_test_case_end = .;

		. = ALIGN(16);
		*(.rodata);
		*(.rodata*);
		*(.srodata);
		*(.srodata*);
		. = ALIGN(16);
	} > RAM

	. = ALIGN(16);
	.preinit_array :
	{
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array*));
		PROVIDE_HIDDEN (__preinit_array_end = .);
		. = ALIGN(16);
	} > RAM

	. = ALIGN(16);
	.init_array :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)));
		KEEP (*(.init_array*));
		PROVIDE_HIDDEN (__init_array_end = .);
		. = ALIGN(16);
	} > RAM

	. = ALIGN(16);
	.fini_array :
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)));
		KEEP (*(.fini_array*));
		PROVIDE_HIDDEN (__fini_array_end = .);
		. = ALIGN(16);
	} > RAM

	. = ALIGN(16);
	.note :
	{
		*(.note.*);
		. = ALIGN(16);
	} > RAM

	. = ALIGN(16);
	.data :
	{
		__ld_data_start = .;
		*(.data);
		*(.data*);
		*(.sdata);
		*(.sdata*);
		. = ALIGN(16);
		__ld_data_end = .;
		__ld_program_end = .;
	} > RAM

	. = ALIGN(16);
	.bss :
	{
		__ld_bss_start = .;
		*(.bss);
		*(.bss*);
		*(.sbss);
		*(.sbss*);
		*(COMMON);
		. = ALIGN(16);
		__ld_bss_end = .;
	} > RAM

	.dynsym : {
		PROVIDE(__dyn_sym_start = .);
		*(.dynsym)
		PROVIDE(__dyn_sym_end = .);
	} > RAM

	.rela.dyn : {
		PROVIDE(__rel_dyn_start = .);
		*(.rela*)
		. = ALIGN(8);
		PROVIDE(__rel_dyn_end = .);
	} > RAM



	__ld_ram_start = ORIGIN(RAM);
	__ld_ram_end = ORIGIN(RAM) + LENGTH(RAM);
	__ld_data_load_start = LOADADDR(.data);
	__ld_data_load_end = __ld_data_load_start + SIZEOF(.data);
	__ld_stack_top = __ld_ram_end;
}


