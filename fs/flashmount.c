/*
 * Flash / sophgo-boot mount policy.
 *
 * The one place that touches platform information for the flash path: for
 * each MTD device that carries a "sophgo-boot" property in its DTB node, read
 * the partition-table offset from that property, wrap the MTD as a block
 * device, and explicitly mount sgpart at /mnt/flash. sgpart does not
 * self-describe (the table is not at a fixed offset), so it cannot be
 * auto-detected and must be mounted with an explicit {blkdev, offset}.
 *
 * This mirrors the legacy sgmtd boot path's DTB handling (lib/sgmtd/sgmtd.c)
 * but targets the VFS instead of a bootdev. Kept separate from sgpart itself:
 * the fs stays platform-agnostic; reading the DTB and choosing the mount point
 * is policy.
 */

#include <stdio.h>
#include <errno.h>
#include <libfdt.h>

#include <common/vfs.h>
#include <common/module.h>
#include <common/common.h>
#include <lib/container_of.h>
#include <driver/mtd.h>
#include <driver/mtdblock.h>
#include <driver/blkdev.h>
#include <driver/platform.h>
#include <fs/sgpart.h>

#define FLASH_MOUNT_POINT	"/mnt/flash"

/* Read the sophgo-boot partition-table offset from an MTD's DTB node. */
static int read_table_offset(struct mtd *mtd, unsigned long *offset)
{
	struct platform_device *pdev;
	const struct fdt_property *prop;
	int plen;

	if (!mtd->hwdev)
		return -ENODEV;

	/* the MTD's hardware device is a platform device with a DTB node */
	pdev = container_of(mtd->hwdev, struct platform_device, device);

	prop = fdt_get_property(pdev->of, pdev->of_node_offset,
				"sophgo-boot", &plen);
	if (!prop)
		return -ENOENT;		/* not a boot-carrying flash */
	if (plen < 8)
		return -EINVAL;		/* expect a 64-bit offset */

	*offset = fdt64_to_cpu(*(const volatile uint64_t *)prop->data);

	return 0;
}

static int flash_mount_one(struct mtd *mtd)
{
	struct sgpart_data sd;
	struct blkdev *blkdev;
	unsigned long offset;
	int ret;

	ret = read_table_offset(mtd, &offset);
	if (ret)
		return ret;

	blkdev = mtdblock_create(mtd);
	if (!blkdev)
		return -ENOMEM;

	if (!vfs_mkdir_p(FLASH_MOUNT_POINT))
		return -EIO;

	sd.blkdev = blkdev;
	sd.offset = offset;

	ret = vfs_mount(mtd->device.name, FLASH_MOUNT_POINT, "sgpart", &sd);
	if (ret) {
		pr_err("flashmount: sgpart mount failed on %s: %d\n",
		       mtd->device.name, ret);
		return ret;
	}

	pr_info("flashmount: %s -> %s (table @ %#lx)\n",
		mtd->device.name, FLASH_MOUNT_POINT, offset);

	return 0;
}

static int flashmount_setup(void)
{
	struct mtd *mtd;

	for (mtd = mtd_first(); mtd; mtd = mtd_next(mtd))
		flash_mount_one(mtd);		/* skip MTDs without sophgo-boot */

	return 0;
}

late_init(flashmount_setup);
