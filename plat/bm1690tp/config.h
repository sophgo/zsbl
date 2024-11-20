#ifndef __CONFIG_H__
#define __CONFIG_H__

#define RAM_SIZE		(512UL * 1024 * 1024)
#define RAM_BASE_MASK		(~(RAM_SIZE - 1))

#define OPENSBI_OFFSET		(16UL * 1024 * 1024)
#define KERNEL_OFFSET		(18UL * 1024 * 1024)
#define DEVICETREE_OFFSET	(128UL * 1024 * 1024)
#define RAMFS_OFFSET		(176UL * 1024 * 1024)

struct boot_file {
	char *name;
	uintptr_t addr;
};

struct config {
	int core_id;
	unsigned long ram_base, ram_size;
	struct boot_file sbi;
	struct boot_file kernel;
	struct boot_file dtb;
	struct boot_file ramfs;
};



#endif
