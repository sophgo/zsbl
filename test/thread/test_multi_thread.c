#include <stdio.h>
#include <timer.h>
#include <common/module.h>
#include <common/common.h>
#include <common/thread.h>
#include <lib/cli.h>

unsigned char stack_a[4096];
unsigned char stack_b[4096];
unsigned char stack_shell[4096];
unsigned long thread_arg = 0xdeadbeef;

struct thread *thread_a, *thread_b, *thread_shell;

static int func_a(void *arg)
{
	int i;

	for (i = 0; true; ++i) {
		printf("a thread running %d round\n", i);
		sched_msleep(1000);
	}

	return 0;
}

static int func_b(void *arg)
{
	int i;

	for (i = 0; true; ++i) {
		printf("b thread running %d round\n", i);
		sched_msleep(1000);
	}

	return 0;
}

static int shell(void *arg)
{
	cli_loop(0);

	return 0;
}

static int test_multi_thread(void)
{
	pr_info("Test Multi Thread\n");

	thread_a = thread_create("a", stack_a, sizeof(stack_a), func_a, &thread_arg);
	thread_b = thread_create("b", stack_b, sizeof(stack_b), func_b, &thread_arg);
	thread_shell = thread_create("shell", stack_shell, sizeof(stack_shell), shell, NULL);

	sched_start();

	return 0;
}


#include <common/semaphore.h>

struct thread *thread_poster, *thread_waiter;
struct sem sem;

static int sem_poster(void *arg)
{
	int i;

	for (i = 0; i < 10; ++i) {
		sched_msleep(1000);
		pr_info("Send semaphore to waiter %d times\n", i);
		sem_post(&sem);
	}
	return 0;
}

static int sem_waiter(void *arg)
{
	int i;

	for (i = 0; i < 10; ++i) {
		pr_info("Waiting semaphore %d times\n", i);
		sem_wait(&sem);
	}
	return 0;
}

unsigned char stack_waiter[4096];
unsigned char stack_poster[4096];

static int test_semaphore(void)
{
	pr_info("Test Semaphore\n");

	sem_init(&sem, 0);
	thread_waiter = thread_create("waiter", stack_waiter, sizeof(stack_waiter),
				      sem_waiter, NULL);
	thread_poster = thread_create("poster", stack_poster, sizeof(stack_poster),
				      sem_poster, NULL);

	sched_start();

	return 0;
}

test_case(test_semaphore);
test_case(test_multi_thread);
