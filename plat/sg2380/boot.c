#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <lib/mmio.h>
#include <sbi/riscv_asm.h>
#include <sbi.h>
#include <framework/module.h>
#include <framework/common.h>
#include <sifive_extensiblecache0.h>
#include <sifive_buserror0.h>
#include <sifive_pl2cache0.h>
#include <sifive_hwpf1.h>
#include <driver/sd/sd.h>
#include <driver/io/io_sd.h>
#include <driver/io/io_flash.h>
#include <driver/io/io.h>
#include <driver/spi-flash/mango_spif.h>
#include <memmap.h>
#include <smp.h>
#include <ini.h>
#include <board.h>
#include <libfdt.h>
#include <timer.h>
#include <sg2380_ncore.h>
#include <sg2380_misc.h>
#include <driver/ddr/ddr.h>
#include <iommu.h>

#ifdef CONFIG_TARGET_FPGA
#define is_boot_from_sd_first(void) true
#else
#define is_boot_from_sd_first(void) (mmio_read_32(BOOT_SEL) & BOOT_FROM_SD_FIRST)
#endif

enum {
	ID_CONFINI = 0,
	ID_OPENSBI,
	ID_KERNEL,
	ID_RAMFS,
	ID_DEVICETREE,
	ID_MAX,
};

board_info sg2380_board_info;
uint8_t conf_file[1024] = {0};
static BOOT_FILE boot_file[ID_MAX] = {
	[ID_CONFINI] = {
		.id = ID_CONFINI,
		.name = "0:riscv64/conf.ini",
		.addr = (uint64_t)conf_file,
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
		.name = "0:riscv64/sg2380-pld.dtb",
		.addr = DEVICETREE_ADDR,
	},
};

static char *img_name_sd[] = {
	"0:riscv64/conf.ini",
	"0:riscv64/fw_dynamic.bin",
	"0:riscv64/riscv64_Image",
	"0:riscv64/initrd.img",
	"0:riscv64/sg2380-pld.dtb",
};

static char *img_name_spi[] = {
	"conf.ini",
	"fw_dynamic.bin",
	"riscv64_Image",
	"initrd.img",
	"sg2380-pld.dtb",
};

static char **img_name[] = {
	[IO_DEVICE_SD] = img_name_sd,
	[IO_DEVICE_SPIFLASH] = img_name_spi,
};

static char *dtb_name_sd[] = {
	"0:riscv64/sg2380-pld.dtb",
	"0:riscv64/sg2380-fpga.dtb",
	"0:riscv64/sg2380-sophgo-evb.dtb",
};

static char *dtb_name_spi[] = {
	"sg2380-pld.dtb",
	"sg2380-fpga.dtb",
	"sg2380-sophgo-evb.dtb",
};

static char __attribute__((unused)) **dtb_name[] = {
	[IO_DEVICE_SD] = dtb_name_sd,
	[IO_DEVICE_SPIFLASH] = dtb_name_spi,
};

static struct fw_dynamic_info dynamic_info = {
	.magic = FW_DYNAMIC_INFO_MAGIC_VALUE,
	.version = 0,
	.next_addr = KERNEL_ADDR,
	.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S,
	.boot_hart = 0xffffffffffffffff,
};

static inline char **get_bootfile_list(int dev_num, char **bootfile[])
{
	if (dev_num < IO_DEVICE_MAX)
		return bootfile[dev_num];

	return NULL;
}

static int boot_device_register(void)
{
	if (sd_io_device_register() || flash_io_device_register())
		return -1;

	return 0;
}

