#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "sg_common.h"

uint64_t get_work_mode(void)
{
        return CHIP_WORK_MODE_POD;
}

uint64_t get_core_type(void)
{
        return CORE_64CORE_RV;
}
