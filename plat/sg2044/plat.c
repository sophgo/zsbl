#include <framework/common.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <lib/mac.h>
#include <driver/bootdev.h>
#include <driver/platform.h>
#include <driver/blkdev.h>
#include <sbi.h>
#include <smp.h>
#include <libfdt.h>
#include <fdt_dump.h>
#include <of.h>
#include <lib/cli.h>

#include "config.h"
#include "efuse.h"

#define RAM_BASE_MASK		(~(1UL * 1024 * 1024 * 1024 - 1))

#define OPENSBI_OFFSET		0x00000000
#define KERNEL_OFFSET		0x00200000
#define DEVICETREE_OFFSET	0x08000000
#define RAMFS_OFFSET		0x0b000000
#define CFG_FILE_OFFSET		0x09000000

static void command_show_csr(struct command *c, int argc, const char *argv[])
{
#define P_REG(reg)							\
	console_printf(command_get_console(c),				\
		       "%-12s %016lx\n",				\
		       #reg, csr_read(reg))

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

cli_command(lscsr, command_show_csr);

static void disable_mac_rxdelay(void)
{
	uint32_t misc_conf;

	misc_conf = mmio_read_32(REG_TOP_MISC_CONTROL_ADDR);
	misc_conf |= RGMII0_DISABLE_INTERNAL_DELAY;
	mmio_write_32(REG_TOP_MISC_CONTROL_ADDR, misc_conf);
}

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

	file->size = err;

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

static struct config cfg;

static void show_boot_file(const char *name, struct boot_file *p)
{
	pr_info("%-16s %-20s 0x%010lx\n", name, p->name ? p->name : "[null]", p->addr);
}

static const char *mode_names[] = {
	"Unknown",
	"POD",
	"CPU",
	"PCIe"
};
static const char *mode2str(int mode)
{
	return mode_names[mode];
}

static void show_config(struct config *cfg)
{
	/* include 0 terminator */
	char mac[7];
	int i;

	pr_info("\n");
	pr_info("%-16s %s\n", "Mode", mode2str(cfg->mode));
	pr_info("%-16s %s\n", "eth0 MAC", mac2str(cfg->mac0, mac));
	pr_info("%-16s %s\n", "eth1 MAC", mac2str(cfg->mac1, mac));

	pr_info("%-16s %s %lluGiB %lluMT/s\n", "DRAM Chip",
			cfg->dram.vendor,
			cfg->dram.capacity / 1024 / 1024 / 1024,
			cfg->dram.data_rate / 1000 / 1000 );

	pr_info("%-16s [", "DRAM Map");

	for (i = 0; i < 8; ++i) {
		if ((cfg->dram.channel_map >> i) & 1)
			pr_info("O");
		else
			pr_info("X");

		if (i != 7)
			pr_info(" ");
	}

	pr_info("]\n");


	pr_info("%-16s %s\n", "SN", cfg->sn ? cfg->sn : "[null]");
	show_boot_file("SBI", &cfg->sbi);
	show_boot_file("Kernel", &cfg->kernel);
	show_boot_file("Device tree", &cfg->dtb);
	show_boot_file("Ramfs", &cfg->ramfs);
	pr_info("\n");
}

/* #define USE_LINUX_BOOT */

extern unsigned long __ld_program_start[0];

static void config_init(struct config *cfg)
{
	unsigned long ram_base = (unsigned long)__ld_program_start & RAM_BASE_MASK;

	pr_debug("ZSBL is loaded at 0x%010lx\n", (unsigned long)__ld_program_start);

	cfg->mode = get_work_mode();

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

	cfg->cfg.name = "conf.ini";
	cfg->cfg.addr = ram_base + CFG_FILE_OFFSET;

	get_dram_info(&cfg->dram);
}

/*Resize fdt to modify some node, eg: bootargs*/
int resize_dtb(struct config *cfg, int delta)
{
	void *fdt;
	int size;
	int ret = 0;

	fdt = (void *)cfg->dtb.addr;
	size = fdt_totalsize(fdt) + delta;

	fdt = realloc(fdt, size);
	if (fdt) {
		ret = fdt_open_into(fdt, fdt, size);
		if (ret != 0)
			pr_err("fdt: resize failed, error[%d\n]", ret);
		else {
			cfg->dtb.addr = (uint64_t)fdt;
			cfg->dtb.size = size;
		}
	} else {
		pr_err("fdt: realloc fdt failed\n");
		ret = -1;
	}

	return ret;
}

static void modify_eth_node(struct config *cfg)
{
	uint8_t byte[6];

	if (cfg->mac0)
		of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
				"/soc/ethernet@7030006000/", "local-mac-address",
				mac2byte(cfg->mac0, byte), 6, PROP_TYPE_U8);

	if (cfg->mac1)
		of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
				"/soc/dwcxlg@6c08000000/", "local-mac-address",
				mac2byte(cfg->mac1, byte), 6, PROP_TYPE_U8);
}

