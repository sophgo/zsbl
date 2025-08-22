#include <stdio.h>
#include <timer.h>
#include <common/module.h>
#include <common/common.h>

static int is_early_init_work;

static int hello_early_init(void)
{
	is_early_init_work = 1;
	return 0;
}

early_init(hello_early_init);

static int hello_arch_init(void)
{
	if (is_early_init_work)
		pr_info("call hello_early_init\n");

	pr_info("call %s\n", __func__);
	return 0;
}

arch_init(hello_arch_init);

static int hello_plat_init(void)
{
	pr_info("call %s\n", __func__);
	return 0;
}

plat_init(hello_plat_init);

static int hello_module_init(void)
{
	pr_info("call %s\n", __func__);
	return 0;
}

module_init(hello_module_init);

static int hello(void)
{
	while (1) {
		pr_info("Hello World\n");
		mdelay(1000);
	}

	return 0;
}

test_case(hello);
