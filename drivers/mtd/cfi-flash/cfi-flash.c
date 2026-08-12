/*
 * Minimal CFI NOR flash driver (read-only).
 *
 * CFI ("Common Flash Interface") parallel NOR flash — e.g. the pflash exposed
 * by the QEMU RISC-V virt machine as the `cfi-flash` node at 0x20000000 — is
 * memory-mapped: the reg window can be read directly with ordinary loads. That
 * makes reads trivial (a plain memcpy from the mapped window) and, unlike a
 * block device, arbitrary-byte addressable — exactly what the MTD layer wants.
 *
 * Only reads are implemented; a bootloader never writes flash, and writing CFI
 * requires unlock/program/erase command sequences we deliberately omit.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <driver/platform.h>
#include <driver/mtd.h>
#include <common/common.h>
#include <common/module.h>

/* CFI NOR is memory-mapped: read is a direct load from the reg window. */
static long cfi_flash_read(struct mtd *mtd, unsigned long offset,
			   unsigned long size, void *buf)
{
	unsigned long base = (unsigned long)mtd->data;

	if (offset >= mtd->total_size)
		return -EINVAL;
	if (offset + size > mtd->total_size)
		size = mtd->total_size - offset;

	memcpy(buf, (void *)(base + offset), size);

	return size;
}

static struct mtdops cfi_flash_mtdops = {
	.read = cfi_flash_read,
};

static int probe(struct platform_device *pdev)
{
	struct mtd *mtd;

	if (pdev->reg_size == 0) {
		pr_err("cfi-flash: no size in reg property\n");
		return -EINVAL;
	}

	mtd = mtd_alloc();
	if (!mtd)
		return -ENOMEM;

	/*
	 * CFI NOR is byte-readable; block_size only bounds the mtd temp buffer
	 * and read granularity hints, so a small value is fine.
	 */
	mtd->block_size = 1;
	mtd->total_size = pdev->reg_size;
	mtd->ops = &cfi_flash_mtdops;
	mtd->hwdev = &pdev->device;
	mtd->data = (void *)pdev->reg_base;

	device_set_child_name(&mtd->device, &pdev->device, "mtd");

	pr_info("cfi-flash: %lu MiB at %lx\n",
		mtd->total_size / (1024 * 1024), pdev->reg_base);

	return mtd_register(mtd);
}

static struct of_device_id match_table[] = {
	{
		.compatible = "cfi-flash",
		.data = NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "cfi-flash",
	.probe = probe,
	.of_match_table = match_table,
};

static int cfi_flash_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(cfi_flash_init);
