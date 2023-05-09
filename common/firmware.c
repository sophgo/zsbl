#include <platform.h>
#include <stdbool.h>
#include <stdio.h>
#include <arch.h>
#include <timer.h>

void __attribute__((weak)) firmware_main(void * __attribute__((unused)) unused)
{
	while (true) {
		printf("dummy firmware_main called\n");
		mdelay(1000);
	}
}

int firmware_start(void)
{
	firmware_main(get_tpu_runtime());
	return 0;
}
