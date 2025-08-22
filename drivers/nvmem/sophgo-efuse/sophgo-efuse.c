/* register to device model */
#include <arch.h>
#include <types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <driver/platform.h>
#include <driver/nvmem.h>
#include <common/common.h>
#include <common/module.h>

#define NUM_ADDRESS_BITS	8

#define EFUSE_MODE		0x00
#define EFUSE_ADR		0x04
#define EFUSE_RD_DATA		0x0c

struct sg_efuse_device {
	unsigned long reg;
	struct nvmem *nvmem;
};

static inline void efuse_write32(struct sg_efuse_device *sdev, unsigned int reg, uint32_t val)
{
	writel(val, (unsigned long)sdev->reg + reg);
}

static inline uint32_t efuse_read32(struct sg_efuse_device *sdev, unsigned int reg)
{
	return readl((unsigned long)sdev->reg + reg);
}

static void efuse_mode_wait_ready(struct sg_efuse_device *sdev)
{
	while ((efuse_read32(sdev, EFUSE_MODE) & 0b11) != 0)
		;
}

static void efuse_mode_reset(struct sg_efuse_device *sdev)
{
	efuse_write32(sdev, EFUSE_MODE, 0);
	efuse_mode_wait_ready(sdev);
}

static void efuse_mode_md_write(struct sg_efuse_device *sdev, uint32_t val)
{
	uint32_t mode = efuse_read32(sdev, EFUSE_MODE);
	uint32_t new = (mode & 0xfffffffc) | (val & 0b11);

	efuse_write32(sdev, EFUSE_MODE, new);
}

static uint32_t make_adr_val(uint32_t address, uint32_t bit_i)
{
	const uint32_t address_mask = (1 << NUM_ADDRESS_BITS) - 1;

	return (address & address_mask) |
		((bit_i & 0x1f) << NUM_ADDRESS_BITS);
}

static void efuse_set_bit(struct sg_efuse_device *sdev, uint32_t address, uint32_t bit_i)
{
	uint32_t adr_val = make_adr_val(address, bit_i);

	efuse_write32(sdev, EFUSE_ADR, adr_val);
	efuse_mode_md_write(sdev, 0b11);
	efuse_mode_wait_ready(sdev);
}

static uint32_t efuse_embedded_read(struct sg_efuse_device *sdev, uint32_t address)
{
	uint32_t adr_val;
	uint32_t ret =  0;

	efuse_mode_reset(sdev);
	adr_val = make_adr_val(address, 0);
	efuse_write32(sdev, EFUSE_ADR, adr_val);
	efuse_mode_md_write(sdev, 0b10);
	efuse_mode_wait_ready(sdev);
	ret = efuse_read32(sdev, EFUSE_RD_DATA);

	return ret;
}

static void efuse_embedded_write(struct sg_efuse_device *sdev, uint32_t address, uint32_t val)
{
	for (int i = 0; i < 32; i++)
		if ((val >> i) & 1)
			efuse_set_bit(sdev, address, i);

}

static long efuse_read(struct nvmem *nvmem, unsigned long off, unsigned long count, void *val)
{
	int size, left, start, i;
	u32 tmp;
	uint8_t *dst;
	struct sg_efuse_device *sdev = nvmem->data;

	left = count;
	dst = val;
	/* head */
	if (off & 0x03) {
		size = MIN(4 - (off & 0x03), left);
		start = (off & 0x03);
		tmp = efuse_embedded_read(sdev, off >> 2);
		memcpy(dst, &((uint8_t *)&tmp)[start], size);
		dst += size;
		left -= size;
		off += size;
	}

	/* body */
	size = left >> 2;
	for (i = 0; i < size; ++i) {
		tmp = efuse_embedded_read(sdev, off >> 2);
		memcpy(dst, &tmp, 4);
		dst += 4;
		left -= 4;
		off += 4;
	}

	/* tail */
	if (left) {
		tmp = efuse_embedded_read(sdev, off >> 2);
		memcpy(dst, &tmp, left);
	}

	return count;
}

static long efuse_write(struct nvmem *nvmem, unsigned long off, unsigned long count, void *val)
{
	int size, left, start, i;
	u32 tmp;
	uint8_t *dst;
	struct sg_efuse_device *sdev = nvmem->data;

	left = count;
	dst = val;
	/* head */
	if (off & 0x03) {
		tmp = 0;
		size = MIN(4 - (off & 0x03), left);
		start = (off & 0x03);
		memcpy(&((uint8_t *)&tmp)[start], dst, size);
		efuse_embedded_write(sdev, off >> 2, tmp);
		dst += size;
		left -= size;
		off += size;
	}

	/* body */
	size = left >> 2;
	for (i = 0; i < size; ++i) {
		memcpy(&tmp, dst, 4);
		efuse_embedded_write(sdev, off >> 2, tmp);
		dst += 4;
		left -= 4;
		off += 4;
	}

	/* tail */
	if (left) {
		tmp = 0;
		memcpy(&tmp, dst, left);
		efuse_embedded_write(sdev, off >> 2, tmp);
	}

	return count;
}

static struct nvmem_ops nvmem_ops = {
	.read = efuse_read,
	.write = efuse_write,
};

static int probe(struct platform_device *pdev)
{
	/* const struct of_device_id *match_id; */
	struct nvmem *nvmem;
	struct sg_efuse_device *sdev;

	/* match_id = platform_get_match_id(pdev); */

	nvmem = nvmem_alloc();
	if (!nvmem)
		return -ENOMEM;

	sdev = malloc(sizeof(struct sg_efuse_device));
	if (!sdev)
		return -ENOMEM;

	sdev->reg = pdev->reg_base;
	sdev->nvmem = nvmem;

	nvmem->cell_bits = 32;
	nvmem->cell_count = 256;
	nvmem->ops = &nvmem_ops;

	nvmem->data = sdev;

	device_set_child_name(&nvmem->device, &pdev->device, "nvmem");

	return nvmem_register(nvmem);
}

static struct of_device_id match_table[] = {
	{
		.compatible = "sg,efuse",
		.data = (void *)NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "efuse",
	.probe = probe,
	.of_match_table = match_table,
};

static int init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(init);

