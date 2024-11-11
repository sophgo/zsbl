/*
 * sophgo, block device, fat boot device
 * 1. block device with a partition table
 * 2. partition table should be in MBR format, GPT is not support yet
 * 3. 1st partition should be in FAT32
 */

#include <errno.h>
#include <string.h>

#include <framework/common.h>
#include <framework/module.h>
#include <driver/blkdev.h>
#include <driver/bootdev.h>
#include <lib/fatio_dev.h>
#include <ff.h>

const char *prefix = "riscv64";

struct sgfat {
	struct bootdev *bootdev;
	struct blkdev *blkdev;
	struct fatio_dev *fdev;
	FATFS fatfs;
};

static long get_file_size(struct bootdev *bootdev, const char *file)
{
	pr_debug("sgfat get file size from device %s\n",bootdev->device.name);

	char fatfs_name[128];
	char fatfs_devid[128];
	int err;

	FILINFO fileinfo;
	FIL fp;

	struct sgfat *sgfat;
	struct fatio_dev *fdev;

	sgfat = bootdev->data;

	fdev = sgfat->fdev;

	sprintf(fatfs_devid, "%d:", fdev->id);
	sprintf(fatfs_name, "%d:%s", fdev->id, file);

	pr_debug("device: %s | %s\n", sgfat->blkdev->device.name, fatfs_name);

	err = f_mount(&sgfat->fatfs, fatfs_devid, 1);

	if (err) {
		pr_err("cannot mount FAT partition\n");
		goto out;
	}

	err = f_open(&fp, fatfs_name, FA_READ);

	if (err) {
		pr_err("cannot open %s for read\n", file);
		goto out;
	}


	err = f_stat(fatfs_name, &fileinfo);

	if (err) {
		pr_err("cannot get file %s size\n", file);
		goto out;
	}
out:

	if (err)
		return -EIO;

	return fileinfo.fsize;
}

static long load(struct bootdev *bootdev, const char *file, void *buf)
{
	pr_debug("sgfat load from device %s\n",bootdev->device.name);

	unsigned int size;
	char fatfs_name[128];
	char fatfs_devid[128];
	int err;

	FILINFO fileinfo;
	FIL fp;

	struct sgfat *sgfat;
	struct fatio_dev *fdev;

	sgfat = bootdev->data;

	fdev = sgfat->fdev;

	sprintf(fatfs_devid, "%d:", fdev->id);
	sprintf(fatfs_name, "%d:%s/%s", fdev->id, prefix, file);

	pr_debug("device: %s | %s\n", sgfat->blkdev->device.name, fatfs_name);

	err = f_mount(&sgfat->fatfs, fatfs_devid, 1);

	if (err) {
		pr_err("cannot mount FAT partition\n");
		goto out;
	}

	err = f_open(&fp, fatfs_name, FA_READ);

	if (err) {
		pr_err("cannot open %s for read\n", file);
		goto out;
	}


	err = f_stat(fatfs_name, &fileinfo);

	if (err) {
		pr_err("cannot get file %s size\n", file);
		goto out;
	}

	err = f_read(&fp, buf, fileinfo.fsize, &size);

	if (err) {
		pr_err("cannot read file %s\n", file);
		goto out;
	}

out:
	if (err)
		return -EIO;

	return size;
}

static struct bootdev_ops bops = {
	.load = load,
	.get_file_size = get_file_size,
};

static long fat_read(struct fatio_dev *fdev, unsigned long offset, unsigned long size, void *buf)
{
	struct sgfat *sgfat;
	struct blkdev *blkdev;

	sgfat = (struct sgfat *)fdev->data;
	blkdev = sgfat->blkdev;

	return blkdev_read(blkdev, offset, size, buf);
}

static long fat_write(struct fatio_dev *fdev, unsigned long offset, unsigned long size, void *buf)
{
	struct sgfat *sgfat;
	struct blkdev *blkdev;

	sgfat = (struct sgfat *)fdev->data;
	blkdev = sgfat->blkdev;

	return blkdev_write(blkdev, offset, size, buf);
}

static struct fatio_ops fops = {
	.read = fat_read,
	.write = fat_write,
};

static int create_fat_dev(struct blkdev *blkdev)
{
	struct bootdev *bootdev;
	struct sgfat *sgfat;
	struct fatio_dev *fdev;
	int err;

	bootdev = bootdev_alloc();

	if (!bootdev) {
		pr_err("allocate boot device failed\n");
		return -ENOMEM;
	}

	fdev = fatio_alloc();

	if (!fdev) {
		pr_err("allocate fatio device failed\n");
		return -ENOMEM;
	}

	sgfat = malloc(sizeof(struct sgfat));

	if (!sgfat) {
		pr_err("allocate sgfat instance failed\n");
		err = -ENOMEM;
		goto free_bootdev;
	}

	memset(sgfat, 0, sizeof(struct sgfat));

	sgfat->bootdev = bootdev;
	sgfat->blkdev = blkdev;
	sgfat->fdev = fdev;
	bootdev->data = sgfat;
	bootdev->ops = &bops;
	bootdev->size = blkdev->total_size;
	fdev->ops = &fops;
	fdev->data = sgfat;

	err = snprintf(bootdev->device.name, sizeof(bootdev->device.name), "%s", blkdev->device.name);
	if (err == sizeof(bootdev->device.name))
		pr_warn("boot device name overflow\n");

	err = fatio_register(fdev);

	if (err) {
		pr_err("Register fatio device failed\n");
		goto free_bootdev;
	}

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
