#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <timer.h>
#include <arch.h>

int plat_main(void);

int __attribute__((weak)) main(void)
{
	return plat_main();
}
