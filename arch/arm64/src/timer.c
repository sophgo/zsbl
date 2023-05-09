#include <stdint.h>
#include <stdio.h>
#include "timer.h"
#include <sysreg.h>

uint64_t timer_frequency(void)
{
	return read_sysreg(cntfrq_el0);
}

uint64_t timer_get_tick(void)
{
	return read_sysreg(cntpct_el0);
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
#ifdef CONFIG_ARCH_SET_TIMER_FREQ
	write_sysreg(cntfrq_el0, CONFIG_ARCH_TIMER_FREQ);
#endif
}
