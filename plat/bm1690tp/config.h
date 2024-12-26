#ifndef __CONFIG_H__
#define __CONFIG_H__

#define RAM_SIZE		(512UL * 1024 * 1024)
#define RAM_BASE_MASK		(~(RAM_SIZE - 1))

#define OPENSBI_OFFSET		0x00000000
#define KERNEL_OFFSET		0x00200000
#define DEVICETREE_OFFSET	0x08000000
#define RAMFS_OFFSET		0x0b000000

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
