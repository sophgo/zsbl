#ifndef __ICTL_H__
#define __ICTL_H__

#include <arch.h>

/* ic configurations */
/* programable interrupt priorities */
#define ICT_HC_PRIORITIES	0
/* reset value of irq_plevel register */
#define ICT_IRQ_PLEVEL		0
/* if priority filter supported */
#define ICT_HAS_PFLT		1

/* 0 is the lowest priority */
/* reset value of priority level for each irq source */
/* where n is the number of irq interrupts, aka 64
 * 16 means max priority level(from 0 to 16)
 * magic configuration
 */
#define ICT_IRQSRC_PLEVEL(n)	(n % 16)

/* so, 1684 and 1684X always bypass all irq because ICT_IRQ_PLEVEL is 0.
 * 0 is the lowest priority, no irq has a lower priority than 0
 */

/* irq number supported by ictl */
#define ICT_IRQ_NUM		64
/* fiq number supported by ictl */
#define ICT_FIQ_NUM		8

/* if vectored interrupted supported */
#define ICT_HAS_VECTOR		1

/* 1 means irq_vector_n has a hardcoded value,
 * 0 means irq_vector_n is programmable and is a read/write value
 */
#define ICT_HC_VECTOR(n)	0

/* end ic configurations */


void ictl_set_irq_enable(void *base, u64 mask);

void ictl_set_irq_mask(void *base, u64 mask);
u64 ictl_get_irq_mask(void *base);
void ictl_set_irq_mask_bits(void *base, u64 mask);
void ictl_clear_irq_mask_bits(void *base, u64 mask);

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
