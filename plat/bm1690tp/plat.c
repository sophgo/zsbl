#include <framework/common.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <driver/bootdev.h>
#include <sbi.h>
#include <smp.h>

#define RAM_BASE_MASK		(~(1UL * 1024 * 1024 * 1024 - 1))

#define OPENSBI_OFFSET		0x00000000
#define KERNEL_OFFSET		0x00200000
#define DEVICETREE_OFFSET	0x08000000
#define RAMFS_OFFSET		0x0b000000

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

static void print_core_ctrlreg(void)
{
	pr_info("C920 control register information:\n");

#define P_REG(reg) \
	pr_info("\t %-12s - %016lx\n", #reg, csr_read(reg))

	P_REG(CSR_MCOR);
	P_REG(CSR_MHCR);
	P_REG(CSR_MCCR2);
	P_REG(CSR_MSMPR);
	P_REG(CSR_MENVCFG);
	P_REG(CSR_MHINT);
	P_REG(CSR_MHINT2);
	P_REG(CSR_MHINT4);
	P_REG(CSR_MXSTATUS);
}

static struct fw_dynamic_info dynamic_info;

static void boot_next_img(struct config *cfg)
{
	unsigned int hartid = current_hartid();

	jump_to(cfg->sbi.addr, hartid, cfg->dtb.addr, (long)&dynamic_info);
}

static struct config cfg;

static void show_boot_file(const char *name, struct boot_file *p)
{
	pr_info("%-16s %-20s 0x%010lx\n", name, p->name ? p->name : "[null]", p->addr);
}

static void show_config(struct config *cfg)
{
	pr_info("%-16s %lx\n", "eth0 MAC", cfg->mac0);
	pr_info("%-16s %lx\n", "eth1 MAC", cfg->mac1);

	pr_info("%-16s %s\n", "SN", cfg->sn ? cfg->sn : "[null]");
	show_boot_file("SBI", &cfg->sbi);
	show_boot_file("Kernel", &cfg->kernel);
	show_boot_file("Device tree", &cfg->dtb);
	show_boot_file("Ramfs", &cfg->ramfs);
}

/* #define USE_LINUX_BOOT */

extern unsigned long __ld_program_start[0];

static void config_init(struct config *cfg)
{
	unsigned long ram_base = (unsigned long)__ld_program_start & RAM_BASE_MASK;

	pr_debug("ZSBL is loaded at 0x%010lx\n", (unsigned long)__ld_program_start);

	cfg->sbi.name = "fw_dynamic.bin";
	cfg->sbi.addr = ram_base + OPENSBI_OFFSET;
	cfg->dtb.name = "sg2044-evb.dtb";
	cfg->dtb.addr = ram_base + DEVICETREE_OFFSET;
#ifdef USE_LINUX_BOOT
	cfg->kernel.name = "riscv64_Image";
	cfg->ramfs.name = "initrd";
	cfg->ramfs.addr = ram_base + RAMFS_OFFSET;
#else
	cfg->kernel.name = "SG2044.fd";
	cfg->ramfs.name = NULL;
	cfg->ramfs.addr = 0;
#endif
	cfg->kernel.addr = ram_base + KERNEL_OFFSET;
}

int plat_main(void)
{
	print_core_ctrlreg();

	config_init(&cfg);

	show_config(&cfg);

	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = cfg.kernel.addr;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	dynamic_info.boot_hart = 0xffffffffffffffffUL,

	boot_next_img(&cfg);

	return 0;
}