static int build_bootfile_info(int dev_num, BOOT_FILE *boot_file, int file_num)
{
	char *sd_dtb_name = NULL;
	char *sd_kernel_name = NULL;
	char *sd_fw_name = NULL;
	char *sd_ramfs_name = NULL;

	char** imgs = get_bootfile_list(dev_num, img_name);

	if (!imgs)
		return -1;

	if (dev_num == IO_DEVICE_SD) {
		if (sg2380_board_info.config_ini.dtb_name != NULL) {
			sd_dtb_name = malloc(64);
			memset(sd_dtb_name, 0, 64);
			strcat(sd_dtb_name, "0:riscv64/");
			strcat(sd_dtb_name, sg2380_board_info.config_ini.dtb_name);
			boot_file[ID_DEVICETREE].name = sd_dtb_name;
		}

		if (sg2380_board_info.config_ini.kernel_name != NULL) {
			sd_kernel_name = malloc(64);
			memset(sd_kernel_name, 0, 64);
			strcat(sd_kernel_name, "0:riscv64/");
			strcat(sd_kernel_name, sg2380_board_info.config_ini.kernel_name);
			boot_file[ID_KERNEL].name = sd_kernel_name;
		}

		if (sg2380_board_info.config_ini.fw_name != NULL) {
			sd_fw_name = malloc(64);
			memset(sd_fw_name, 0, 64);
			strcat(sd_fw_name, "0:riscv64/");
			strcat(sd_fw_name, sg2380_board_info.config_ini.fw_name);
			boot_file[ID_OPENSBI].name = sd_fw_name;
		}

		if (sg2380_board_info.config_ini.ramfs_name != NULL) {
			sd_ramfs_name = malloc(64);
			memset(sd_ramfs_name, 0, 64);
			strcat(sd_ramfs_name, "0:riscv64/");
			strcat(sd_ramfs_name, sg2380_board_info.config_ini.ramfs_name);
			boot_file[ID_RAMFS].name = sd_ramfs_name;
		}
	} else if (dev_num == IO_DEVICE_SPIFLASH) {
		if (sg2380_board_info.config_ini.dtb_name != NULL)
			boot_file[ID_DEVICETREE].name = sg2380_board_info.config_ini.dtb_name;

		if (sg2380_board_info.config_ini.kernel_name != NULL)
			boot_file[ID_KERNEL].name = sg2380_board_info.config_ini.kernel_name;

		if (sg2380_board_info.config_ini.fw_name != NULL)
			boot_file[ID_OPENSBI].name = sg2380_board_info.config_ini.fw_name;

		if (sg2380_board_info.config_ini.ramfs_name != NULL)
			boot_file[ID_RAMFS].name = sg2380_board_info.config_ini.ramfs_name;
	}

	if (sg2380_board_info.config_ini.dtb_addr)
		boot_file[ID_DEVICETREE].addr = sg2380_board_info.config_ini.dtb_addr;

	if (sg2380_board_info.config_ini.kernel_addr)
		boot_file[ID_KERNEL].addr = sg2380_board_info.config_ini.kernel_addr;

	if (sg2380_board_info.config_ini.fw_addr)
		boot_file[ID_OPENSBI].addr = sg2380_board_info.config_ini.fw_addr;

	if (sg2380_board_info.config_ini.ramfs_addr)
		boot_file[ID_RAMFS].addr = sg2380_board_info.config_ini.ramfs_addr;


	if (sg2380_board_info.config_ini.kernel_name == NULL)
		boot_file[ID_KERNEL].name = imgs[ID_KERNEL];

	if (sg2380_board_info.config_ini.fw_name == NULL)
		boot_file[ID_OPENSBI].name = imgs[ID_OPENSBI];

	if ((strcmp(boot_file[ID_KERNEL].name, "0:riscv64/u-boot.bin") == 0)
		|| (strcmp(boot_file[ID_KERNEL].name, "u-boot.bin") == 0)
		|| (strcmp(boot_file[ID_KERNEL].name, "0:riscv64/SG2380.fd") == 0)
		|| (strcmp(boot_file[ID_KERNEL].name, "SG2380.fd") == 0))
		boot_file[ID_RAMFS].name = NULL;
	else if (sg2380_board_info.config_ini.ramfs_name == NULL)
		boot_file[ID_RAMFS].name = imgs[ID_RAMFS];

	return 0;
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
		if (value && strlen(value))
			pconfig->ramfs_name = strdup(value);
	}
	else if (MATCH("ramfs", "addr"))
		pconfig->ramfs_addr = strtoul(value, NULL, 16);
	else
		return 0;

	return -1;
}

