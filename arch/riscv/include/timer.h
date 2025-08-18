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

/* fast method to real time */
uint64_t timer_tick2us(uint64_t tick);
uint64_t timer_tick2ms(uint64_t tick);
uint64_t timer_tick2s(uint64_t tick);

uint64_t timer_us2tick(uint64_t us);
uint64_t timer_ms2tick(uint64_t ms);
uint64_t timer_s2tick(uint64_t s);

/* interrupt */
int timer_enable_irq(uint64_t tick, void (*isr)(void *data), void *data);
int timer_disable_irq(void);

/* called by arch */
void timer_init(void);
void mtimer_isr(void);

#define mdelay(__ms)	timer_mdelay(__ms)
#define udelay(__us)	timer_udelay(__us)

#endif
