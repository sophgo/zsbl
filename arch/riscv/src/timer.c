#include <stdint.h>
#include <stdio.h>
#include <arch.h>
#include "timer.h"

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
#if defined CONFIG_PLAT_QEMU || CONFIG_PLAT_SG2380
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

void timer_init(void)
{
}
