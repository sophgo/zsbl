#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <timer.h>
#include <arch.h>

#include <common/common.h>
#include <platform.h>

int plat_main(void);

/* major, minor, patch */

#define VERSION_MAJOR "3"
#define VERSION_MINOR "0"
#define VERSION_PATCH "6"

#define ZSBL_VERSION	VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH

int __attribute__((weak)) main(void)
{
	/* show baner */
	pr_info("\n\nSOPHGO ZSBL v%s %s\n", ZSBL_VERSION, CONFIG_PLAT);

	return plat_main();
}
