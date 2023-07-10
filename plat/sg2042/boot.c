#define DEBUG
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <driver/sd/sd.h>
#include <driver/io/io_sd.h>
#include <driver/io/io_flash.h>
#include <driver/io/io.h>
#include <driver/spi-flash/mango_spif.h>

#include <ff.h>
#include <platform.h>
#include <memmap.h>
#include <lib/mmio.h>
#include <assert.h>
#include <of.h>
#include <smp.h>
#include <sbi/riscv_asm.h>
#include "spinlock.h"
#include "board.h"
#include <libfdt.h>
#include "ini.h"
//#include <thread_safe_printf.h>

//#define ZSBL_BOOT_DEBUG
//#define ZSBL_BOOT_DEBUG_LOOP

#ifdef ZSBL_BOOT_DEBUG
void uart_putc(int ch);
void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}

static void hex2asc(const char *pdata, char *pstr, int len)
{
	char ch;
	int i, mylen;

	if(len>16)
		mylen = 16;
	else
		mylen = len;

	for(i=mylen; i>0; i--)
	{
		ch = pdata[(i-1)>>1];
		if( i%2 )
			ch &= 0xF;
		else
			ch >>= 4;
		if(ch<10)
			ch += '0';
		else
			ch += ('A'-10);
		pstr[mylen-i] = ch;

	}
	pstr[mylen] = 0;
}

void print_u32(unsigned int u32)
{
	char str[8+1];

	hex2asc((const char *)&u32, str, 8);
	uart_puts(str);
}

void print_u8(unsigned int u32)
{
	char str[2+1];

	hex2asc((const char *)&u32, str, 2);
	uart_puts(str);
}
#endif

#define L1_CACHE_BYTES 64

static inline void sync_is(void)
{
	asm volatile (".long 0x01b0000b");
}

void wbinv_va_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile (".long 0x0275000b"); /* dcache.civa a0 */

	sync_is();
}

enum {
	ID_CONFINI = 0,
	ID_OPENSBI,
	ID_KERNEL,
	ID_RAMFS,
	ID_DEVICETREE,
	ID_MAX,
};

uint8_t conf_file[1024] = {0};
BOOT_FILE boot_file[ID_MAX] = {
	[ID_CONFINI] = {
		.id = ID_CONFINI,
		.name = "0:riscv64/conf.ini",
		.addr = (uint64_t)conf_file,
	},
	[ID_OPENSBI] = {
		.id = ID_OPENSBI,
		.name = "0:riscv64/fw_jump.bin",
		.addr = OPENSBI_ADDR,
	},
	[ID_KERNEL] = {
		.id = ID_KERNEL,
		.name = "0:riscv64/riscv64_Image",
		.addr = KERNEL_ADDR,
	},
	[ID_RAMFS] = {
		.id = ID_RAMFS,
		.name = "0:riscv64/initrd.img",
		.addr = RAMFS_ADDR,
	},
	[ID_DEVICETREE] = {
		.id = ID_DEVICETREE,
		.name = "0:riscv64/mango.dtb",
		.addr = DEVICETREE_ADDR,
	},
};

static char *img_name_sd[] = {
	"0:riscv64/conf.ini",
	"0:riscv64/fw_jump.bin",
	"0:riscv64/riscv64_Image",
	"0:riscv64/initrd.img",
	"0:riscv64/mango.dtb",
};

static char *img_name_spi[] = {
	"conf.ini",
	"fw_jump.bin",
	"riscv64_Image",
	"initrd.img",
	"mango.dtb",
};

char **img_name[] = {
	[IO_DEVICE_SD] = img_name_sd,
	[IO_DEVICE_SPIFLASH] = img_name_spi,
};

static char *dtb_name_sd[] = {
	"0:riscv64/mango-sophgo-x8evb.dtb",
	"0:riscv64/mango-milkv-pioneer.dtb",
	"0:riscv64/mango-sophgo-pisces.dtb",
	"0:riscv64/mango-sophgo-x4evb.dtb",
};

