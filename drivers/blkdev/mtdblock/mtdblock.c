/*
 * mtdblock: expose an MTD device as a block device.
 *
 * Wraps each registered `struct mtd` in a `struct blkdev` so the upper layers
 * (filesystems, mount policy) face a single block-device abstraction regardless
 * of the underlying medium. Unlike Linux's mtdblock, reads are passed straight
 * through to mtd_read with no 512-byte sector alignment — the MTD's
 * arbitrary-byte read capability is preserved, which is what the sophgo
 * partition-table fs (sgpart) wants.
 *
 * A late_init pass enumerates every MTD device and creates one mtdblock each.
 * mtdblock_create() is factored out so a mounter can also wrap a specific MTD
 * on demand.
 *
 * Read-only: a bootloader never writes flash. blkdev_register() requires a
 * write op, so a stub returning -ENOSYS is provided.
 */

#include <stdio.h>
#include <errno.h>

#include <driver/mtd.h>
#include <driver/blkdev.h>
#include <driver/mtdblock.h>
#include <common/common.h>
#include <common/module.h>

/* Pass reads straight through — no sector alignment, arbitrary-byte reads. */
static long mtdblock_read(struct blkdev *blkdev, unsigned long offset,
			  unsigned long size, void *buf)
{
	return mtd_read(blkdev->data, offset, size, buf);
}

static long mtdblock_write(struct blkdev *blkdev, unsigned long offset,
			   unsigned long size, void *buf)
{
	return -ENOSYS;		/* flash is read-only in the loader */
}

static struct blkops mtdblock_blkops = {
	.read = mtdblock_read,
	.write = mtdblock_write,
};

/* Return the mtdblock already wrapping this MTD, or NULL if none. */
static struct blkdev *mtdblock_find(struct mtd *mtd)
{
	struct blkdev *blkdev;

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev))
		if (blkdev->ops == &mtdblock_blkops && blkdev->data == mtd)
			return blkdev;

	return NULL;
}

struct blkdev *mtdblock_create(struct mtd *mtd)
{
	struct blkdev *blkdev;

	blkdev = mtdblock_find(mtd);
	if (blkdev)
		return blkdev;		/* already wrapped; idempotent */

	blkdev = blkdev_alloc();
	if (!blkdev)
		return NULL;

	/*
	 * block_size = 1: the MTD is byte-addressable and reads pass straight
	 * through, so advertise no alignment requirement to upper layers.
	 */
	blkdev->block_size = 1;
	blkdev->total_size = mtd->total_size;
	blkdev->ops = &mtdblock_blkops;
	blkdev->data = mtd;

	device_set_child_name(&blkdev->device, &mtd->device, "block");

	if (blkdev_register(blkdev)) {
		blkdev_free(blkdev);
		return NULL;
	}

	return blkdev;
}

static int mtdblock_setup(void)
{
	struct mtd *mtd;

	for (mtd = mtd_first(); mtd; mtd = mtd_next(mtd)) {
		if (!mtdblock_create(mtd))
			pr_err("mtdblock: failed to wrap %s\n", mtd->device.name);
	}

	return 0;
}

late_init(mtdblock_setup);
