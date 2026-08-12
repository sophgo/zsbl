/*
 * mtdblock pass-through test.
 *
 * mtdblock wraps an MTD device as a blkdev whose reads forward straight to
 * mtd_read with no sector alignment. This exercises that full path
 * (blkdev_read -> mtdblock_read -> mtd_read -> medium) and asserts it returns
 * exactly what mtd_read returns, at both an aligned and an unaligned
 * offset/size. The unaligned case is the point: a 512-sector block device
 * could not serve it, but mtdblock preserves the MTD's arbitrary-byte reads.
 *
 * Content-agnostic by design: it compares the two read paths against each
 * other rather than against a fixed magic, so it holds regardless of what
 * backs the flash. That mtd_read itself reaches the real medium is covered by
 * the cfi-flash driver bring-up.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <common/module.h>
#include <common/common.h>
#include <driver/mtd.h>
#include <driver/blkdev.h>

/* Find the mtdblock that wraps a given MTD (it stores mtd in blkdev->data). */
static struct blkdev *find_mtdblock(struct mtd *mtd)
{
	struct blkdev *blkdev;

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev))
		if (blkdev->data == mtd)
			return blkdev;

	return NULL;
}

static int check_range(struct mtd *mtd, struct blkdev *blkdev,
		       unsigned long offset, unsigned long size)
{
	unsigned char via_mtd[32];
	unsigned char via_blk[32];
	long n_mtd;
	long n_blk;

	n_mtd = mtd_read(mtd, offset, size, via_mtd);
	n_blk = blkdev_read(blkdev, offset, size, via_blk);

	if (n_mtd != (long)size || n_blk != (long)size) {
		pr_err("mtdblock: short read at %lu+%lu (mtd=%ld blk=%ld)\n",
		       offset, size, n_mtd, n_blk);
		return -EIO;
	}

	if (memcmp(via_mtd, via_blk, size) != 0) {
		pr_err("mtdblock: data mismatch at %lu+%lu\n", offset, size);
		return -EIO;
	}

	return 0;
}

static int test_mtdblock(void)
{
	struct mtd *mtd;
	int tested = 0;

	for (mtd = mtd_first(); mtd; mtd = mtd_next(mtd)) {
		struct blkdev *blkdev = find_mtdblock(mtd);

		if (!blkdev) {
			pr_err("mtdblock: no blkdev wrapping %s\n",
			       mtd->device.name);
			return -1;
		}

		/* mtdblock advertises byte addressability */
		if (blkdev->block_size != 1) {
			pr_err("mtdblock: %s block_size %lu, expected 1\n",
			       blkdev->device.name, blkdev->block_size);
			return -1;
		}

		if (blkdev->total_size != mtd->total_size) {
			pr_err("mtdblock: %s size mismatch\n",
			       blkdev->device.name);
			return -1;
		}

		/* aligned, then deliberately unaligned offset/size */
		if (check_range(mtd, blkdev, 0, 16))
			return -1;
		if (check_range(mtd, blkdev, 3, 7))
			return -1;

		/* writes are not supported */
		if (blkdev_write(blkdev, 0, 1, "x") != -ENOSYS) {
			pr_err("mtdblock: write should return -ENOSYS\n");
			return -1;
		}

		tested++;
	}

	if (tested == 0)
		printf("mtdblock test skipped (no MTD device)\n");
	else
		printf("mtdblock test ok\n");

	return 0;
}

test_case(test_mtdblock);