static char *dtb_name_spi[] = {
	"mango-sophgo-x8evb.dtb",
	"mango-milkv-pioneer.dtb",
	"mango-sophgo-pisces.dtb",
	"mango-sophgo-x4evb.dtb",
};

char **dtb_name[] = {
	[IO_DEVICE_SD] = dtb_name_sd,
	[IO_DEVICE_SPIFLASH] = dtb_name_spi,
};

char *ddr_node_name[SG2042_MAX_CHIP_NUM][DDR_CHANLE_NUM] = {
	{
		"/memory@0/",
		"/memory@1/",
		"/memory@2/",
		"/memory@3/",
	}, {
		"/memory@4/",
		"/memory@5/",
		"/memory@6/",
		"/memory@7/",
	}

};

board_info sg2042_board_info;

static inline char** get_bootfile_list(int dev_num, char** bootfile[])
{
	if (dev_num < IO_DEVICE_MAX)
		return bootfile[dev_num];
	return NULL;
}

static int handler_img(void* user, const char* section, const char* name,
				   const char* value)
{
	config_ini *pconfig = (config_ini *)user;

	#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

	if (MATCH("devicetree", "name"))
		pconfig->dtb_name = strdup(value);
	else if (MATCH("devicetree", "addr"))
		pconfig->dtb_addr = strtoul(value, NULL, 16);
	else if (MATCH("kernel", "name"))
		pconfig->kernel_name = strdup(value);
	else if (MATCH("kernel", "addr"))
		pconfig->kernel_addr = strtoul(value, NULL, 16);
	else if (MATCH("firmware", "name"))
		pconfig->fw_name = strdup(value);
	else if (MATCH("firmware", "addr"))
		pconfig->fw_addr = strtoul(value, NULL, 16);
	else if (MATCH("ramfs", "name")) {
		if (value && strlen(value)) {
			pconfig->ramfs_name = strdup(value);
		} else {
			boot_file[ID_RAMFS].name = NULL;
			img_name_sd[ID_RAMFS] = NULL;
			img_name_spi[ID_RAMFS] = NULL;
		}
	}
	else if (MATCH("ramfs", "addr"))
		pconfig->ramfs_addr = strtoul(value, NULL, 16);
	else
		return 0;

	return -1;
}

int read_conf_and_parse(IO_DEV *io_dev,  int conf_file_index, int dev_num)
{
	FILINFO info;
	const char *header = "[sophgo-config]";
	const char *tail = "[eof]";
	char *eof;

	if (dev_num == IO_DEVICE_SD) {
		if (io_dev->func.open(boot_file[conf_file_index].name, FA_READ))
			return -1;

		if (io_dev->func.get_file_info(boot_file, conf_file_index, &info)) {
			pr_err("get %s info failed\n", boot_file[conf_file_index].name);
			return -1;
		}
		boot_file[conf_file_index].len = info.fsize;
		if (io_dev->func.read(boot_file, conf_file_index, info.fsize)) {
			pr_err("read %s failed\n", boot_file[conf_file_index].name);
			return -1;
		}

		if (io_dev->func.close()) {
			pr_err("close %s failed\n", boot_file[conf_file_index].name);
			return -1;
		}

		wbinv_va_range(boot_file[conf_file_index].addr, boot_file[conf_file_index].addr + info.fsize);
		__asm__ __volatile__ ("fence.i"::);
	} else if (dev_num == IO_DEVICE_SPIFLASH) {
		bm_spi_init(FLASH0_BASE);
		bm_spi_flash_read((uint8_t *)boot_file[conf_file_index].addr, 0, sizeof(conf_file) - 1);
		// back to DMMR mode
		mmio_write_32(FLASH0_BASE + REG_SPI_DMMR, 1);
	}

	boot_file[ID_DEVICETREE].name = NULL;

	if (strncmp(header, (const char *)boot_file[conf_file_index].addr, strlen(header))) {
		pr_err("conf.ini should start with \"%s\"\n", header);
		return -1;
	}

	eof = strstr((const char *)boot_file[conf_file_index].addr, tail);

	if (!eof) {
		pr_err("conf.ini should terminated by \"%s\"\n", tail);
		return -1;
	}

	*eof = 0;

	if (ini_parse_string((const char*)boot_file[conf_file_index].addr,
			     handler_img, &(sg2042_board_info.config_ini)) < 0)
		return -1;

	return 0;
}

