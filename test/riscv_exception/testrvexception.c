#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>

#include <spinlock.h>
#include <smp.h>
#include <sbi/riscv_asm.h>
#include <thread_safe_printf.h>

#define STACK_SIZE 4096

typedef struct {
	uint8_t stack[STACK_SIZE];
} core_stack;

static core_stack __unused secondary_core_stack[CONFIG_SMP_NUM];

static void __unused secondary_core_fun(void *priv)
{
	while (1) {
		thread_safe_printf("hart id %u print hello world\n", current_hartid());

		mdelay(1000);
	}
}

static int testrvexception(void)
{
	unsigned int hartid = current_hartid();

	thread_safe_printf("main core id = %u\n", hartid);

	uint64_t *point = (uint64_t *)0;
	*point = 1;

	while (1) {
		thread_safe_printf("hello, main core id = %u\n", current_hartid());

		mdelay(1000);
	};

	return 0;
}

test_case(testrvexception);
