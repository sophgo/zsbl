#include <framework/common.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <driver/bootdev.h>
#include <driver/mtd.h>
#include <sbi.h>
#include <smp.h>
#include <timer.h>

#include <libfdt.h>
#include <of.h>

#include "config.h"

#define OPENSBI_ADDR		0x00000000
#define KERNEL_ADDR		0x02000000
#define DEVICETREE_ADDR		0x20000000
#define RAMFS_ADDR		0x30000000


static void print_core_ctrlreg(void)
{
	pr_info("C920 control register information:\n");

#define P_REG(reg) \
	pr_info("\t %-12s - %016lx\n", #reg, csr_read(reg))

	P_REG(CSR_MCOR);
	P_REG(CSR_MHCR);
	P_REG(CSR_MCCR2);
	P_REG(CSR_MSMPR);
	P_REG(CSR_MHINT);
	P_REG(CSR_MHINT2);
	P_REG(CSR_MHINT4);
	P_REG(CSR_MXSTATUS);
}

static long load(struct boot_file *file)
{
	long err;

	if (!file->name || !file->name[0])
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
	unsigned int hartid = current_hartid();
	int i;

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

static void show_ddr_info(struct ddr_info ddr[][DDR_CHANNEL_NUM])
{
	int i, j;
	char title[64];

	for (i = 0; i < MAX_CHIP_NUM; ++i) {
		for (j = 0; j < DDR_CHANNEL_NUM; ++j) {
			if (ddr[i][j].size) {
				snprintf(title, sizeof(title), "Chip%d DDR%d", i, j);
				pr_info("%-16s 0x%010lx - 0x%010lx (0x%010lx)\n", title,
						ddr[i][j].base, ddr[i][j].base + ddr[i][j].size,
						ddr[i][j].size);
			}
		}
	}
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

	show_ddr_info(cfg->ddr);
}

static void config_init(struct config *cfg)
{
	cfg->sbi.name = "fw_dynamic.bin";
	cfg->sbi.addr = OPENSBI_ADDR;
	cfg->dtb.name = "mango-sophgo-x8evb.dtb";
	cfg->dtb.addr = DEVICETREE_ADDR;
	cfg->kernel.name = "riscv64_Image";
	cfg->kernel.addr = KERNEL_ADDR;
	cfg->ramfs.name = "initrd.img";
	cfg->ramfs.addr = RAMFS_ADDR;
}

uint64_t get_ddr_size(uint64_t ddr_reg_size, int ddr_channel)
{
	uint64_t ddr_size = 0xffUL << (ddr_channel << 3);
	ddr_size = (ddr_size & ddr_reg_size) >> (ddr_channel << 3);
	if (ddr_size == DDR_SIZE_ZERO)
		return 0;

	ddr_size = 1UL << (MAX_PHY_ADDR_WIDTH - ddr_size);

	return ddr_size;
}

static int build_ddr_info(struct config *cfg, int chip_num)
{
	uint64_t reg_ddr_size_base = chip_num * 0x8000000000 + DDR_SIZE_ADDR;
	uint32_t sg2042_ddr_reg_size = mmio_read_32(reg_ddr_size_base);
	uint64_t ddr_start_base = cfg->ddr[chip_num][0].base;
	int i;

	pr_debug("chip%d ddr info: raw data=0x%x, \n", chip_num, sg2042_ddr_reg_size);

	for (i = 0; i < DDR_CHANNEL_NUM; i++) {
		cfg->ddr[chip_num][i].base = ddr_start_base;
		cfg->ddr[chip_num][i].size = get_ddr_size(sg2042_ddr_reg_size, i);

		ddr_start_base += cfg->ddr[chip_num][i].size;

		pr_debug("    DDR%d 0x%lx bytes\n", i, cfg->ddr[chip_num][i].size);
	}

	return 0;
}

const char *dtb_names[] = {
	"mango-sophgo-x8evb.dtb",
	"mango-milkv-pioneer.dtb",
	"mango-sophgo-pisces.dtb",
	"mango-sophgo-x4evb.dtb",
};

static void build_board_info(struct config *cfg)
{
	int i;

	if (mmio_read_32(BOOT_SEL_ADDR) & MULTI_SOCKET_MODE) {
		pr_info("SG2042 work in multi socket mode\n");
		cfg->multi_socket_mode = true;
	} else {
		pr_info("SG2042 work in single socket mode\n");
		cfg->multi_socket_mode = false;
	}

	cfg->board_type = mmio_read_32(BOARD_TYPE_REG);

	if (cfg->board_type >= BOARD_TYPE_MIN && cfg->board_type <= BOARD_TYPE_MAX)
		cfg->dtb.name = (char *)dtb_names[cfg->board_type - BOARD_TYPE_MIN];
	else
		pr_err("Can not find device tree\n");

#if 0
	for (int i = 0; i < MAX_CHIP_NUM; i++)
		for (int j = 0; j < DDR_CHANNEL_NUM; j++)
			sg2042_board_info.ddr_info[i].ddr_node_name[j] = ddr_node_name[i][j];
#endif

	for (i = 0; i < MAX_CHIP_NUM; ++i)
		cfg->ddr[i][0].base = CHIP_ADDR_SPACE * i;

	for (i = 0; i < MAX_CHIP_NUM; i++) {
		build_ddr_info(cfg, i);
		if (!cfg->multi_socket_mode)
			break;
	}
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

int add_memory_node(struct config *cfg, uint64_t addr, uint64_t len, int numa_node_id)
{
	void *fdt;
	int root_node, memory_node;
	char name[64];

	if (len == 0)
		return 0;

	fdt = (void *)cfg->dtb.addr;
	root_node = fdt_path_offset(fdt, "/");

	memset(name, 0, sizeof(name));
	sprintf(name, "memory@%lx", addr);

	memory_node = fdt_add_subnode(fdt, root_node, name);
	if (memory_node < 0) {
		pr_err("fdt: failed to add memory node [%lx, %lx], error(%d)\n", addr, addr+len, memory_node);
		return -1;
	}

	fdt_setprop_string(fdt, memory_node, "device_type", "memory");

	fdt_appendprop_addrrange(fdt, root_node, memory_node, "reg", addr, len);

	fdt_setprop_u32(fdt, memory_node, "numa-node-id", numa_node_id);

	return 0;
}

static int show_ddr_node(struct config *cfg, uint64_t addr)
{
	void *fdt;
	int node;
	char path[64] = {0};
	const uint64_t *p_value = NULL;
	int p_len;

	fdt = (void *)cfg->dtb.addr;
	sprintf(path, "/memory@%lx", addr);

	node = fdt_path_offset(fdt, path);
	if (node < 0) {
		pr_err("can not find %s\n", path);
		return -1;
	}

	p_value = fdt_getprop(fdt, node, "reg", &p_len);
	if (!p_value) {
		pr_err("can not get reg\n");
		return -1;
	}
	if (p_len != 16) {

		pr_err("the size is error\n");
		return -1;
	}

	if (fdt64_ld(&p_value[1]) != 0)
		pr_debug("    base:0x%010lx, len:0x%lx\n", fdt64_ld(&p_value[0]), fdt64_ld(&p_value[1]));

	return 0;
}

static int modify_ddr_node(struct config *cfg)
{
	uint64_t start, size;
	uint64_t reserved_start, reserved_size;
	int chip_num = 1;
	int numa_node_id;

	if (cfg->multi_socket_mode)
		chip_num = MAX_CHIP_NUM;

	for (int i = 0; i < chip_num; i++) {
		pr_debug("chip%d ddr node in dtb:\n", i);
		for (int j = 0; j < DDR_CHANNEL_NUM; j++) {
			start = cfg->ddr[i][j].base;
			size = cfg->ddr[i][j].size;
			numa_node_id = ((i == 0) ? j : (i * DDR_CHANNEL_NUM + j));

			if (size == 0)
				break;

			if (i == 0 && j == 0) {
				//reserved memory for pcie switch
				reserved_start = 0xc0000000;
				reserved_size  = 0x40000000;

				if (reserved_start >= start
						&& (reserved_start + reserved_size) <= (start + size)) {
					add_memory_node(cfg, start, reserved_start-start, numa_node_id);
					show_ddr_node(cfg, start);

					size = (start + size) - (reserved_start + reserved_size);
					start = reserved_start + reserved_size;
				}
			}

			add_memory_node(cfg, start, size, numa_node_id);
			show_ddr_node(cfg, start);
		}
	}

	return 0;
}

static int modify_bootargs(struct config *cfg)
{
	struct fdt_property *prop;
	void *fdt, *ramfs;
	int node;
	int oldlen;
	int ret = 0;

	char bootargs[256] = {0};
	char append[128] = {0};

	fdt = (void *)cfg->dtb.addr;
	ramfs = (void*)cfg->ramfs.addr;
	sprintf(append, "root=/dev/ram0 rw initrd=0x%lx,32M", (unsigned long)ramfs);

	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		node = fdt_path_offset(fdt, "/");
		node = fdt_add_subnode(fdt, node, "chosen");
		if (node < 0) {
			pr_err("fdt: create /chosen failed, error[%d]\n", node);
			return -1;
		}
	}

	prop = fdt_get_property_w(fdt, node, "bootargs", &oldlen);
	if (prop) {
		if (oldlen > sizeof(bootargs)-strlen(append)-1) {
			pr_err("fdt: old bootargs is too large\n");
			return -1;
		}
		sprintf(bootargs, "%s %s", (char*)prop->data, append);
	} else {
		sprintf(bootargs,
			"console=ttyS0,115200 earlycon root=/dev/ram0 rw initrd=0x%lx,32M",
			(unsigned long)ramfs);
	}
	ret = fdt_setprop_string(fdt, node, "bootargs", bootargs);

	return ret;
}

static int modify_chip_node(struct config *cfg, int chip_num)
{
	uint64_t mp0_status_base = chip_num * CHIP_ADDR_SPACE + MP0_STATUS_ADDR;
	uint64_t mp0_control_base = chip_num * CHIP_ADDR_SPACE + MP0_CONTROL_ADDR;
	uint32_t cluster_id;
	uint32_t cluster_status;
	uint32_t cpu_id;
	char cpu_node[16];

	for (int i = 0; i < CLUSTER_PER_CHIP; i++) {
		cluster_id = mmio_read_32(mp0_status_base + (i << 3));
		cluster_status = mmio_read_32(mp0_control_base + (i << 3));
		if (cluster_status == 0) {
			for (int j = 0; j < CORES_PER_CLUSTER; j++) {
				cpu_id = cluster_id * CORES_PER_CLUSTER + j;
				memset(cpu_node, 0, sizeof(cpu_node));
				sprintf(cpu_node, "/cpus/cpu@%d/", cpu_id);
				of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
						cpu_node, "status", (void *)"dis", sizeof("dis"), PROP_TYPE_STR);
			}
		}
	}

	return 0;
}

static int modify_cpu_node(struct config *cfg)
{
	int chip_num = 1;

	if (cfg->multi_socket_mode)
		chip_num = MAX_CHIP_NUM;

	for (int i = 0; i < chip_num; i++)
		modify_chip_node(cfg, i);

	return 0;
}

void modify_eth_node(struct config *cfg)
{
	uint64_t mac = 0;
	uint8_t *mac_p = (uint8_t *)&cfg->mac0;

	for (int i = 0; i < 6; i++)
		mac |= ((uint64_t)mac_p[i]) << (5 - i) * 8;

	cfg->mac0 = mac;
	of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
			"/soc/ethernet@7040026000/", "local-mac-address",
			(void *)&cfg->mac0, 6, PROP_TYPE_U8);

	if (cfg->multi_socket_mode) {
		mac = 0;
		mac_p = (uint8_t *)&cfg->mac1;
		for (int i = 0; i < 6; i++)
			mac |= ((uint64_t)mac_p[i]) << (5 - i) * 8;

		cfg->mac1 = mac;
		of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
				"/soc/ethernet@f040026000/", "local-mac-address",
				(void *)&cfg->mac1, 6, PROP_TYPE_U8);
	}
}

static void modify_dtb(struct config *cfg)
{
	int ret = 0;

	ret = resize_dtb(cfg, 4096);
	if (!ret) {
		modify_ddr_node(cfg);
		modify_bootargs(cfg);
	}

	modify_cpu_node(cfg);
	modify_eth_node(cfg);
}

static void reset_device(uint32_t index)
{
	uint32_t ret;

	ret = mmio_read_32(TOP_RESET_BASE);
	ret &= ~(1 << index);
	mmio_write_32(TOP_RESET_BASE, ret);

	timer_udelay(100);

	ret |= 1 << index;
	mmio_write_32(TOP_RESET_BASE, ret);
}

int plat_main(void)
{
	print_core_ctrlreg();

	build_board_info(&cfg);

	config_init(&cfg);

	parse_config_file(&cfg);
	show_config(&cfg);
	load_images(&cfg);

	modify_dtb(&cfg);

	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = cfg.kernel.addr;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	dynamic_info.boot_hart = 0xffffffffffffffffUL;

	reset_device(SD_RESET_INDEX);

	boot_next_img(&cfg);

	return 0;
}

