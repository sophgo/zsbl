#ifndef __MTD_H__
#define __MTD_H__

#include <driver/device.h>
#include <lib/list.h>

struct mtd;

struct mtdops {
	long (*read)(struct mtd *mtd, unsigned long offset, unsigned long size, void *buf);
};

struct mtd {
	struct device device;
	struct mtdops *ops;
	struct device *hwdev;

	unsigned long block_size;
	unsigned long total_size;

	void *data;
};

struct mtd *mtd_alloc(void);
int mtd_register(struct mtd *mtd);
int mtd_list_devices(void);
struct mtd *mtd_first(void);
struct mtd *mtd_next(struct mtd *current);

int mtd_open(struct mtd *mtd);
long mtd_read(struct mtd *mtd, unsigned long offset, unsigned long size, void *buf);
int mtd_close(struct mtd *mtd);
void mtd_free(struct mtd *mtd);
struct mtd *mtd_find_by_name(const char *name);

#endif
