#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <timer.h>

#include <common/module.h>
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

#define CLINT_NAME		"clint"
#define ACLINT_NAME		"aclint-mtimer"
#define CLINT_MTIMECMP_OFFSET	0x4000

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

struct timer_ctx {
	struct {
		char *type;
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

void mtimer_isr(void)
{
	uint64_t idx = get_isr_index();

	if (!ctx->isr[idx].func)
		return;

	ctx->isr[idx].func(ctx->isr[idx].data);
	writeq(timer_get_tick() + ctx->isr[idx].tick, ctx->isr[idx].mtimecmp);
}

int timer_enable_irq(uint64_t tick, void (*isr)(void *data), void *data)
{
	uint64_t idx = get_isr_index();

	if (!ctx)
		return -ENODEV;

	ctx->isr[idx].tick = tick;
	ctx->isr[idx].func = isr;
	ctx->isr[idx].data = data;
	ctx->isr[idx].mtimecmp = ctx->mtimecmp.base + idx * 8;

	writeq(timer_get_tick() + tick, ctx->isr[idx].mtimecmp);

	/* enable timer interrupt */
	arch_enable_local_timer_irq();

	return 0;
}

int timer_disable_irq(void)
{
	if (!ctx)
		return -ENODEV;

	arch_disable_local_timer_irq();

	return 0;
}

static int timer_probe(struct platform_device *pdev)
{
	int is_aclint = false;

	if (pdev->match->data)
		if (strcmp(pdev->match->data, ACLINT_NAME) == 0)
			is_aclint = true;

	ctx = malloc(sizeof(struct timer_ctx));
	if (!ctx)
		return -ENOMEM;

	memset(ctx, 0, sizeof(struct timer_ctx));

	if (is_aclint) {
		ctx->mtimecmp.type = ACLINT_NAME;
		ctx->mtimecmp.base = (void *)pdev->reg_base;
	} else {
		ctx->mtimecmp.type = CLINT_NAME;
		ctx->mtimecmp.base = (void *)pdev->reg_base + CLINT_MTIMECMP_OFFSET;
	}

	/* enable global interrupt, MSTATUS.MIE */
	arch_enable_local_irq();
	/* disable timer interrupt on reset */
	arch_disable_local_timer_irq();

	return 0;
}

static struct of_device_id match_table[] = {
	{
		.compatible = "riscv,clint0",
		.data = (void *)CLINT_NAME,
	},
	{
		.compatible = "thead,c900-aclint-mtimer",
		.data = (void *)ACLINT_NAME,
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

