#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <platform.h>

struct boot_file {
	char *name;
	uintptr_t addr;
	unsigned long size;
};

struct ddr_info {
	unsigned long base;
	unsigned long size;
};

struct config {
	uint64_t mac0;
	uint64_t mac1;
	char *sn;
	
	struct boot_file sbi;
	struct boot_file kernel;
	struct boot_file dtb;
	struct boot_file dtbo;
	struct boot_file ramfs;

	int board_type;
	char *bootargs;

	int multi_socket_mode;

	struct ddr_info ddr[MAX_CHIP_NUM][DDR_CHANNEL_NUM];
};

#endif
