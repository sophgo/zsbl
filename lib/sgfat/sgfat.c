/*
 * sophgo, block device, fat boot device
 * 1. block device with a partition table
 * 2. partition table should be in MBR format, GPT is not support yet
 * 3. 1st partition should be in FAT32
 */

#include <errno.h>

#include <framework/common.h>
#include <framework/module.h>
#include <driver/blkdev.h>
#include <driver/bootdev.h>

struct sgfat {
	struct bootdev *bootdev;
	struct blkdev *blkdev;
};

static long get_file_size(struct bootdev *bootdev, const char *file)
{
	return 0;
}

static long load(struct bootdev *bootdev, const char *file, void *buf)
{
	pr_debug("sgfat load from device %s\n",bootdev->device.name);
	return 0;
}

static struct bootdev_ops ops = {
	.load = load,
	.get_file_size = get_file_size,
};

static int create_fat_dev(struct blkdev *blkdev)
{
	struct bootdev *bootdev;
	struct sgfat *sgfat;
	int err;

	bootdev = bootdev_alloc();

	if (!bootdev) {
		pr_err("allocate boot device failed\n");
		return -ENOMEM;
	}

	sgfat = malloc(sizeof(struct sgfat));

	if (!sgfat) {
		pr_err("allocate sgfat instance failed\n");
		err = -ENOMEM;
		goto free_bootdev;
	}

	sgfat->bootdev = bootdev;
	sgfat->blkdev = blkdev;
	bootdev->data = sgfat;
	bootdev->ops = &ops;
	bootdev->size = blkdev->total_size;

	err = snprintf(bootdev->device.name, sizeof(bootdev->device.name), "%s", blkdev->device.name);
	if (err == sizeof(bootdev->device.name))
		pr_warn("boot device name overflow\n");

	bootdev_add(bootdev);

	return 0;
free_bootdev:
	bootdev_free(bootdev);
	return err;
}

int sgfat_init(void)
{
	int err = 0;
	struct blkdev *blkdev;

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev)) {
		pr_info("%s\n", blkdev->device.name);
		/* create new boot devices */
		err |= create_fat_dev(blkdev);
		if (err) {
			pr_err("cannot create sg fat device for block device %s\n", blkdev->device.name);
			continue;
		}
	}

	return err;
}

late_init(sgfat_init);