int read_all_img(IO_DEV *io_dev, int dev_num)
{
	FILINFO info;
	uint32_t reg = 0;
	char** dtbs = get_bootfile_list(dev_num, dtb_name);

	if (boot_file[ID_DEVICETREE].name == NULL) {
		reg = mmio_read_32(BOARD_TYPE_REG);
		if (reg >= 0x02 && reg <= 0x05) {
			boot_file[ID_DEVICETREE].name = dtbs[reg - 0x02];
		} else {
			pr_err("can not find dtb\n");
			return -1;
		}
	}

	if (io_dev->func.init()) {
		pr_err("init %s device failed\n", io_dev->type == IO_DEVICE_SD ? "sd" : "flash");
		goto umount_dev;
	}

	for (int i = 1; i < ID_MAX; i++) {
		if (boot_file[i].name == NULL)
			continue;

		if (io_dev->func.open(boot_file[i].name, FA_READ)) {
			pr_err("open %s failed\n", boot_file[i].name);
			goto close_file;
		}

		if (io_dev->func.get_file_info(boot_file, i, &info)) {
			pr_err("get %s info failed\n", boot_file[i].name);
			goto close_file;
		}
		boot_file[i].len = info.fsize;
		if (io_dev->func.read(boot_file, i, info.fsize)) {
			pr_err("read %s failed\n", boot_file[i].name);
			goto close_file;
		}

		if (io_dev->func.close()) {
			pr_err("close %s failed\n", boot_file[i].name);
			goto umount_dev;
		}

		wbinv_va_range(boot_file[i].addr, boot_file[i].addr + info.fsize);
		__asm__ __volatile__ ("fence.i"::);
	}

	io_dev->func.destroy();

	return 0;

close_file:
	io_dev->func.close();
umount_dev:
	io_dev->func.destroy();

	return -1;
}

int boot_device_register()
{
	if (sd_io_device_register() || flash_io_device_register())
		return -1;

	return 0;
}

