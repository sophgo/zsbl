#include <arch.h>
#include <common/ictl.h>

/* constants */
#define ICTL_PRIORITY_MAX	16

/* registers */
#define IRQ_INTEN_L		0x00
#define IRQ_INTEN_H		0x04
#define IRQ_INTMASK_L		0x08
#define IRQ_INTMASK_H		0x0c
#define IRQ_INTFORCE_L		0x10
#define IRQ_INTFORCE_H		0x14
#define IRQ_RAWSTATUS_L		0x18
#define IRQ_RAWSTATUS_H		0x1c
#define IRQ_STATUS_L		0x20
#define IRQ_STATUS_H		0x24
#define IRQ_MASKSTATUS_L	0x28
#define IRQ_MASKSTATUS_H	0x2c
#define IRQ_FINALSTATUS_L	0x30
#define IRQ_FINALSTATUS_H	0x34

#define FIQ_INTEN		0xc0
#define FIQ_INTMASK		0xc4
#define FIQ_INTFORCE		0xc8
#define FIQ_RAWSTATUS		0xcc
#define FIQ_STATUS		0xd0
#define FIQ_FINALSTATUS		0xd4

#define IRQ_PLEVEL		0xd8

#define ictl_read_reg32(base, offset)	readl((u8 *)(base) + (offset))
#define ictl_write_reg32(base, offset, value)	\
	writel((value), (u8 *)(base) + (offset))

#define ictl_read_reg64(base, reg_name)				\
	((u64)readl((u8 *)(base) + (reg_name ##_L)) |		\
	 ((u64)readl((u8 *)(base) + (reg_name ##_H)) << 32))

#define ictl_write_reg64(base, reg_name, value)				\
	do {								\
		writel((value) & 0xffffffff,				\
		       (u8 *)(base) + (reg_name ##_L));			\
		writel(((value) >> 32) & 0xffffffff,			\
		       (u8 *)(base) + (reg_name ##_H));			\
	} while (0)


void ictl_set_irq_enable(void *base, u64 mask)
{
	ictl_write_reg64(base, IRQ_INTEN, mask);
}

void ictl_set_irq_mask(void *base, u64 mask)
{
	ictl_write_reg64(base, IRQ_INTMASK, mask);
}

u64 ictl_get_irq_mask(void *base)
{
	return ictl_read_reg64(base, IRQ_INTMASK);
}

void ictl_set_irq_mask_bits(void *base, u64 mask)
{
	mask = ictl_get_irq_mask(base) | mask;
	ictl_set_irq_mask(base, mask);
}

void ictl_clear_irq_mask_bits(void *base, u64 mask)
{
	mask = ictl_get_irq_mask(base) & ~mask;
	ictl_set_irq_mask(base, mask);
}

void ictl_set_irq_force(void *base, u64 mask)
{
	ictl_write_reg64(base, IRQ_INTFORCE, mask);
}

void ictl_set_fiq_enable(void *base, u32 mask)
{
	ictl_write_reg32(base, FIQ_INTEN, mask);
}

void ictl_set_fiq_mask(void *base, u32 mask)
{
	ictl_write_reg32(base, FIQ_INTMASK, mask);
}

void ictl_set_fiq_force(void *base, u32 mask)
{
	ictl_write_reg32(base, FIQ_INTFORCE, mask);
}

void ictl_set_system_priority_level(void *base, u32 priority)
{
	ictl_write_reg32(base, IRQ_PLEVEL,
		priority > ICTL_PRIORITY_MAX ? ICTL_PRIORITY_MAX : priority);
}

u64 ictl_get_irq_raw_status(void *base)
{
	return ictl_read_reg64(base, IRQ_RAWSTATUS);
}

u64 ictl_get_irq_masked_status(void *base)
{
	return ictl_read_reg64(base, IRQ_MASKSTATUS);
}

u64 ictl_get_irq_status(void *base)
{
	return ictl_read_reg64(base, IRQ_STATUS);
}

u64 ictl_get_irq_final_status(void *base)
{
	return ictl_read_reg64(base, IRQ_FINALSTATUS);
}

u32 ictl_get_fiq_raw_status(void *base)
{
	return ictl_read_reg32(base, FIQ_RAWSTATUS);
}

u32 ictl_get_fiq_status(void *base)
{
	return ictl_read_reg32(base, FIQ_STATUS);
}

u32 ictl_get_fiq_final_status(void *base)
{
	return ictl_read_reg32(base, FIQ_FINALSTATUS);
}

void ictl_init(void *base)
{
	/* init irq */
	ictl_set_irq_enable(base, 0);
	ictl_set_irq_mask(base, 0xffffffffffffffffUL);
	ictl_set_irq_force(base, 0);

	/* init fiq */
	ictl_set_fiq_enable(base, 0);
	ictl_set_fiq_mask(base, 0xffffffff);
	ictl_set_fiq_force(base, 0);

	/* irq priority filter donot filter out any irq */
	ictl_set_system_priority_level(base, 0);
}
