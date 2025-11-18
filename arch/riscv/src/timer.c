#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <timer.h>

#include <common/module.h>
#include <common/common.h>
#include <driver/platform.h>

uint64_t timer_frequency(void)
{
#ifndef CONFIG_ARCH_TIMER_FREQ
#error "we need timer frequency"
#else
	return CONFIG_ARCH_TIMER_FREQ;
#endif
}

uint64_t timer_get_tick(void)
{
#ifdef CONFIG_PLAT_QEMU
	return *(unsigned long *)(0x200bff8);
#else
	return csr_read(CSR_TIME);
#endif
}

void timer_delay_tick(uint64_t tick)
{
	uint64_t start = timer_get_tick();
	uint64_t end;

	do {
		end = timer_get_tick();
	} while (end - start < tick);
}

void timer_mdelay(uint32_t ms)
{
	timer_delay_tick((uint64_t)timer_frequency() * ms / 1000);
}

void timer_udelay(uint32_t us)
{
	timer_delay_tick((uint64_t)timer_frequency() * us / (1000 * 1000));

}

uint64_t timer_tick2us(uint64_t tick)
{
	return tick * 1000 * 1000 / timer_frequency();
}

uint64_t timer_tick2ms(uint64_t tick)
{
	return tick * 1000 / timer_frequency();
}

uint64_t timer_tick2s(uint64_t tick)
{
	return tick / timer_frequency();
}

uint64_t timer_us2tick(uint64_t us)
{
	return timer_frequency() * us / 1000 / 1000;
}

uint64_t timer_ms2tick(uint64_t ms)
{
	return timer_frequency() * ms / 1000;
}

uint64_t timer_s2tick(uint64_t s)
{
	return timer_frequency() * s;
}

void timer_init(void)
{
}

#ifdef CONFIG_DRIVER_PLATFORM
/* timer late init, enable timer interrupts */
/* assume hart id is contigurous */

#ifdef CONFIG_SMP
#define ISR_NUM	CONFIG_SMP_NUM
static uint64_t get_isr_index(void)
{
	return current_hartid();
}
#else
#define ISR_NUM	1
static uint64_t get_isr_index(void)
{
	return 0;
}
#endif

enum {
	TIMER_TYPE_CLINT,
	TIMER_TYPE_ACLINT,
	TIMER_TYPE_SSTC
};

#define CLINT_MTIMECMP_OFFSET	0x4000

struct timer_ctx {
	struct {
		int type;
		void *base;
	} mtimecmp;

	struct {
		void (*func)(void *data);
		void *data;
		uint64_t tick;
		void *mtimecmp;
	} isr[ISR_NUM];
};

/* arch timer always global */
static struct timer_ctx *ctx;

static void setup_next_time(void *reg, uint64_t next)
{
	if (reg) {
		writel(next >> 32, reg + 4);
		writel(next & 0xffffffff, reg);
	} else {
		csr_write(CSR_STIMECMP, next);
	}
}

void mtimer_isr(void)
{
	uint64_t idx = get_isr_index();
	uint64_t next;

	next = timer_get_tick() + ctx->isr[idx].tick;

	setup_next_time(ctx->isr[idx].mtimecmp, next);

	if (!ctx->isr[idx].func)
		return;

	ctx->isr[idx].func(ctx->isr[idx].data);
}

void stimer_isr(void)
{
	mtimer_isr();
}

int timer_enable_irq(uint64_t tick, void (*isr)(void *data), void *data)
{
	uint64_t idx = get_isr_index();
	uint64_t next;

	if (!ctx)
		return -ENODEV;

	ctx->isr[idx].tick = tick;
	ctx->isr[idx].func = isr;
	ctx->isr[idx].data = data;
	if (ctx->mtimecmp.type == TIMER_TYPE_SSTC)
		ctx->isr[idx].mtimecmp = 0;
	else
		ctx->isr[idx].mtimecmp = ctx->mtimecmp.base + idx * 8;

	next = timer_get_tick() + tick;

	setup_next_time(ctx->isr[idx].mtimecmp, next);

	/* enable timer interrupt */
	if (ctx->mtimecmp.type == TIMER_TYPE_SSTC)
		arch_enable_local_stimer_irq();
	else
		arch_enable_local_timer_irq();

	return 0;
}

int timer_disable_irq(void)
{
	if (!ctx)
		return -ENODEV;

	if (ctx->mtimecmp.type == TIMER_TYPE_SSTC)
		arch_disable_local_stimer_irq();
	else
		arch_disable_local_timer_irq();

	return 0;
}

static int timer_probe(struct platform_device *pdev)
{
	int timer_type;

	if (pdev->match->data)
		timer_type = *(int *)(pdev->match->data);
	else
		timer_type = TIMER_TYPE_CLINT;

	if (timer_type == TIMER_TYPE_SSTC) {
		csr_set(CSR_MENVCFG, MENVCFG_STCE);
		if ((csr_read(CSR_MENVCFG) & MENVCFG_STCE) == 0)
			return -ENODEV;
	}

	ctx = malloc(sizeof(struct timer_ctx));
	if (!ctx)
		return -ENOMEM;

	memset(ctx, 0, sizeof(struct timer_ctx));

	ctx->mtimecmp.type = timer_type;

	switch (timer_type) {
	case TIMER_TYPE_CLINT:
		ctx->mtimecmp.base = (void *)pdev->reg_base + CLINT_MTIMECMP_OFFSET;
		break;
	case TIMER_TYPE_ACLINT:
		ctx->mtimecmp.base = (void *)pdev->reg_base;
		break;
	default: /* sstc */
		break;
	}

	/* disable timer interrupt on reset */
	if (timer_type == TIMER_TYPE_SSTC)
		arch_disable_local_stimer_irq();
	else
		arch_disable_local_timer_irq();

	return 0;
}

static const int timer_type_clint = TIMER_TYPE_CLINT;
static const int timer_type_aclint = TIMER_TYPE_ACLINT;
static const int timer_type_sstc = TIMER_TYPE_SSTC;

static struct of_device_id match_table[] = {
	{
		.compatible = "riscv,clint0",
		.data = &timer_type_clint,
	},
	{
		.compatible = "thead,c900-clint-mtimer",
		.data = &timer_type_clint,
	},
	{
		.compatible = "thead,c900-aclint-mtimer",
		.data = &timer_type_aclint,
	},
	{
		.compatible = "sstc",
		.data = &timer_type_sstc,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "aclint",
	.probe = timer_probe,
	.of_match_table = match_table,
};

static int timer_irq_init(void)
{
	return platform_driver_register(&driver);
}

module_init(timer_irq_init);
#else
int timer_enable_irq(uint64_t tick, void (*isr)(void *data), void *data)
{
	return -ENODEV;
}
int timer_disable_irq(void)
{
	return -ENODEV;
}
void mtimer_isr(void)
{
}
#endif