static void modify_memory_node(struct config *cfg)
{
	uint64_t dram_base, dram_size;
	void *fdt;
	char memory_node_name[64] = "/memory";
	int root_node_offset, memory_node_offset;

	dram_base = 0x80000000;
	dram_size = cfg->dram.channel_number * cfg->dram.capacity;

	fdt = (void *)cfg->dtb.addr;

	/* remove all memory nodes */
	do {
		memory_node_offset = fdt_path_offset_namelen(fdt, memory_node_name, strlen(memory_node_name));

		if (memory_node_offset >= 0) {
			pr_debug("Remove memory node at offset %d\n", memory_node_offset);
			fdt_del_node(fdt, memory_node_offset);
		}

	} while (memory_node_offset >= 0);

	/* add memory node at root node */
	root_node_offset = fdt_path_offset(fdt, "/");

	snprintf(memory_node_name, sizeof(memory_node_name), "memory@%llx", dram_base);

	memory_node_offset = fdt_add_subnode(fdt, root_node_offset, memory_node_name);
	if (memory_node_offset < 0) {
		pr_err("Failed to add memory node %s\n", memory_node_name);
		return;
	}

	if (fdt_appendprop_addrrange(fdt, root_node_offset, memory_node_offset, "reg", dram_base, dram_size)) {
		pr_err("Failed to add memory region in node %s\n", memory_node_name);
		return;
	}

	if (fdt_setprop_string(fdt, memory_node_offset, "device_type", "memory")) {
		pr_err("Failed to set memory node type for node %s\n", memory_node_name);
		return;
	}
}

static int of_get_chosen(void *fdt)
{
	int node;

	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		node = fdt_path_offset(fdt, "/");
		node = fdt_add_subnode(fdt, node, "chosen");
		if (node < 0) {
			pr_err("fdt: create /chosen failed, error[%d]\n", node);
			return -1;
		}
	}

	return node;
}

static int modify_initramfs(struct config *cfg)
{
	uint64_t addr, size;
	void *fdt;
	int node, ret;

	if (!cfg->ramfs.name)
		return 0;

	addr = (uint64_t)cfg->ramfs.addr;
	size = (uint64_t)cfg->ramfs.size;
	fdt = (void *)cfg->dtb.addr;

	node = of_get_chosen(fdt);
	if (node < 0)
		return node;

	ret = fdt_setprop_u64(fdt, node, "linux,initrd-start", addr);
	if (ret < 0) {
		pr_err("fdt: failed to set linux,initrd-start, error[%d]\n", ret);
		return -1;
	}

	ret = fdt_setprop_u64(fdt, node, "linux,initrd-end", addr + size);
	if (ret < 0) {
		pr_err("fdt: failed to set linux,initrd-end, error[%d]\n", ret);
		return -1;
	}

	return 0;
}

static int modify_bootargs(struct config *cfg)
{
	void *fdt;
	int node;
	int ret;

	if (!cfg->bootargs)
		return 0;

	fdt = (void *)cfg->dtb.addr;

	node = of_get_chosen(fdt);
	if (node < 0)
		return node;

	ret = fdt_setprop_string(fdt, node, "bootargs", cfg->bootargs);
	if (ret < 0) {
		pr_err("fdt: failed to set bootargs, error[%d]\n", ret);
		return -1;
	}

	return 0;
}

static void modify_dtb(struct config *cfg)
{
	resize_dtb(cfg, 4096);
	modify_eth_node(cfg);
	modify_memory_node(cfg);
	modify_initramfs(cfg);
	modify_bootargs(cfg);
}

int plat_main(void)
{
	disable_mac_rxdelay();

	config_init(&cfg);

	if (cfg.mode == CHIP_WORK_MODE_CPU) {
		parse_config_file(&cfg);
		show_config(&cfg);
		cli_loop(100000);
		load_images(&cfg);
		modify_dtb(&cfg);
	} else {
		show_config(&cfg);
	}

	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = cfg.kernel.addr;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	dynamic_info.boot_hart = 0xffffffffffffffffUL;

	fdt_dump_blob((void *)cfg.dtb.addr, false);

	boot_next_img(&cfg);

	return 0;
}

