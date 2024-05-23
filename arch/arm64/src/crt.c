#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <arch.h>
#include <cache.h>
#include <timer.h>
#include <sysreg.h>
#include <smp.h>
#include <framework/module.h>
#include <framework/common.h>

/* #define DEBUG_IRQ */
/* #define DEBUG_FIQ */

extern unsigned long __ld_ram_start[0], __ld_ram_end[0];
extern unsigned long __ld_bss_start[0], __ld_bss_end[0];
extern unsigned long __ld_data_start[0], __ld_data_end[0];
extern unsigned long __ld_data_load_start[0], __ld_data_load_end[0];
extern unsigned long __ld_stack_top[0];
extern unsigned long exception_handler[0];

extern unsigned long __ld_early_init_start[0], __ld_early_init_end[0];
extern unsigned long __ld_arch_init_start[0], __ld_arch_init_end[0];
extern unsigned long __ld_plat_init_start[0], __ld_plat_init_end[0];
extern unsigned long __ld_module_init_start[0], __ld_module_init_end[0];

void load_data(void)
{
	volatile uint128_t *s, *d, *e;

	if (__ld_data_load_start == __ld_data_start)
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
	enable_caches(default_memory_map);
	timer_init();

	run_init("early",
		 (module_init_func *)__ld_early_init_start,
		 (module_init_func *)__ld_early_init_end);

	/* a53lite must run at el3 */
	assert(current_el() == 3);

#ifdef CONFIG_PLAT
	pr_debug("Hello %s\n", CONFIG_PLAT);
#else
	pr_debug("Hello\n");
#endif

	pr_debug("disable neon exception\n");
	write_sysreg(cptr_el3, 0);

	pr_debug("set exception handler\n");
	write_sysreg(vbar_el3, (unsigned long)exception_handler);

	pr_debug("disable irq and fiq, enable serror and debug exception\n");
	asm volatile ("msr	daifclr, #0x0c");
	asm volatile ("msr	daifset, #0x03");

	pr_debug("enable irq and fiq in el3\n");
	write_sysreg(scr_el3,
			read_sysreg(scr_el3) | (1 << 1) | (1 << 2));

	pr_debug("runtime space %016lx - %016lx\n",
	       (unsigned long)__ld_ram_start,
	       (unsigned long)__ld_ram_end);

	run_init("arch",
		 (module_init_func *)__ld_arch_init_start,
		 (module_init_func *)__ld_arch_init_end);

	run_init("platform",
		 (module_init_func *)__ld_plat_init_start,
		 (module_init_func *)__ld_plat_init_end);

	run_init("module",
		 (module_init_func *)__ld_module_init_start,
		 (module_init_func *)__ld_module_init_end);

}

void exception_dump(struct context_frame *ctx)
{
	int i;
	unsigned long el = read_sysreg(currentel) >> 2;

	switch (el) {
	case 1:
		ctx->spsr = read_sysreg(spsr_el1);
		ctx->elr = read_sysreg(elr_el1);
		break;
	case 2:
		ctx->spsr = read_sysreg(spsr_el2);
		ctx->elr = read_sysreg(elr_el2);
		break;
	case 3:
		ctx->spsr = read_sysreg(spsr_el3);
		ctx->elr = read_sysreg(elr_el3);
		break;
	default:
		break;
	}

	printf("\n\n*************************************************\n");
	printf("exception occur in el%ld\n", (ctx->spsr >> 2) & 0x03);
	printf("exception handled by el%ld\n\n", el);

	for (i = 0; i < 30; ++i)
		printf("%sx%d: %016lx%c",
		       i < 10 ? " " : "",
		       i, ctx->x[i], i % 2 == 0 ? ' ' : '\n');

	printf("\n");

	printf("lr: %016lx\n", ctx->lr);
	printf("sp: %016lx\n", ctx->sp);
	printf("spsr: %016lx\n", ctx->spsr);
	printf("elr: %016lx\n", ctx->elr);

	printf("*************************************************\n");

}

void serror_dump(struct context_frame *ctx)
{
	int i;
	unsigned long el = read_sysreg(currentel) >> 2;

	switch (el) {
	case 1:
		ctx->spsr = read_sysreg(spsr_el1);
		ctx->elr = read_sysreg(elr_el1);
		break;
	case 2:
		ctx->spsr = read_sysreg(spsr_el2);
		ctx->elr = read_sysreg(elr_el2);
		break;
	case 3:
		ctx->spsr = read_sysreg(spsr_el3);
		ctx->elr = read_sysreg(elr_el3);
		break;
	default:
		break;
	}

	printf("\n\n*************************************************\n");
	printf("serror occur in el%ld\n", (ctx->spsr >> 2) & 0x03);
	printf("serror handled by el%ld\n\n", el);

	for (i = 0; i < 30; ++i)
		printf("%sx%d: %016lx%c",
		       i < 10 ? " " : "",
		       i, ctx->x[i], i % 2 == 0 ? ' ' : '\n');

	printf("\n");

	printf("lr: %016lx\n", ctx->lr);
	printf("sp: %016lx\n", ctx->sp);
	printf("spsr: %016lx\n", ctx->spsr);
	printf("elr: %016lx\n", ctx->elr);

	printf("*************************************************\n");

}

