#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <framework/module.h>
#include <framework/common.h>
#include <framework/debug.h>
#include <lib/fatio_dev.h>
#include <driver/blkdev.h>

#include <ff.h>

struct fatio {
	struct blkdev *blkdev;
	struct fatio_dev fdev;
	FATFS fatfs;
};

static struct fatio fatio_list[16];

static long read(struct fatio_dev *fdev, unsigned long offset, unsigned long size, void *buf)
{
	struct fatio *fatio;
	struct blkdev *blkdev;

	fatio = (struct fatio *)fdev->data;
	blkdev = fatio->blkdev;

	return blkdev_read(blkdev, offset, size, buf);
}

static long write(struct fatio_dev *fdev, unsigned long offset, unsigned long size, void *buf)
{
	struct fatio *fatio;
	struct blkdev *blkdev;

	fatio = (struct fatio *)fdev->data;
	blkdev = fatio->blkdev;


	return blkdev_write(blkdev, offset, size, buf);
}

static struct fatio_ops ops = {
	.read = read,
	.write = write,
};

static int device_init()
{
	int err = 0, i = 0;
	struct blkdev *blkdev;

	pr_info("Block Device Test\n");

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev)) {
		pr_info("Register device %s as FAT32 IO device\n", blkdev->device.name);

		fatio_list[i].blkdev = blkdev;
		fatio_list[i].fdev.ops = &ops;
		fatio_list[i].fdev.data = &fatio_list[i];

		++i;

		err |= fatio_register(&fatio_list[i].fdev);

		if (err) {
			pr_err("cannot register fatio device for block device %s\n", blkdev->device.name);
			continue;
		}
	}

	return err;
}

static long load(int fd, const char *file)
{
	unsigned int size;
	void *buf;
	char fatfs_name[128];
	char fatfs_devid[128];

	FILINFO fileinfo;
	FIL fp;

	struct fatio *fatio;
	struct fatio_dev *fdev;

	fatio = &fatio_list[fd];
	fdev = &fatio->fdev;

	sprintf(fatfs_devid, "%d:", fdev->id);
	sprintf(fatfs_name, "%d:%s", fdev->id, file);

	pr_info("device: %s | %s", fatio->blkdev->device.name, fatfs_name);

	assert(f_mount(&fatio->fatfs, fatfs_devid, 1) == 0);
	assert(f_open(&fp, fatfs_name, FA_READ) == 0);
	assert(f_stat(fatfs_name, &fileinfo) == 0);

	buf = malloc(fileinfo.fsize);
	assert(buf);

	assert(f_read(&fp, buf, fileinfo.fsize, &size));

	/* dump 1st sector */
	dump_hex(buf, 512);

	f_close(&fp);
	f_unmount(fatfs_devid);
	return 0;
}
static int testfat32(void)
{
	int i;

	assert(device_init() == 0);

	for (i = 0; i < ARRAY_SIZE(fatio_list); ++i)
		if (fatio_list[i].blkdev)
			load(i, "riscv64/zsbl.bin");
	
	return 0;
}

test_case(testfat32);