int build_bootfile_info(int dev_num)
{
	char *sd_dtb_name = NULL;
	char *sd_kernel_name = NULL;
	char *sd_fw_name = NULL;
	char *sd_ramfs_name = NULL;

	char** imgs = get_bootfile_list(dev_num, img_name);

	if (!imgs)
		return -1;

	if (dev_num == IO_DEVICE_SD) {
		if (sg2042_board_info.config_ini.dtb_name != NULL) {
			sd_dtb_name = malloc(64);
			memset(sd_dtb_name, 0, 64);
			strcat(sd_dtb_name, "0:riscv64/");
			strcat(sd_dtb_name, sg2042_board_info.config_ini.dtb_name);
			boot_file[ID_DEVICETREE].name = sd_dtb_name;
		}

		if (sg2042_board_info.config_ini.kernel_name != NULL) {
			sd_kernel_name = malloc(64);
			memset(sd_kernel_name, 0, 64);
			strcat(sd_kernel_name, "0:riscv64/");
			strcat(sd_kernel_name, sg2042_board_info.config_ini.kernel_name);
			boot_file[ID_KERNEL].name = sd_kernel_name;
		}

		if (sg2042_board_info.config_ini.fw_name != NULL) {
			sd_fw_name = malloc(64);
			memset(sd_fw_name, 0, 64);
			strcat(sd_fw_name, "0:riscv64/");
			strcat(sd_fw_name, sg2042_board_info.config_ini.fw_name);
			boot_file[ID_OPENSBI].name = sd_fw_name;
		}

		if (sg2042_board_info.config_ini.ramfs_name != NULL) {
			sd_ramfs_name = malloc(64);
			memset(sd_ramfs_name, 0, 64);
			strcat(sd_ramfs_name, "0:riscv64/");
			strcat(sd_ramfs_name, sg2042_board_info.config_ini.ramfs_name);
			boot_file[ID_RAMFS].name = sd_ramfs_name;
		}
	} else if (dev_num == IO_DEVICE_SPIFLASH) {
		if (sg2042_board_info.config_ini.dtb_name != NULL)
			boot_file[ID_DEVICETREE].name = sg2042_board_info.config_ini.dtb_name;

		if (sg2042_board_info.config_ini.kernel_name != NULL)
			boot_file[ID_KERNEL].name = sg2042_board_info.config_ini.kernel_name;

		if (sg2042_board_info.config_ini.fw_name != NULL)
			boot_file[ID_OPENSBI].name = sg2042_board_info.config_ini.fw_name;

		if (sg2042_board_info.config_ini.ramfs_name != NULL)
			boot_file[ID_RAMFS].name = sg2042_board_info.config_ini.ramfs_name;
	}

	if (sg2042_board_info.config_ini.dtb_addr)
		boot_file[ID_DEVICETREE].addr = sg2042_board_info.config_ini.dtb_addr;

	if (sg2042_board_info.config_ini.kernel_addr)
		boot_file[ID_KERNEL].addr = sg2042_board_info.config_ini.kernel_addr;

	if (sg2042_board_info.config_ini.fw_addr)
		boot_file[ID_OPENSBI].addr = sg2042_board_info.config_ini.fw_addr;

	if (sg2042_board_info.config_ini.ramfs_addr)
		boot_file[ID_RAMFS].addr = sg2042_board_info.config_ini.ramfs_addr;


	if (sg2042_board_info.config_ini.kernel_name == NULL)
		boot_file[ID_KERNEL].name = imgs[ID_KERNEL];

	if (sg2042_board_info.config_ini.fw_name == NULL)
		boot_file[ID_OPENSBI].name = imgs[ID_OPENSBI];

	if (sg2042_board_info.config_ini.ramfs_name == NULL)
		boot_file[ID_RAMFS].name = imgs[ID_RAMFS];

	return 0;
}

int read_config_file(void)
{
	IO_DEV *io_dev;
	int dev_num;
	int ret = 0;

	if (boot_device_register())
		return -1;

	if (bm_sd_card_detect())
		dev_num = IO_DEVICE_SD;
	else
		dev_num = IO_DEVICE_SPIFLASH;

	io_dev = set_current_io_device(dev_num);
	if (io_dev == NULL) {
		pr_debug("set current io device failed\n");
		return -1;
	}

	io_dev->func.init();
	ret = read_conf_and_parse(io_dev, ID_CONFINI, dev_num);
	if (ret != 0) {
		if (dev_num == IO_DEVICE_SD) {
			io_dev->func.destroy();
			dev_num = IO_DEVICE_SPIFLASH;
			io_dev = set_current_io_device(dev_num);
			if (io_dev == NULL) {
				pr_debug("set current io device failed\n");
				return -1;
			}
			io_dev->func.init();
			ret = read_conf_and_parse(io_dev, ID_CONFINI, dev_num);
			if (ret != 0) {
				pr_err("have no conf.ini file\n");
			} else {
				pr_debug("read config from spi flash\n");
			}
		}

	} else {
		pr_debug("read config from %s\n", dev_num == IO_DEVICE_SD ? "sd": "flash");
	}

	io_dev->func.destroy();

	return 0;
}