static void (*arch_irq_handler)(void);
static void (*arch_fiq_handler)(void);

void set_handle_irq(void (*irq_handler)(void))
{
	assert(arch_irq_handler == NULL);

	arch_irq_handler = irq_handler;
}

void set_handle_fiq(void (*fiq_handler)(void))
{
	assert(arch_fiq_handler == NULL);

	arch_fiq_handler = fiq_handler;
}

int spin_trylock(spinlock_t *lock)
{
	int old, tmp0, tmp1;

	asm volatile ("1:	ldxr	%w0, %3\n"
		      "		cbnz	%w0, 2f\n"	/* out */
		      "		add	%w1, %w0, #1\n"
		      "		stxr	%w2, %w1, %3\n"
		      "		cbnz	%w2, 1b\n"
		      "2:\n"
		      : "=r"(old), "=r"(tmp0), "=r"(tmp1), "+Q"(lock->lock)
		      :
		      : "memory"
		      );

	/* 0 means get lock success
	 * 1 means get lock failed
	 */

	return !old;
}

void irq_entry(struct context_frame *ctx)
{
#ifdef DEBUG_IRQ
	int i;
	unsigned long el = read_sysreg(currentel) >> 2;

	switch (el) {
	case 1:
		ctx->spsr = read_sysreg(spsr_el1);
		ctx->elr = read_sysreg(elr_el1);
		break;
	case 2:
		ctx->spsr = read_sysreg(spsr_el2);
		ctx->elr = read_sysreg(elr_el2);
		break;
	case 3:
		ctx->spsr = read_sysreg(spsr_el3);
		ctx->elr = read_sysreg(elr_el3);
		break;
	default:
		break;
	}

	pr_debug("\n\n*************************************************\n");
	pr_debug("interrupt occur in el%ld\n", (ctx->spsr >> 2) & 0x03);
	pr_debug("interrupt handled by el%ld\n\n", el);

	for (i = 0; i < 30; ++i)
		pr_debug("%sx%d: %016lx%c",
		       i < 10 ? " " : "",
		       i, ctx->x[i], i % 2 == 0 ? ' ' : '\n');

	pr_debug("\n");

	pr_debug("lr: %016lx\n", ctx->lr);
	pr_debug("sp: %016lx\n", ctx->sp);
	pr_debug("spsr: %016lx\n", ctx->spsr);
	pr_debug("elr: %016lx\n", ctx->elr);

	pr_debug("*************************************************\n");
#endif

	assert(arch_irq_handler);

	arch_irq_handler();
}

void fiq_entry(struct context_frame *ctx)
{
#ifdef DEBUG_FIQ
	int i;
	unsigned long el = read_sysreg(currentel) >> 2;

	switch (el) {
	case 1:
		ctx->spsr = read_sysreg(spsr_el1);
		ctx->elr = read_sysreg(elr_el1);
		break;
	case 2:
		ctx->spsr = read_sysreg(spsr_el2);
		ctx->elr = read_sysreg(elr_el2);
		break;
	case 3:
		ctx->spsr = read_sysreg(spsr_el3);
		ctx->elr = read_sysreg(elr_el3);
		break;
	default:
		break;
	}

	pr_debug("\n\n*************************************************\n");
	pr_debug("fiq occur in el%ld\n", (ctx->spsr >> 2) & 0x03);
	pr_debug("fiq handled by el%ld\n\n", el);

	for (i = 0; i < 30; ++i)
		pr_debug("%sx%d: %016lx%c",
		       i < 10 ? " " : "",
		       i, ctx->x[i], i % 2 == 0 ? ' ' : '\n');

	pr_debug("\n");

	pr_debug("lr: %016lx\n", ctx->lr);
	pr_debug("sp: %016lx\n", ctx->sp);
	pr_debug("spsr: %016lx\n", ctx->spsr);
	pr_debug("elr: %016lx\n", ctx->elr);

	pr_debug("*************************************************\n");
#endif

	assert(arch_fiq_handler);

	arch_fiq_handler();
}

/* newlib stub */

static unsigned long heap_start;
static unsigned long heap_end;

void *_sbrk(unsigned long inc)
{
	void *last;

	if (heap_start == 0) {
		heap_start = (unsigned long)__ld_bss_end;
		heap_end = heap_start;
	}
	last = (void *)heap_end;
	heap_end += inc;
	return last;
}

void _exit(int n)
{
	printf("a53lite terminated with code %d\n", n);
	while (true)
		;
}

void *__stack_chk_guard = (void *)0xdeadbeef;

void __stack_chk_fail(void) {
	printf("Stack smashing detected.\n");
	while (true)
		;
}

#ifdef CONFIG_TARGET_EMULATOR

/* overwrite default output functions
 * waiting uart output is not happy for emulator
 */
int printf(const char *format, ...)
{
	return 0;
}

int puts(const char *s)
{
	return 0;
}

#endif
