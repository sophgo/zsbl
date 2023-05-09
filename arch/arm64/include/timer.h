#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

uint64_t timer_frequency(void);

/* tick */
uint64_t timer_get_tick(void);

/* delay */
void timer_delay_tick(uint64_t tick);
void timer_mdelay(uint32_t ms);
void timer_udelay(uint32_t us);

/* fast method to real tiem */
uint64_t timer_tick2us(uint64_t tick);
uint64_t timer_tick2ms(uint64_t tick);
uint64_t timer_tick2s(uint64_t tick);

void timer_init(void);

#define mdelay(__ms)	timer_mdelay(__ms)
#define udelay(__us)	timer_udelay(__us)

#endif