int read_boot_file(void)
{
	IO_DEV *io_dev;
	int dev_num;

	if (boot_device_register())
		return -1;

	if ((mmio_read_32(BOOT_SEL_ADDR) & BOOT_FROM_SD_FIRST)
	    && bm_sd_card_detect()) {
		dev_num = IO_DEVICE_SD;
		pr_debug("rv boot from sd card\n");
	} else {
		dev_num = IO_DEVICE_SPIFLASH;
		pr_debug("rv boot from spi flash\n");
	}
	// dev_num = IO_DEVICE_SPIFLASH;
	build_bootfile_info(dev_num);
	io_dev = set_current_io_device(dev_num);
	if (io_dev == NULL) {
		pr_debug("set current io device failed\n");
		return -1;
	}

	if (read_all_img(io_dev, dev_num)) {
		if (dev_num == IO_DEVICE_SD) {
			dev_num = IO_DEVICE_SPIFLASH;
			build_bootfile_info(dev_num);
			io_dev = set_current_io_device(dev_num);
			if (io_dev == NULL) {
				pr_debug("set current device to flash failed\n");
				return -1;
			}

			if (read_all_img(io_dev, dev_num)) {
				pr_debug("%s read file failed\n",
					 dev_num == IO_DEVICE_SD ? "sd" : "flash");
				return -1;
			}

			pr_debug("%s read file ok\n", dev_num == IO_DEVICE_SD ? "sd" : "flash");
		} else {
			pr_debug("%s read file failed\n", dev_num == IO_DEVICE_SD ? "sd" : "flash");
			return -1;
		}
	} else {
		pr_debug("%s read file ok\n",
			 dev_num == IO_DEVICE_SD ? "sd" : "flash");
	}

	return 0;
}

int show_ddr_node(char *path)
{
	int len;
	int node;
	const uint64_t *p_value = NULL;

	node = fdt_path_offset((void *)boot_file[ID_DEVICETREE].addr, path);
	if (node < 0) {
		pr_err("can not find %s\n", path);
		return -1;
	}
	p_value = fdt_getprop((void *)boot_file[ID_DEVICETREE].addr, node, "reg", &len);
	if (!p_value) {
		pr_err("can not get reg\n");
		return -1;
	}
	if (len != 16) {
		pr_err("the size is error\n");
		return -1;
	}

	pr_debug("    base:0x%lx, len:0x%lx\n", fdt64_ld(&p_value[0]), fdt64_ld(&p_value[1]));

	return 0;
}

int modify_ddr_node(void)
{
	uint64_t value[2];
	int chip_num = 1;

	if (sg2042_board_info.multi_sockt_mode)
		chip_num = SG2042_MAX_CHIP_NUM;

	for (int i = 0; i < chip_num; i++) {
		pr_debug("chip%d ddr node in dtb:\n", i);
		for (int j = 0; j < DDR_CHANLE_NUM; j++) {
			value[0] = sg2042_board_info.ddr_info[i].ddr_start_base[j];
			value[1] = sg2042_board_info.ddr_info[i].chip_ddr_size[j];

			of_modify_prop((void *)boot_file[ID_DEVICETREE].addr, boot_file[ID_DEVICETREE].len,
				       sg2042_board_info.ddr_info[i].ddr_node_name[j], "reg", (void *)value,
				       sizeof(value), PROP_TYPE_U64);

			show_ddr_node(sg2042_board_info.ddr_info[i].ddr_node_name[j]);
		}
	}

	return 0;
}

