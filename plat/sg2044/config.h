#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

struct pcie_win {
	uint64_t pci, cpu, len;
};

struct pcie_res {
	uint64_t base, len;
};

struct pcie_config {
	char *compatible;
	/* include bus_start and bus_end */
	int bus_start, bus_end;
	int domain;

	struct pcie_res dbi;
	struct pcie_res ctl;
	struct pcie_res atu;
	struct pcie_res cfg;

	int coherent;

	struct pcie_win io;
	struct pcie_win mem32p;
	struct pcie_win mem32;
	struct pcie_win mem64p;
	struct pcie_win mem64;

	struct pcie_win ib;

	int enable;

	int irq;
};

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
	CHIP_WORK_MODE_CPU = 0,
	CHIP_WORK_MODE_SOC,
	CHIP_WORK_MODE_PCIE,
};

#define PCIE_MAX	(10)

struct config {
	uint64_t mac0;
	uint64_t mac1;
	char *sn;
	struct dram_info dram;
	struct pcie_config pcie[PCIE_MAX];
	int mode;
	char *bootargs;
	unsigned long reserved_memory_size;

	struct boot_file sbi;
	struct boot_file kernel;
	struct boot_file dtb;
	struct boot_file ramfs;
	struct boot_file cfg;
};

#endif
