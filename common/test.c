#include <stdio.h>
#include <stdlib.h>
#include <arch.h>

#include <common/module.h>
#include <common/common.h>
#include <lib/cli.h>

extern unsigned long __ld_test_case_start[0], __ld_test_case_end[0];

static int run_test(const char *name,
		    module_init_func *start, module_init_func *end)
{
	int err;
	module_init_func *fn;

	pr_debug("%s test\n", name);

	for (fn = (module_init_func *)start;
	     fn != (module_init_func *)end;
	     ++fn) {
		err = (*fn)();
		if (err) {
			if (stdout_ready())
				pr_err("function %016lx failed with code %d\n",
				       (unsigned long)fn, err);

			while (1)
				asm volatile ("wfi");
		}
	}

	return 0;
}

int main(void)
{

	run_test("testcase",
		 (module_init_func *)__ld_test_case_start,
		 (module_init_func *)__ld_test_case_end);

	cli_loop(0);

	return 0;
}
