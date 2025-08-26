#include <stdio.h>
#include <timer.h>
#include <stdlib.h>
#include <common/module.h>
#include <common/common.h>

static int test_malloc(void)
{
	int i;
	void *p;

	pr_info("Test Malloc\n");

	for (i = 0; i < 1024 * 128; ++i) {
		p = malloc(1024);
		if (!p) {
			pr_err("Malloc failed\n");
			break;
		}
		free(p);
	}

	return 0;
}

test_case(test_malloc);
