#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <common/module.h>
#include <common/common.h>
#include <lib/libc/errno.h>

#include <spinlock.h>
#include <smp.h>
#include <asm.h>

#define STACK_SIZE 4096

typedef struct {
	uint8_t stack[STACK_SIZE];
} core_stack;
static core_stack secondary_core_stack[CONFIG_SMP_NUM];

static void secondary_core_fun(void *priv)
{
	while (1) {
		printf("hart id %u print hello world\n", current_hartid());

		mdelay(1000);
	}
}

static int testsmp(void)
{
	unsigned int hartid = current_hartid();

	printf("main core id = %u\n", hartid);

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, secondary_core_fun, NULL,
				   &secondary_core_stack[i], STACK_SIZE);
	}
	while (1) {
		printf("hello, main core id = %u\n", current_hartid());

		mdelay(1000);
	};

	return 0;
}

test_case(testsmp);
