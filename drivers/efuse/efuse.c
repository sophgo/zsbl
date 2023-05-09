#include <arch.h>

#define EFUSE_MODE	0x0000
#define EFUSE_ADDR	0x0004
#define EFUSE_DATA	0x000c

static inline uint32_t efuse_read_reg(unsigned long base, unsigned int offset)
{
	return readl(base + offset);
}

static inline void efuse_write_reg(unsigned long base, unsigned int offset,
				   uint32_t value)
{
	 writel(value, base + offset);
}

static inline void efuse_wait_ready(unsigned long base)
{
	while (efuse_read_reg(base, EFUSE_MODE) != 0x80)
		;
}

uint32_t efuse_read_cell(unsigned long base, int offset)
{
	efuse_write_reg(base, EFUSE_MODE, 0);

	efuse_wait_ready(base);

	efuse_write_reg(base, EFUSE_ADDR, offset);
	efuse_write_reg(base, EFUSE_MODE, 2);

	efuse_wait_ready(base);


	return efuse_read_reg(base, EFUSE_DATA);
}