int modify_chip_node(int chip_num)
{
	uint64_t mp0_status_base = chip_num * SG2040_CHIP_ADDR_SPACE + MP0_STATUS_ADDR;
	uint64_t mp0_control_base = chip_num * SG2040_CHIP_ADDR_SPACE + MP0_CONTROL_ADDR;
	uint32_t cluster_id;
	uint32_t cluster_status;
	uint32_t cpu_id;
	char cpu_node[16];

	for (int i = 0; i < SG2042_CLUSTER_PER_CHIP; i++) {
		cluster_id = mmio_read_32(mp0_status_base + (i << 3));
		cluster_status = mmio_read_32(mp0_control_base + (i << 3));
		if (cluster_status == 0) {
			for (int j = 0; j < MANGO_CORES_PER_CLUSTER; j++) {
				cpu_id = cluster_id * MANGO_CORES_PER_CLUSTER + j;
				memset(cpu_node, 0, sizeof(cpu_node));
				sprintf(cpu_node, "/cpus/cpu@%d/", cpu_id);
				of_modify_prop((void *)boot_file[ID_DEVICETREE].addr, boot_file[ID_DEVICETREE].len,
						cpu_node, "status", (void *)"dis", sizeof("dis"), PROP_TYPE_STR);

			}
		}
	}

	return 0;
}

int modify_cpu_node(void)
{
	int chip_num = 1;

	if (sg2042_board_info.multi_sockt_mode)
		chip_num = SG2042_MAX_CHIP_NUM;

	for (int i = 0; i < chip_num; i++) {
		modify_chip_node(i);
	}

	return 0;
}

static int handler_mac_addr(void* user, const char* section, const char* name,
				   const char* value)
{
	config_ini *pconfig = (config_ini *)user;

	#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

	if (MATCH("mac-address", "mac0"))
		pconfig->mac0 = strtoul(value, NULL, 16);
	else if (MATCH("mac-address", "mac1"))
		pconfig->mac1 = strtoul(value, NULL, 16);
	else
		return 0;

	return -1;
}

int parse_mac_from_confi(void)
{
	if (ini_parse_string((const char*)boot_file[ID_CONFINI].addr, handler_mac_addr,
	    &(sg2042_board_info.config_ini)) < 0 || sg2042_board_info.config_ini.mac0 == 0) {
		pr_info("use default mac address\n");
		return false;
	} else {
		pr_info("mac0:0x%lx\n", sg2042_board_info.config_ini.mac0);
		if (sg2042_board_info.multi_sockt_mode == 1) {
			pr_info("mac1:0x%lx\n", sg2042_board_info.config_ini.mac1);
		}
		return true;
	}

	return true;
}

void modify_mac_address(void)
{
	uint64_t mac = 0;
	uint8_t *mac_p = (uint8_t *)&sg2042_board_info.config_ini.mac0;

	for (int i = 0; i < 6; i++)
		mac |= ((uint64_t)mac_p[i]) << (5-i) * 8;
	sg2042_board_info.config_ini.mac0 = mac;
	of_modify_prop((void *)boot_file[ID_DEVICETREE].addr, boot_file[ID_DEVICETREE].len,
		        	"/soc/ethernet@7040026000/", "local-mac-address", (void *)&sg2042_board_info.config_ini.mac0, 6, PROP_TYPE_U8);

	if (sg2042_board_info.multi_sockt_mode == 1) {
		mac = 0;
		uint8_t *mac_p = (uint8_t *)&sg2042_board_info.config_ini.mac1;
		for (int i = 0; i < 6; i++)
			mac |= ((uint64_t)mac_p[i]) << (5-i) * 8;
		sg2042_board_info.config_ini.mac1 = mac;
		of_modify_prop((void *)boot_file[ID_DEVICETREE].addr, boot_file[ID_DEVICETREE].len,
			       "/soc/ethernet@f040026000/", "local-mac-address", (void *)&sg2042_board_info.config_ini.mac1, 6, PROP_TYPE_U8);
	}
}

void modify_eth_node(void)
{
	if (parse_mac_from_confi())
		modify_mac_address();
}

int modify_dtb(void)
{
	modify_ddr_node();
	modify_cpu_node();
	modify_eth_node();

	return 0;
}

#define STACK_SIZE 4096

typedef struct {
	uint8_t stack[STACK_SIZE];
} core_stack;
static core_stack secondary_core_stack[CONFIG_SMP_NUM];

