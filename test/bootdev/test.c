#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <framework/module.h>
#include <framework/common.h>
#include <framework/debug.h>

#include <driver/bootdev.h>

#include <platform.h>

static int test(void)
{
	void *buf = (void *)KERNEL_ADDR;

	assert(buf);

	pr_info("Test Boot Devices\n");

	bdm_load("riscv64/zsbl.bin", buf);

	dump_hex(buf, 512);

	return 0;
}

test_case(test);
