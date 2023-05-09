#include <stdio.h>
#include "timer.h"
#include "irq.h"
#include "arch.h"
#include "framework/module.h"

#define GIC_TEST_DEBUG
#ifdef GIC_TEST_DEBUG
#define gic_test_dbg(x, args...) printf("[%s] - " x, __func__, ##args)
#else
#define gic_test_dbg(x, args...)
#endif

static volatile int mark;
static volatile int cur_irq = -1;

static int irq_handler(int irqn, void *priv)
{
	gic_test_dbg("irq num:%d\n", irqn);
	mark++;

	irq_clear(irqn);

	return 0;
}

static int check_irq_handler(int timeout)
{
	int i;

	for (i = 0; i < timeout; ++i) {
		if (mark > 0)
			return 0;
		mdelay(1);
	}

	return -1;
}

int testcase_gic(void)
{
	int irq;
	cur_irq = irq_get_nums() - 1;
	gic_test_dbg("gic spi interrupt nums:%d\n", cur_irq);
	for (; cur_irq >= 0; --cur_irq) {
		mark = 0;
#if 1
		irq = cur_irq;
#else
		irq = 247;
#endif
		request_irq(irq, irq_handler, 0, "software int", NULL);
		irq_trigger(irq);
		if (check_irq_handler(1000) != 0) {
			gic_test_dbg("test err int-num:%d\n", irq);
			return -1;
		}
	}

	printf("test gic passed!\n");

	return 0;
}

test_case(testcase_gic);
