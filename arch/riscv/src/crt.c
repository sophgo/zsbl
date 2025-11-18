#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <arch.h>
#include <timer.h>

#include <common/module.h>
#include <common/common.h>

#include <lib/cli.h>

/* #define DEBUG_IRQ */
/* #define DEBUG_FIQ */

extern unsigned long __ld_ram_start[0], __ld_ram_end[0];
extern unsigned long __ld_bss_start[0], __ld_bss_end[0];
extern unsigned long __ld_data_start[0], __ld_data_end[0];
extern unsigned long __ld_program_start[0], __ld_program_end[0];
extern unsigned long __ld_data_load_start[0], __ld_data_load_end[0];
extern unsigned long __ld_stack_top[0];
extern unsigned long exception_handler[0];

extern unsigned long __ld_early_init_start[0], __ld_early_init_end[0];
extern unsigned long __ld_arch_init_start[0], __ld_arch_init_end[0];
extern unsigned long __ld_plat_init_start[0], __ld_plat_init_end[0];
extern unsigned long __ld_subsys_init_start[0], __ld_subsys_init_end[0];
extern unsigned long __ld_module_init_start[0], __ld_module_init_end[0];
extern unsigned long __ld_subsys_probe_start[0], __ld_subsys_probe_end[0];
extern unsigned long __ld_late_init_start[0], __ld_late_init_end[0];

void load_data(void)
{
	volatile uint128_t *s, *d, *e;

	if ((unsigned long)__ld_data_load_start == (unsigned long)__ld_data_start)
		return;

	s = (uint128_t *)__ld_data_load_start;
	d = (uint128_t *)__ld_data_start;
	e = (uint128_t *)__ld_data_end;

	while (d != e) {
		*d = *s;
		++s;
		++d;
	}
}

void  clear_bss(void)
{
	volatile uint128_t *p = (uint128_t *)__ld_bss_start;

	while ((unsigned long)p != (unsigned long)__ld_bss_end) {
		*p = 0;
		++p;
	}
}

static int run_init(const char *name,
		    module_init_func *start, module_init_func *end)
{
	int err;
	module_init_func *fn;

	if (stdout_ready())
		pr_debug("%s init\n", name);

	for (fn = (module_init_func *)start;
	     fn != (module_init_func *)end;
	     ++fn) {
		err = (*fn)();
		if (err) {
			if (stdout_ready())
				pr_err("function %016lx failed with code %d\n",
				       (unsigned long)fn, err);

			while (1)
				asm volatile ("wfi");
		}
	}

	return 0;
}

void system_init(void)
{
	arch_enable_local_irq();

	timer_init();

	run_init("early",
		 (module_init_func *)__ld_early_init_start,
		 (module_init_func *)__ld_early_init_end);

	pr_debug("runtime space %016lx - %016lx\n",
	       (unsigned long)__ld_ram_start,
	       (unsigned long)__ld_ram_end);

	run_init("arch",
		 (module_init_func *)__ld_arch_init_start,
		 (module_init_func *)__ld_arch_init_end);

	run_init("platform",
		 (module_init_func *)__ld_plat_init_start,
		 (module_init_func *)__ld_plat_init_end);

	run_init("subsystem",
		 (module_init_func *)__ld_subsys_init_start,
		 (module_init_func *)__ld_subsys_init_end);

	run_init("module",
		 (module_init_func *)__ld_module_init_start,
		 (module_init_func *)__ld_module_init_end);

	run_init("subsystem probe",
		 (module_init_func *)__ld_subsys_probe_start,
		 (module_init_func *)__ld_subsys_probe_end);

	run_init("late",
		 (module_init_func *)__ld_late_init_start,
		 (module_init_func *)__ld_late_init_end);
}

static unsigned long heap_start;
static unsigned long heap_end;

#define MEMALIGN	16

void *_sbrk(intptr_t inc)
{
	void *last;

	if (heap_start == 0) {
		heap_start = ROUND_UP((unsigned long)__ld_bss_end, MEMALIGN);
		heap_end = heap_start;
	}

	last = (void *)heap_end;

	inc = ROUND_UP(inc, MEMALIGN);

	heap_end += inc;

	if (heap_end > (unsigned long)__ld_ram_end)
		return (void *)-1;

	memset(last, 0, inc);


	return last;
}

void *sbrk(intptr_t inc)
{
	return _sbrk(inc);
}

void _exit(int n)
{
	/* disable all interrupts */
	csr_write(CSR_MSTATUS, csr_read(CSR_MSTATUS) & ~(1 << 3));
	printf("RISC-V terminated with code %d\n", n);
	while (true)
		;
}

/* stack protection stub */
uintptr_t __stack_chk_guard = 0xdeadbeef;

void __stack_chk_fail(void)
{
	printf("Stack Overflow\n");
	asm volatile ("ebreak");
}

void show_memmap(void)
{
#define show_region(name, start, end)	\
	printf("%-10s %012lx %012lx\n", name, (unsigned long)(start), (unsigned long)(end))

	show_region("RAM",__ld_ram_start, __ld_ram_end);
	show_region("TEXT", __ld_program_start, __ld_data_start);
	show_region("DATA", __ld_data_start, __ld_data_end);
	show_region("BSS", __ld_bss_start, __ld_bss_end);
	show_region("STACK", __ld_bss_end, __ld_stack_top);
	show_region("HEAP", heap_start, heap_end);
}

static void command_show_memmap(struct command *c, int argc, const char *argv[])
{
	show_memmap();
}

cli_command(memmap, command_show_memmap);

