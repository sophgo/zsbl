#include <framework/common.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <driver/bootdev.h>
#include <sbi.h>
#include <smp.h>
#include <board.h>

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

static void disable_mac_rxdelay(void)
{
	uint32_t misc_conf;

	misc_conf = mmio_read_32(REG_TOP_MISC_CONTROL_ADDR);
	misc_conf |= RGMII0_DISABLE_INTERNAL_DELAY;
	mmio_write_32(REG_TOP_MISC_CONTROL_ADDR, misc_conf);
}

enum {
	CHIP_WORK_MODE_POD = 0x1,
	CHIP_WORK_MODE_CPU = 0x2,
	CHIP_WORK_MODE_PCIE =0x3,
};

static int get_work_mode(void)
{
	uint32_t bootsel;

	bootsel = mmio_read_32(BOOT_SEL_ADDR);
	if (bootsel & BOOT_FROM_SRAM)
		return CHIP_WORK_MODE_PCIE;
	else
		return CHIP_WORK_MODE_CPU;
}


static long load(struct boot_file *file)
{
	long err;

	if (!file->name || !file->addr || !file->name[0])
		return 0;

	err = bdm_load(file->name, (void *)file->addr);

	if (err <= 0) {
		pr_err("Load %s failed, stop booting\n", file->name);
		while (true)
			;
	}

	return err;
}

static struct fw_dynamic_info dynamic_info;

static unsigned char secondary_core_stack[CONFIG_SMP_NUM][4096];

static void secondary_core_fun(void *priv)
{
	struct config *cfg = priv;

	jump_to(cfg->sbi.addr, current_hartid(), cfg->dtb.addr, (long)&dynamic_info);
}

static void boot_next_img(struct config *cfg)
{
	int i;
	unsigned int hartid = current_hartid();

	for (i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;

		wake_up_other_core(i, secondary_core_fun, cfg,
				secondary_core_stack[i], sizeof(secondary_core_stack[i]));
	}

	jump_to(cfg->sbi.addr, hartid, cfg->dtb.addr, (long)&dynamic_info);
}

int parse_config_file(struct config *cfg);

static void load_images(struct config *cfg)
{
	load(&cfg->sbi);
	load(&cfg->kernel);
	load(&cfg->dtb);
	load(&cfg->ramfs);
}

static struct config cfg = {
	.sbi = {"fw_dynamic.bin", 0x80000000},
	.kernel = {"SG2044.fd", 0x80200000},
	.dtb = {"sg2044-evb.dtb", 0x88000000},
	.ramfs = {"initrd", 0},
};

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

int plat_main(void)
{
	print_core_ctrlreg();
	disable_mac_rxdelay();

	if (get_work_mode() == CHIP_WORK_MODE_CPU) {
		pr_info("Working at CPU mode\n");
		parse_config_file(&cfg);
		show_config(&cfg);
		load_images(&cfg);
	} else {
		pr_info("Working at PCIe mode\n");
	}

	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = cfg.kernel.addr;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	dynamic_info.boot_hart = 0xffffffffffffffffUL,

	boot_next_img(&cfg);

	return 0;
}

