#define DEBUG

#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <common/module.h>
#include <common/common.h>
#include <lib/libc/errno.h>
#include <stdint.h>
#include <driver/spi-flash/mango_spif.h>

#define DISK_PART_TABLE_ADDR	0x600000
#define LOAD_MEMORY_ADDR	0x800000

int flash_test_rw(void)
{
	int ret;
	uint32_t addr;
	uint8_t flash_buf[256];

	memset(flash_buf, 0, sizeof(flash_buf));

	addr = LOAD_MEMORY_ADDR;
	pr_debug("into flash_test\n");
	bm_spi_init(BM_FLASH_BASE);
	pr_debug("bm_spi_init ok\n");
	ret = bm_spi_flash_erase_sector(BM_FLASH_BASE, addr);
	if (ret != 0) {
		pr_info("erase sector failed\n");
		return -1;
	}

	ret = bm_spi_flash_program_sector(BM_FLASH_BASE, addr);
	if (ret != 0) {
		pr_info("program sector failed\n");
		return -1;
	}

	ret = spi_data_read(BM_FLASH_BASE, flash_buf, addr, 256);
	if (ret != 0) {
		pr_info("read date to flash_buf failed\n");
		return -1;
	}

	for (int i = 0; i < ARRAY_SIZE(flash_buf); i++) {
		if (flash_buf[i] != 0x5c) {
			pr_info("compare spi-flash data failed\n");
			return -EIO;
		}
	}

	pr_info("spi-flash wr test ok\n");

	return 0;
}

test_case(flash_test_rw);
