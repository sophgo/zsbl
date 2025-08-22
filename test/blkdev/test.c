#include <stdio.h>
#include <timer.h>
#include <errno.h>

#include <common/module.h>
#include <common/common.h>
#include <common/debug.h>

#include <driver/blkdev.h>

static int test_blkdev(struct blkdev *blkdev)
{
	int err;
	int i;
	unsigned long offset;

	void *buf;

	buf = malloc(blkdev->block_size);

	if (!buf) {
		pr_err("Allocate buffer failed\n");
		return -ENOMEM;
	}

	err = blkdev_open(blkdev);

	if (err) {
		pr_err("Device %s open failed\n", blkdev->device.name);
		return -EIO;
	}

	for (i = 0; i < 8; ++i) {
		offset = blkdev->block_size * i;

		pr_info("Offset 0x%08lx\n", offset);
		blkdev_read(blkdev, offset, blkdev->block_size, buf);

		/* show data in hex format */
		dump_hex(buf, blkdev->block_size);
	}

	blkdev_close(blkdev);

	return 0;
}

static int blkdev_test(void)
{
	int err = 0;
	struct blkdev *blkdev;

	pr_info("Block Device Test\n");

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev)) {
		pr_info("Testing device %s\n", blkdev->device.name);
		err |= test_blkdev(blkdev);
		if (err) {
			pr_err("cannot create sg fat device for block device %s\n", blkdev->device.name);
			continue;
		}
	}

	return err;
}

test_case(blkdev_test);