static int read_conf_and_parse(IO_DEV *io_dev,  int conf_file_index, int dev_num)
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
			     handler_img, &(sg2380_board_info.config_ini)) < 0)
		return -1;

	return 0;
}

static int read_all_img(IO_DEV *io_dev, BOOT_FILE *boot_file, int file_num)
{
	FILINFO info;

	if (io_dev->func.init()) {
		pr_err("init %s device failed\n",
		       io_dev->type == IO_DEVICE_SD ? "sd" : "flash");
		goto umount_dev;
	}

	for (int i = 1; i < file_num; i++) {
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

		/* skip empty file */
		if (info.fsize == 0) {
			pr_warn("%s file length zero, skip it!\n",
				boot_file[i].name);
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

		__asm__ __volatile__("fence.i" ::);
	}

	io_dev->func.destroy();

	return 0;

close_file:
	io_dev->func.close();
umount_dev:
	io_dev->func.destroy();

	return -1;
}

static int __attribute__((unused)) read_config_file(void)
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

static int __attribute__((unused)) read_boot_file(BOOT_FILE *boot_file, int file_num)
{
	IO_DEV *io_dev;
	int dev_num;

	if (boot_device_register())
		return -1;

	if (is_boot_from_sd_first() && bm_sd_card_detect()) {
		dev_num = IO_DEVICE_SD;
		pr_debug("rv boot from sd card\n");
	} else {
		dev_num = IO_DEVICE_SPIFLASH;
		pr_debug("rv boot from spi flash\n");
	}

	build_bootfile_info(dev_num, boot_file, file_num);
	io_dev = set_current_io_device(dev_num);
	if (io_dev == NULL) {
		pr_debug("set current io device failed\n");
		return -1;
	}

	if (read_all_img(io_dev, boot_file, file_num)) {
		if (dev_num == IO_DEVICE_SD) {
			dev_num = IO_DEVICE_SPIFLASH;
			build_bootfile_info(dev_num, boot_file, file_num);
			io_dev = set_current_io_device(dev_num);
			if (io_dev == NULL) {
				pr_debug(
					"set current device to flash failed\n");
				return -1;
			}

			if (read_all_img(io_dev, boot_file, file_num)) {
				pr_debug("%s read file failed\n",
					 dev_num == IO_DEVICE_SD ? "sd" :
								   "flash");
				return -1;
			}

			pr_debug("%s read file ok\n",
				 dev_num == IO_DEVICE_SD ? "sd" : "flash");
		} else {
			pr_debug("%s read file failed\n",
				 dev_num == IO_DEVICE_SD ? "sd" : "flash");
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

static int boot_next_img(void)
{
	unsigned int hartid = current_hartid();

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, secondary_core_fun, NULL,
					&secondary_core_stack[i], 4096);
	}

	return 0;
}

static int boot(void)
{
	printf("sophgo SG2380 zsbl!\n");

	sg2380_platform_init();
	sifive_extensiblecache0_init();

	sg2380_ncore_init();
	sg2380_ddr_init();
#ifndef CONFIG_TARGET_FPGA
	sg2380_iommu_init();
	sg2380_multimedia_itlvinit();
	sg2380_ssperi_phy_config(SSPERI_MODE_PCIe);
	//sg2380_pcie_init();
	sg2380_eth_type_config(SSPERI_ETH_BOTH_25G);
	sg2380_eth_mul_channel_intr_enable();
#ifdef CONFIG_TPU_SYS
	sg2380_set_tpu_run(0x200000000);
#endif
#endif

#ifndef CONFIG_TARGET_PALLADIUM
	read_config_file();
	sg2380_top_reset(SD_RESET_INDEX);
	if (read_boot_file(boot_file, ID_MAX)) {
		pr_err("read boot file faile\n");
		assert(0);
	}
#endif
	if (boot_next_img()) {
		pr_err("boot next img failed\n");
		assert(0);
	}

	__asm__ __volatile__ ("fence.i"::);
	jump_to(OPENSBI_ADDR, current_hartid(),
		DEVICETREE_ADDR, (unsigned long)&dynamic_info);

	return 0;
}
plat_init(boot);
