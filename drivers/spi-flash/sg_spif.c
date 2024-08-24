// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2025, Sophgo Limited and Contributors. All rights reserved.
 *
 */

#include <assert.h>
#include <stddef.h>
#include <lib/libc/errno.h>
#include <lib/mmio.h>
#include <driver/spi-flash/sg_spif.h>
#include <driver/spinand/spinand.h>
#include <string.h>
#include <stdio.h>
#include <framework/common.h>
#include <string.h>
#include <memmap.h>
#include <driver/io/io.h>

static unsigned long ctrl_base = FLASH_BASE;

int spi_dmmr_read(uint64_t spi_base, uint32_t offset, uint8_t *dst, uint32_t size)
{
	uint8_t *src = (uint8_t *)(spi_base + offset);

	memcpy(dst, src, size);

	return 0;
}

int plat_spif_read(uint64_t addr, uint32_t offset, uint32_t size, int dev_num)
{
	int ret;
	if (dev_num == IO_DEVICE_SPIFLASH) 
		return spi_dmmr_read(ctrl_base, offset, (uint8_t *)addr, size);

	ret = load_from_spinand(offset, (uint8_t *)addr, size);

	return ret;
}

int mango_load_from_sf(uint64_t lma, uint32_t start, int size)
{
	return plat_spif_read(lma, start, size, IO_DEVICE_SPIFLASH);
}