#ifdef ZSBL_BOOT_DEBUG
static spinlock_t print_lock = SPIN_LOCK_INITIALIZER;
volatile uint32_t core_stats[CONFIG_SMP_NUM];
volatile uint32_t *sram_point   = (uint32_t *)(0x70101d2000Ul);
volatile uint32_t *sram_sp_addr = (uint32_t *)(0x70101d3000Ul);
#endif

static void secondary_core_fun(void *priv)
{
#ifdef ZSBL_BOOT_DEBUG
	unsigned int cid = current_hartid();
	register unsigned long sp asm("sp");

	sram_point[cid] = sram_point[cid] + 0x100;
	core_stats[cid] = 0xbeef;
	__asm__ volatile("":::"memory");
	__asm__ volatile("fence rw, rw":::);

	spin_lock(&print_lock);

	sram_point[cid] = sram_point[cid] + 0x10;
	sram_sp_addr[cid] = sp;

	uart_puts("my core id : "); print_u8(cid); uart_puts("\r\n");

	spin_unlock(&print_lock);

	sram_point[cid] = sram_point[cid] + 0x10;
	core_stats[cid] = 0xface0000;
	__asm__ volatile("":::"memory");
	__asm__ volatile("fence rw, rw":::);

#ifdef ZSBL_BOOT_DEBUG_LOOP
	while(1) {
		mdelay(10000);
		sram_point[cid] = sram_point[cid] + 1;
		core_stats[cid] = core_stats[cid] + 1;
		__asm__ volatile("":::"memory");
		__asm__ volatile("fence rw, rw":::);
	}
#endif // ZSBL_BOOT_DEBUG_LOOP

#endif // ZSBL_BOOT_DEBUG

	__asm__ __volatile__ ("fence.i"::);
	jump_to(boot_file[ID_OPENSBI].addr, current_hartid(),
		boot_file[ID_DEVICETREE].addr);
}

int boot_next_img(void)
{
	unsigned int hartid = current_hartid();

#ifdef ZSBL_BOOT_DEBUG
	spin_lock(&print_lock);
	uart_puts("main core id : "); print_u8(hartid); uart_puts("\r\n");
	spin_unlock(&print_lock);
#endif // ZSBL_BOOT_DEBUG

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, secondary_core_fun, NULL,
					&secondary_core_stack[i], STACK_SIZE);
	}

	return 0;
}

uint64_t get_ddr_size(uint64_t ddr_reg_size, int ddr_chanle)
{
	uint64_t ddr_size = 0xffUL << (ddr_chanle << 3);
	ddr_size = (ddr_size & ddr_reg_size) >> (ddr_chanle << 3);
	if (ddr_size == DDR_SIZE_ZERO)
		return 0;

	ddr_size = 1UL << (SG2042_MAX_PHY_ADDR_WIDTH - ddr_size);

	return ddr_size;
}


int build_ddr_info (int chip_num)
{
	uint64_t reg_ddr_size_base = chip_num * 0x8000000000 + DDR_SIZE_ADDR;
	uint32_t sg2042_ddr_reg_size = mmio_read_32(reg_ddr_size_base);
	uint64_t ddr_start_base = sg2042_board_info.ddr_info[chip_num].ddr_start_base[0];

	pr_debug("chip%d ddr info: raw data=0x%x, \n", chip_num, sg2042_ddr_reg_size);

	for (int i = 0; i < DDR_CHANLE_NUM; i++) {
		sg2042_board_info.ddr_info[chip_num].ddr_start_base[i] = ddr_start_base;
		sg2042_board_info.ddr_info[chip_num].chip_ddr_size[i] = get_ddr_size(sg2042_ddr_reg_size, i);
		ddr_start_base += sg2042_board_info.ddr_info[chip_num].chip_ddr_size[i];

		pr_info("    ddr%d size:0x%lx\n", i, sg2042_board_info.ddr_info[chip_num].chip_ddr_size[i]);
	}

	return 0;
}

