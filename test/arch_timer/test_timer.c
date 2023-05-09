#include <stdio.h>
#include <timer.h>
#include <framework/module.h>

int timer_test(void)
{

	printf("start arch timer test\n");

	int i;
	for (i = 0; i < 10; ++i) {
		timer_mdelay(1000);
		printf("%d seconds\n", i);
	}

	return 0;
}

test_case(timer_test);
