#ifndef __NVMEM_H__
#define __NVMEM_H__

#include <driver/device.h>
#include <lib/list.h>

struct nvmem;

struct nvmem_ops {
	long (*read)(struct nvmem *nvmem, unsigned long offset, unsigned long size, void *buf);
	long (*write)(struct nvmem *nvmem, unsigned long offset, unsigned long size, void *buf);
};

struct nvmem {
	struct device device;
	struct nvmem_ops *ops;

	unsigned int cell_bits;
	unsigned int cell_count;

	void *data;
};

struct nvmem *nvmem_alloc(void);
int nvmem_register(struct nvmem *nvmem);
int nvmem_list_devices(void);
struct nvmem *nvmem_first(void);
struct nvmem *nvmem_next(struct nvmem *current);
struct nvmem *nvmem_find_by_name(const char *name);

long nvmem_read(struct nvmem *nvmem, unsigned long offset, unsigned long size, void *buf);
long nvmem_write(struct nvmem *nvmem, unsigned long offset, unsigned long size, void *buf);
void nvmem_free(struct nvmem *nvmem);

#endif
