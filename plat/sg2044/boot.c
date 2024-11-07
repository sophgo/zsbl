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
#include <asm.h>
#include <spinlock.h>
#include "board.h"
#include <libfdt.h>
#include "ini.h"
#include <sbi.h>
#include "sg_common.h"
#include "tp_dtb.h"

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

uint8_t conf_file[2048] = {0};
BOOT_FILE boot_file[ID_MAX] = {
	[ID_CONFINI] = {
		.id = ID_CONFINI,
		.name = "0:riscv64/conf.ini",
		.addr = CONFINI_ADDR,
	},
	[ID_OPENSBI] = {
		.id = ID_OPENSBI,
		.name = "0:riscv64/fw_dynamic.bin",
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
		.name = "0:riscv64/sg2044-evb.dtb",
		.addr = DEVICETREE_ADDR,
	},
};

static char *img_name_sd[] = {
	"0:riscv64/conf.ini",
	"0:riscv64/fw_dynamic.bin",
	"0:riscv64/SG2044.fd",
	NULL,
	"0:riscv64/sg2044-evb.dtb",
};

static char *img_name_spi[] = {
	"conf.ini",
	"fw_dynamic.bin",
	"SG2044.fd",
	NULL,
	"sg2044-evb.dtb",
};

char **img_name[] = {
	[IO_DEVICE_SD] = img_name_sd,
	[IO_DEVICE_SPIFLASH] = img_name_spi,
};


static char *dtb_name_sd[] = {
	"0:riscv64/sg2044r-evb.dtb",
	"0:riscv64/bm1690-cdm-rp.dtb",
	"0:riscv64/sg2044-evb.dtb",
	"0:riscv64/bm1690-evb.dtb",
	"NA",
	"0:riscv64/bm1690-cdm-rp.dtb",
};

static char *dtb_name_spi[] = {
	"sg2044r-evb.dtb",
	"bm1690-cdm-rp.dtb",
	"sg2044-evb.dtb",
	"bm1690-evb.dtb",
	"NA",
	"bm1690-cdm-rp.dtb",
};

char **dtb_name[] = {
	[IO_DEVICE_SD] = dtb_name_sd,
	[IO_DEVICE_SPIFLASH] = dtb_name_spi,
};

board_info sg2044_board_info;
struct fw_dynamic_info dynamic_info = {
	.magic = FW_DYNAMIC_INFO_MAGIC_VALUE,
	.version = 0,
	.next_addr = KERNEL_ADDR,
	.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S,
	.boot_hart = 0xffffffffffffffff,
};

uint64_t opensbi_base[10] = {
	[CORE_TOP_MCU] = 0x0,
	[CORE_64CORE_RV] = 0x80000000,
	[CORE_TPU_SCALAR0] = 0x101000000, // unused bellow
	[CORE_TPU_SCALAR1] = 0x111000000,
	[CORE_TPU_SCALAR2] = 0x121000000,
	[CORE_TPU_SCALAR3] = 0x131000000,
	[CORE_TPU_SCALAR4] = 0x141000000,
	[CORE_TPU_SCALAR5] = 0x151000000,
	[CORE_TPU_SCALAR6] = 0x161000000,
	[CORE_TPU_SCALAR7] = 0x171000000,

};

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
			     handler_img, &(sg2044_board_info.config_ini)) < 0)
		return -1;

	return 0;
}

