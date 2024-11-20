#include <framework/common.h>
#include <framework/module.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <sbi.h>
#include <smp.h>

#include "config.h"

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
	pr_info("%-16s %d\n", "Core ID", cfg->core_id);
	pr_info("%-16s 0x%010lx\n", "RAM Base", cfg->ram_base);
	pr_info("%-16s 0x%010lx\n", "RAM Size", cfg->ram_size);

	show_boot_file("SBI", &cfg->sbi);
	show_boot_file("Kernel", &cfg->kernel);
	show_boot_file("Device tree", &cfg->dtb);
	show_boot_file("Ramfs", &cfg->ramfs);
}

extern unsigned long __ld_program_start[0];

static void config_init(struct config *cfg)
{
	pr_debug("ZSBL is loaded at 0x%010lx\n", (unsigned long)__ld_program_start);
	cfg->ram_base = (unsigned long)__ld_program_start & RAM_BASE_MASK;
	cfg->ram_size = RAM_SIZE;
	cfg->core_id = mmio_read_32(CLINT_MHART_ID);

	cfg->sbi.name = "fw_dynamic.bin";
	cfg->sbi.addr = cfg->ram_base + OPENSBI_OFFSET;
	cfg->dtb.name = "sg2044-evb.dtb";
	cfg->dtb.addr = cfg->ram_base + DEVICETREE_OFFSET;
	cfg->kernel.name = "riscv64_Image";
	cfg->kernel.addr = cfg->ram_base + KERNEL_OFFSET;
	cfg->ramfs.name = "initrd";
	cfg->ramfs.addr = cfg->ram_base + RAMFS_OFFSET;
}

int modify_tpu_dtb(struct config *cfg);

int plat_main(void)
{
	print_core_ctrlreg();

	config_init(&cfg);

	show_config(&cfg);

	modify_tpu_dtb(&cfg);

	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = cfg.kernel.addr;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	dynamic_info.boot_hart = 0xffffffffffffffffUL,

	boot_next_img(&cfg);

	return 0;
}

