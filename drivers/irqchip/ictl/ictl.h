#ifndef __ICTL_H__
#define __ICTL_H__

#include <arch.h>

void ictl_set_irq_enable(void *base, u64 mask);
void ictl_set_irq_mask(void *base, u64 mask);
void ictl_set_irq_force(void *base, u64 mask);
void ictl_set_fiq_enable(void *base, u32 mask);
void ictl_set_fiq_mask(void *base, u32 mask);
void ictl_set_fiq_force(void *base, u32 mask);
void ictl_set_system_priority_level(void *base, u32 priority);
u64 ictl_get_irq_raw_status(void *base);
u64 ictl_get_irq_masked_status(void *base);
u64 ictl_get_irq_status(void *base);
u64 ictl_get_irq_final_status(void *base);
u32 ictl_get_fiq_raw_status(void *base);
u32 ictl_get_fiq_status(void *base);
u32 ictl_get_fiq_final_status(void *base);
void ictl_init(void *base);

#endif
