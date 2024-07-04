#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "sg_common.h"
#include <lib/mmio.h>
#include <memmap.h>

uint64_t get_work_mode(void)
{
	return CHIP_WORK_MODE_POD;
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
