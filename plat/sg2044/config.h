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

/* code depends on the sequence of these elements, don't change it */
enum CHIP_CONNER_TYPE {
	CHIP_CONNER_TT = 0,
	CHIP_CONNER_FF,
	CHIP_CONNER_FS,
	CHIP_CONNER_SF,
	CHIP_CONNER_SS,
	CHIP_CONNER_MAX,
};

/* tpu has two status, enabled or disabled
 * when tpu is enabled, eg. ReservedMemory is set by customers, we need check
 *     struct condition->tpu[1] to see if such condition matches "TPU working" mode.
 * otherwise, we need check
 *     struct condition->tpu[0].
 */
struct condition {
	int tpu[2];
	int conner[CHIP_CONNER_MAX];
};

#define DVFS_ENTRY_MAX	16

struct dvfs_entry {
	uint64_t min;
	uint64_t max;
	uint64_t vol;
};

struct op_point {
	const char *name;
	uint64_t vddr; /* unit is mV */
	uint64_t cpu_freq; /* unit is Hz */
	uint64_t tpu_freq; /* unit is Hz */
	uint64_t noc_freq; /* unit is Hz */
	struct condition cond;

	struct dvfs_entry dvfs[DVFS_ENTRY_MAX];
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
	int tpu_avl;
	int conner;
	const struct op_point *op;

	struct boot_file sbi;
	struct boot_file kernel;
	struct boot_file dtb;
	struct boot_file ramfs;
	struct boot_file cfg;

	struct boot_file pub_key;
	struct boot_file sbi_sig;
	struct boot_file kernel_sig;
	struct boot_file dtb_sig;
	struct boot_file ramfs_sig;
};

#endif
