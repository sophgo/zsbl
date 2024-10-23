#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "sg_common.h"
#include <lib/mmio.h>
#include <memmap.h>

uint64_t get_work_mode(void)
{
	uint32_t bootsel;

	bootsel = mmio_read_32(BOOT_SEL_ADDR);
	if (bootsel & BOOT_FROM_SRAM)
		return CHIP_WORK_MODE_PCIE;
	else
		return CHIP_WORK_MODE_CPU;
}

uint64_t get_core_type(void)
{
#ifdef CONFIG_TPU_SCALAR
	int core_id = mmio_read_32(CLINT_MHART_ID);
	return (CORE_TPU_SCALAR0 + core_id);
#else
	return CORE_64CORE_RV;
#endif
}

void disable_mac_rxdelay(void)
{
	uint32_t misc_conf;

	misc_conf = mmio_read_32(REG_TOP_MISC_CONTROL_ADDR);
	misc_conf |= RGMII0_DISABLE_INTERNAL_DELAY;
	mmio_write_32(REG_TOP_MISC_CONTROL_ADDR, misc_conf);
}
