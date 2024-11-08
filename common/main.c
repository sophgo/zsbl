#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <timer.h>
#include <arch.h>

#include <framework/common.h>
#include <platform.h>

int plat_main(void);

/* major, minor, platform */

/* platform version defines in platform.h */

#define VERSION_MAJOR "1"
#define VERSION_MINOR "0"

#define ZSBL_VERSION	VERSION_MAJOR "." VERSION_MINOR "." VERSION_PLATFORM

int __attribute__((weak)) main(void)
{
	/* show baner */
	pr_info("\n\nSOPHGO ZSBL v%s ""\n", ZSBL_VERSION);

	return plat_main();
}