int build_board_info(void)
{
	if (mmio_read_32(BOOT_SEL_ADDR) & MULTI_SOCKET_MODE) {
		pr_info("sg2042 work in multi socket mode\n");
		sg2042_board_info.multi_sockt_mode = 1;
	} else {
		pr_info("sg2042 work in single socket mode\n");
	}

	for (int i = 0; i < SG2042_MAX_CHIP_NUM; i++)
		for (int j = 0; j < DDR_CHANLE_NUM; j++)
			sg2042_board_info.ddr_info[i].ddr_node_name[j] = ddr_node_name[i][j];

	sg2042_board_info.ddr_info[0].ddr_start_base[0] = 0;
	sg2042_board_info.ddr_info[1].ddr_start_base[0] = 0x8000000000;

	if (sg2042_board_info.multi_sockt_mode) {
		for (int i = 0; i < SG2042_MAX_CHIP_NUM; i++)
			build_ddr_info(i);
	} else {
		build_ddr_info(0);
	}

	return 0;
}

int print_banner(void)
{
	pr_info("\n\nSOPHGO ZSBL\nsg2042:v%s\n\n", ZSBL_VERSION);

	return 0;
}

void sg2042_top_reset(uint32_t index)
{
	uint32_t ret;

	ret = mmio_read_32(TOP_RESET_BASE);
	ret &= ~(1 << index);
	mmio_write_32(TOP_RESET_BASE, ret);

	timer_udelay(100);

	ret |= 1 << index;
	mmio_write_32(TOP_RESET_BASE, ret);
}

int boot(void)
{
	print_banner();
	build_board_info();

	read_config_file();

	if (read_boot_file()) {
		pr_err("read boot file faile\n");
		assert(0);
	}

	if (modify_dtb()) {
		pr_err("modify dtb failed\n");
		assert(0);
	}

	if (boot_next_img()) {
		pr_err("boot next img failed\n");
		assert(0);
	}

#ifdef ZSBL_BOOT_DEBUG
	//spin_lock(&print_lock);
	uart_puts("all cores woke up\r\n");
	//spin_unlock(&print_lock);

	core_stats[current_hartid()] = 0xface0001;
	sram_point[current_hartid()] = 0x5A5A0001;
	__asm__ volatile("":::"memory");
	__asm__ volatile("fence rw, rw":::);

#ifdef ZSBL_BOOT_DEBUG_LOOP
	while(1) {
		mdelay(20000);

		//spin_lock(&print_lock);
		uart_puts(">>>\r\n");
		for (int i = 0; i < 64; i++) {
			uart_puts("core_stats[");     print_u8(i); uart_puts("] = ");
				print_u32(core_stats[i]); uart_puts("\r\n");
			uart_puts("   sram_point[");  print_u8(i); uart_puts("]   = ");
				print_u32(sram_point[i]); uart_puts("\r\n");
			uart_puts("   sram_sp_addr["); print_u8(i); uart_puts("] = ");
				print_u32(sram_sp_addr[i]); uart_puts("\r\n");
		}
		uart_puts("<<<\r\n");
		//spin_unlock(&print_lock);

		sram_point[current_hartid()] = sram_point[current_hartid()] + 1;
		core_stats[current_hartid()] = core_stats[current_hartid()] + 1;
		__asm__ volatile("":::"memory");
		__asm__ volatile("fence rw, rw":::);
	}
#endif // ZSBL_BOOT_DEBUG_LOOP

#endif // ZSBL_BOOT_DEBUG

	sg2042_top_reset(SD_RESET_INDEX);
	__asm__ __volatile__ ("fence.i"::);
	jump_to(boot_file[ID_OPENSBI].addr, current_hartid(),
		boot_file[ID_DEVICETREE].addr);

	return 0;
}
test_case(boot);
