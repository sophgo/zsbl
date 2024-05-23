#include <stdint.h>
#include <framework/module.h>
#include <framework/common.h>

static int qemu_init(void)
{
	pr_debug("platform init\n");
	return 0;
}

plat_init(qemu_init);

