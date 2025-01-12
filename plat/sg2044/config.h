#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

struct boot_file {
	char *name;
	uintptr_t addr;
	unsigned long size;
};

struct dram_info {
	const char *vendor;
	uint64_t capacity;
	uint64_t data_rate;
	unsigned int channel_number;
	uint64_t channel_map;
};

enum {
	CHIP_WORK_MODE_POD = 0x1,
	CHIP_WORK_MODE_CPU = 0x2,
	CHIP_WORK_MODE_PCIE =0x3,
};

struct config {
	uint64_t mac0;
	uint64_t mac1;
	char *sn;
	struct dram_info dram;
	int mode;
	char *bootargs;

	struct boot_file sbi;
	struct boot_file kernel;
	struct boot_file dtb;
	struct boot_file ramfs;
	struct boot_file cfg;
};

#endif
