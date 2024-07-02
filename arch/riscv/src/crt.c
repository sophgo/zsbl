#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <assert.h>
#include <arch.h>
#include <timer.h>

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

extern uintptr_t __metal_init_hart;

void system_init(void)
{
	unsigned int hartid = current_hartid();

	timer_init();

	if (hartid == __metal_init_hart)
		run_init("early",
			(module_init_func *)__ld_early_init_start,
			(module_init_func *)__ld_early_init_end);

#ifdef CONFIG_PLAT
	pr_debug("Hello %s\n", CONFIG_PLAT);
#else
	pr_debug("Hello\n");
#endif

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


/* newlib stub */
#define _ssize_t ssize_t

static unsigned long heap_start;
static unsigned long heap_end;

void *_sbrk(unsigned long inc)
{
	void *last;

	if (heap_start == 0) {
		if ((unsigned long)__ld_bss_end & (0x7)) {
			heap_start = ((unsigned long)__ld_bss_end & ~(unsigned long)(0x7)) + 0x8;
		} else {
			heap_start = (unsigned long)__ld_bss_end;
		}

		heap_end = heap_start;
	}
	last = (void *)heap_end;
	if (inc & 0x7) {
		inc = (inc & ~(0x7)) + 0x8;
	}
	heap_end += inc;
	return last;
}
#if 0
struct _reent {
	int fd;
};

_ssize_t _write_r(struct _reent *ptr, int fd,
		  const void *buf, size_t cnt)
{
	size_t i;

	for (i = 0; i < cnt; ++i)
		stdio_output(((uint8_t *)buf)[i]);

	return cnt;
}

int _close_r(struct _reent *reent, int fd)
{
	return 0;
}

int _fstat_r(struct _reent *reent, int fd, struct stat *stat)
{
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		memset(stat, 0, sizeof(struct stat));
		stat->st_mode = S_IFCHR;
		return 0;
	}

	errno = EBADF;

	return -errno;
}

int _isatty_r(struct _reent *reent, int fd)
{
	return 1;
}

off_t _lseek_r(struct _reent *reent, int fd, off_t offset, int pos)
{
	return 0;
}

_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t len)
{
	return 0;
}

void _init(void)
{
}

void _exit(int n)
{
	printf("riscv terminated with code %d\n", n);
	while (true)
		;
}

void _kill(int pid, int sig)
{
	printf("kill %d with signal %d\n", pid, sig);
	_exit(-1);
}

int _getpid(void)
{
	return 0;
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
#endif