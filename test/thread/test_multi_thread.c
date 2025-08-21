#include <stdio.h>
#include <timer.h>
#include <framework/module.h>
#include <framework/common.h>
#include <framework/thread.h>

unsigned char stack_a[4096];
unsigned char stack_b[4096];
unsigned long thread_arg = 0xdeadbeef;

struct thread *thread_a, *thread_b;

static int func_a(void *arg)
{
	int i;

	for (i = 0; true; ++i) {
		printf("a thread running %d round\n", i);
		mdelay(1000);
	}

	return 0;
}

static int func_b(void *arg)
{
	int i;

	for (i = 0; true; ++i) {
		printf("b thread running %d round\n", i);
		mdelay(1000);
	}

	return 0;
}

static int test_multi_thread(void)
{
	pr_info("Test Multi Thread\n");

	thread_a = thread_create("a", stack_a, sizeof(stack_a), func_a, &thread_arg);
	thread_b = thread_create("b", stack_b, sizeof(stack_b), func_b, &thread_arg);

	sched_start();

	return 0;
}

test_case(test_multi_thread);
