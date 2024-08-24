// SPDX-License-Identifier: GPL-2.0

#ifndef __PLAT_H_
#define __PLAT_H_

#include <stdint.h>

enum {
	BOOT_SPINOR_PRIORITY = 0,
	BOOT_EMMC_PRIORITY,
	BOOT_SPINAND_PRIORITY,
};

int read_boot_file(void);
void jump(void);
void sg2380_top_reset(void);
unsigned char get_boot_sel(void);
void boot_from_emmc(void);
void boot_from_spi(uint32_t dev_num);
int boot_from_sd(void);
void plat_main();
#endif
