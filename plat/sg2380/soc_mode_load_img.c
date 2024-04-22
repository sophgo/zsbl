// SPDX-License-Identifier: GPL-2.0

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
#include <stdlib.h>
#include <string.h>
#include <ff.h>
#include <platform.h>
#include <memmap.h>
#include <lib/mmio.h>
#include <assert.h>
#include <of.h>
#include <smp.h>
#include <sbi/riscv_asm.h>
#include "spinlock.h"
#include <libfdt.h>
#include <sbi.h>
#include <thread_safe_printf.h>

#define L1_CACHE_BYTES 64
#define STACK_SIZE 4096

static inline void sync_is(void)
{
	asm volatile(".long 0x01b0000b");
}

void wbinv_va_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile(".long 0x0275000b"); /* dcache.civa a0 */

	sync_is();
}

struct core_stack {
	uint8_t stack[STACK_SIZE];
};

static struct core_stack secondary_core_stack[CONFIG_SMP_NUM];

static struct fw_dynamic_info ap_dynamic_info = {
	.magic = FW_DYNAMIC_INFO_MAGIC_VALUE,
	.version = 0,
	.next_addr = KERNEL_ADDR,
	.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S,
	.boot_hart = 0xffffffffffffffff,
};

void *get_stack_base(int core_id)
{
	return (void *)&secondary_core_stack[core_id];
}

struct fw_dynamic_info *get_ap_fw_info(void)
{
	return &ap_dynamic_info;
}

enum { RP_ID_ZSBL = 0,
       RP_ID_MAX,
};

enum { AP_ID_OPENSBI = 0,
       AP_ID_KERNEL,
       AP_ID_RAMFS,
       AP_ID_DEVICETREE,
       AP_ID_MAX,
};

static BOOT_FILE ap_boot_file[AP_ID_MAX] = {
	[AP_ID_OPENSBI] = {
		.id = AP_ID_OPENSBI,
		.name = "0:riscv64/fw_dynamic.bin",
		.addr = OPENSBI_ADDR,
	},
	[AP_ID_KERNEL] = {
		.id = AP_ID_KERNEL,
		.name = "0:riscv64/riscv64_Image",
		.addr = KERNEL_ADDR,
	},
	[AP_ID_RAMFS] = {
		.id = AP_ID_RAMFS,
		.name = "0:riscv64/initrd.img",
		.addr = RAMFS_ADDR,
	},
	[AP_ID_DEVICETREE] = {
		.id = AP_ID_DEVICETREE,
		.name = "0:riscv64/ap.dtb",
		.addr = DEVICETREE_ADDR,
	},
};

static char *ap_img_name_sd[] = {
	"0:riscv64/fw_dynamic.bin",
	"0:riscv64/riscv64_Image",
	"0:riscv64/initrd.img",
	"0:riscv64/ap.dtb",
};

static char *ap_img_name_spi[] = {
	"fw_dynamic.bin",
	"riscv64_Image",
	"initrd.img",
	"ap.dtb",
};

static char **ap_img_name[] = {
	[IO_DEVICE_SD] = ap_img_name_sd,
	[IO_DEVICE_SPIFLASH] = ap_img_name_spi,
};

static inline char **get_bootfile_list(int dev_num, char **bootfile[])
{
	if (dev_num < IO_DEVICE_MAX)
		return bootfile[dev_num];

	return NULL;
}

int read_all_img(IO_DEV *io_dev, BOOT_FILE *boot_file, int file_num)
{
	FILINFO info;

	if (io_dev->func.init()) {
		pr_err("init %s device failed\n",
		       io_dev->type == IO_DEVICE_SD ? "sd" : "flash");
		goto umount_dev;
	}

	for (int i = 0; i < file_num; i++) {
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

		wbinv_va_range(boot_file[i].addr,
			       boot_file[i].addr + info.fsize);
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

int boot_device_register(void)
{
	if (sd_io_device_register() || flash_io_device_register())
		return -1;

	return 0;
}

int build_bootfile_info(int dev_num, BOOT_FILE *boot_file, int file_num)
{
	char **imgs = NULL;
	int i;

	imgs = get_bootfile_list(dev_num, ap_img_name);
	pr_err("error boot file list\n");

	if (!imgs)
		return -1;

	for (i = 0; i < file_num; i++)
		boot_file[i].name = imgs[i];

	return 0;
}

int read_boot_file(BOOT_FILE *boot_file, int file_num)
{
	IO_DEV *io_dev;
	int dev_num;

	if (boot_device_register())
		return -1;

	if (!(mmio_read_32(BOOT_SEL) & BOOT_FROM_SD_FIRST) &&
	    bm_sd_card_detect()) {
		dev_num = IO_DEVICE_SD;
		pr_debug("rv boot from sd card\n");
	} else {
		dev_num = IO_DEVICE_SPIFLASH;
		pr_debug("rv boot from spi flash\n");
	}
	// dev_num = IO_DEVICE_SPIFLASH;
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

static void soc_mode_secondary_core_fun(void *priv)
{
	__asm__ __volatile__("fence.i" ::);

	jump_to(OPENSBI_ADDR, current_hartid(), DEVICETREE_ADDR,
		(long)get_ap_fw_info());
}

static int soc_mode_jump_to_sbi(void)
{
	unsigned int hartid = current_hartid();

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, soc_mode_secondary_core_fun, NULL,
				   get_stack_base(i), STACK_SIZE);
	}

	jump_to(OPENSBI_ADDR, current_hartid(), DEVICETREE_ADDR,
		(long)get_ap_fw_info());

	return 0;
}

int ap_boot(void)
{
	if (read_boot_file(ap_boot_file, AP_ID_MAX)) {
		pr_err("ap read boot file faile\n");
		assert(0);
	}

	soc_mode_jump_to_sbi();

	return 0;
}

int boot_from_storage(void)
{
	pr_info("boot from storage\n");

	ap_boot();

	return 0;
}
