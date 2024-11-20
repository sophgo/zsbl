#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

struct boot_file {
	char *name;
	uintptr_t addr;
};

struct config {
	uint64_t mac0;
	uint64_t mac1;
	char *sn;
	
	struct boot_file sbi;
	struct boot_file kernel;
	struct boot_file dtb;
	struct boot_file ramfs;
};

#endif
