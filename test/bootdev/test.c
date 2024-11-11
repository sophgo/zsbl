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
	const char *file = "zsbl.bin";

	assert(buf);

	pr_info("Test Boot Devices\n");

	pr_info("Loading %s\n", file);
	bdm_load(file, buf);

	dump_hex(buf, 512);

	return 0;
}

test_case(test);
