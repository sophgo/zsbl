#ifndef __BLKDEV_H__
#define __BLKDEV_H__

#include <driver/device.h>
#include <lib/list.h>

#define BLK_DEV_NAME_SUFFIX_MAX	32

struct blkdev;

struct blkops {
	long (*read)(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf);
	long (*write)(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf);
};

struct blkdev {
	struct device device;
	struct blkops *ops;

	unsigned long block_size;
	unsigned long total_size;

	char suffix[BLK_DEV_NAME_SUFFIX_MAX];

	void *tmp_buf;

	void *data;
};

struct blkdev *blkdev_alloc(void);
int blkdev_register(struct blkdev *blkdev);
int blkdev_list_devices(void);
struct blkdev *blkdev_first(void);
struct blkdev *blkdev_next(struct blkdev *current);

long blkdev_read(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf);
long blkdev_write(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf);
void blkdev_free(struct blkdev *blkdev);

#endif