int read_all_img(IO_DEV *io_dev, int dev_num)
{
	FILINFO info;
	uint32_t reg = 0;
	char** dtbs = get_bootfile_list(dev_num, dtb_name);

	reg = mmio_read_32(TOP_GP_REG31);
	if (reg >= 0x0 && reg <= 0x07) {
		boot_file[ID_DEVICETREE].name = dtbs[reg];
	} else {
		pr_err("can not find dtb\n");
		return -1;
	}

	if (io_dev->func.init()) {
		pr_err("init %s device failed\n", io_dev->type == IO_DEVICE_SD ? "sd" : "flash");
		goto umount_dev;
	}

	for (int i = 0; i < ID_MAX; i++) {
		if (boot_file[i].name == NULL)
			continue;

		if (io_dev->func.open(boot_file[i].name, FA_READ)) {
			if (i == ID_RAMFS || i == ID_CONFINI) {
				pr_warn("%s open fail, ignore it!\n", boot_file[i].name);
				continue;
			}

			pr_err("open %s failed\n", boot_file[i].name);
			goto close_file;
		}

		if (io_dev->func.get_file_info(boot_file, i, &info)) {
			pr_err("get %s info failed\n", boot_file[i].name);
			goto close_file;
		}

		/* skip empty file */
		if (info.fsize == 0) {
			pr_warn("%s file length zero, skip it!\n", boot_file[i].name);
			continue;
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
		if (sg2044_board_info.config_ini.dtb_name != NULL) {
			sd_dtb_name = malloc(64);
			memset(sd_dtb_name, 0, 64);
			strcat(sd_dtb_name, "0:riscv64/");
			strcat(sd_dtb_name, sg2044_board_info.config_ini.dtb_name);
			boot_file[ID_DEVICETREE].name = sd_dtb_name;
		}

		if (sg2044_board_info.config_ini.kernel_name != NULL) {
			sd_kernel_name = malloc(64);
			memset(sd_kernel_name, 0, 64);
			strcat(sd_kernel_name, "0:riscv64/");
			strcat(sd_kernel_name, sg2044_board_info.config_ini.kernel_name);
			boot_file[ID_KERNEL].name = sd_kernel_name;
		}

		if (sg2044_board_info.config_ini.fw_name != NULL) {
			sd_fw_name = malloc(64);
			memset(sd_fw_name, 0, 64);
			strcat(sd_fw_name, "0:riscv64/");
			strcat(sd_fw_name, sg2044_board_info.config_ini.fw_name);
			boot_file[ID_OPENSBI].name = sd_fw_name;
		}

		if (sg2044_board_info.config_ini.ramfs_name != NULL) {
			sd_ramfs_name = malloc(64);
			memset(sd_ramfs_name, 0, 64);
			strcat(sd_ramfs_name, "0:riscv64/");
			strcat(sd_ramfs_name, sg2044_board_info.config_ini.ramfs_name);
			boot_file[ID_RAMFS].name = sd_ramfs_name;
		}
	} else if (dev_num == IO_DEVICE_SPIFLASH) {
		if (sg2044_board_info.config_ini.dtb_name != NULL)
			boot_file[ID_DEVICETREE].name = sg2044_board_info.config_ini.dtb_name;

		if (sg2044_board_info.config_ini.kernel_name != NULL)
			boot_file[ID_KERNEL].name = sg2044_board_info.config_ini.kernel_name;

		if (sg2044_board_info.config_ini.fw_name != NULL)
			boot_file[ID_OPENSBI].name = sg2044_board_info.config_ini.fw_name;

		if (sg2044_board_info.config_ini.ramfs_name != NULL)
			boot_file[ID_RAMFS].name = sg2044_board_info.config_ini.ramfs_name;
	}

	if (sg2044_board_info.config_ini.dtb_addr)
		boot_file[ID_DEVICETREE].addr = sg2044_board_info.config_ini.dtb_addr;

	if (sg2044_board_info.config_ini.kernel_addr)
		boot_file[ID_KERNEL].addr = sg2044_board_info.config_ini.kernel_addr;

	if (sg2044_board_info.config_ini.fw_addr)
		boot_file[ID_OPENSBI].addr = sg2044_board_info.config_ini.fw_addr;

	if (sg2044_board_info.config_ini.ramfs_addr)
		boot_file[ID_RAMFS].addr = sg2044_board_info.config_ini.ramfs_addr;

	if (sg2044_board_info.config_ini.dtb_name == NULL)
		boot_file[ID_DEVICETREE].name = imgs[ID_DEVICETREE];

	if (sg2044_board_info.config_ini.kernel_name == NULL)
		boot_file[ID_KERNEL].name = imgs[ID_KERNEL];

	if (sg2044_board_info.config_ini.fw_name == NULL)
		boot_file[ID_OPENSBI].name = imgs[ID_OPENSBI];

	if (sg2044_board_info.config_ini.ramfs_name == NULL)
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

	if (!(mmio_read_32(BOOT_SEL_ADDR) & BOOT_FROM_SD_FIRST)
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

#define STACK_SIZE 4096

typedef struct {
	uint8_t stack[STACK_SIZE];
} core_stack;
static core_stack secondary_core_stack[CONFIG_SMP_NUM];

static void secondary_core_fun(void *priv)
{
	__asm__ __volatile__ ("fence.i"::);
	jump_to(boot_file[ID_OPENSBI].addr, current_hartid(),
		boot_file[ID_DEVICETREE].addr, (unsigned long)&dynamic_info);
}

int boot_next_img(void)
{
	unsigned int hartid = current_hartid();

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, secondary_core_fun, NULL,
					&secondary_core_stack[i], STACK_SIZE);
	}

	return 0;
}

void print_core_ctrlreg()
{
	pr_info("C920 control register information:\n");

#define P_REG(reg) \
	pr_info("\t %s - %016lx\n", #reg, csr_read(reg))

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

int print_banner(void)
{
	pr_info("\n\nSOPHGO ZSBL\nSG2044:v%s\n\n", ZSBL_VERSION);
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

#ifdef CONFIG_TPU_SCALAR
int calculate_addr(void)
{
	int core_id = sg2044_board_info.core_type - CORE_TPU_SCALAR0;
	sg2044_board_info.opensbi_base[sg2044_board_info.core_type] = (uint64_t)CONFIG_TPU_SCALAR_START + core_id * (uint64_t)CONFIG_TPU_SCALAR_MEM_OFFSET + 0x1000000UL;
	boot_file[ID_OPENSBI].addr = sg2044_board_info.opensbi_base[sg2044_board_info.core_type];
	boot_file[ID_KERNEL].addr = sg2044_board_info.opensbi_base[sg2044_board_info.core_type] + KERNEL_IMAGE_OFFSET;
	boot_file[ID_DEVICETREE].addr = sg2044_board_info.opensbi_base[sg2044_board_info.core_type] + DTB_OFFSET;
	boot_file[ID_RAMFS].addr = sg2044_board_info.opensbi_base[sg2044_board_info.core_type] + RAMFS_OFFSET;

	dynamic_info.next_addr = boot_file[ID_KERNEL].addr;

	pr_info("tp: set boot_file[ID_OPENSBI].addr to 0x%lx\n", boot_file[ID_OPENSBI].addr);
	pr_info("tp: set boot_file[ID_KERNEL].addr to 0x%lx\n", boot_file[ID_KERNEL].addr);
	pr_info("tp: set boot_file[ID_DEVICETREE].addr to 0x%lx\n", boot_file[ID_DEVICETREE].addr);
	pr_info("tp: set boot_file[ID_RAMFS].addr to 0x%lx\n", boot_file[ID_RAMFS].addr);

	return 0;
}
#endif

uint64_t build_board_info(void)
{
	sg2044_board_info.opensbi_base = opensbi_base;
	sg2044_board_info.core_type = get_core_type();
	pr_info("core_type is %ld\n", sg2044_board_info.core_type);

	return 0;
}

int boot(void)
{
	print_banner();
	print_core_ctrlreg();

	disable_mac_rxdelay();

#ifdef CONFIG_TPU_SCALAR
	void *fdt;
	int core_id;

	csr_write(CSR_SENVCFG, 0x70);
	build_board_info();
	calculate_addr();
	fdt = (void *)boot_file[ID_DEVICETREE].addr;
	core_id = sg2044_board_info.core_type - CORE_TPU_SCALAR0;

	if (modify_tpu_dtb(fdt, core_id)) {
		pr_err("modfiy dtb failed\n");
		assert(0);
	}
#else
	if (get_work_mode() == CHIP_WORK_MODE_CPU) {
		read_config_file();
		if (read_boot_file()) {
			pr_err("read boot file faile\n");
			assert(0);
		}
	} else if (get_work_mode() == CHIP_WORK_MODE_PCIE) {
		printf("boot file have been load by sram, go!\n");
	} else {
		build_board_info();
	}

	if (boot_next_img()) {
		pr_err("boot next img failed\n");
		assert(0);
	}
#endif

	__asm__ __volatile__ ("fence.i"::);
	printf("main core %u sbi jump to 0x%lx, dynamic info:%p\n",
			    current_hartid(), boot_file[ID_OPENSBI].addr, &dynamic_info);
	jump_to(boot_file[ID_OPENSBI].addr, current_hartid(),
		boot_file[ID_DEVICETREE].addr, (unsigned long)&dynamic_info);

	return 0;
}
plat_init(boot);
