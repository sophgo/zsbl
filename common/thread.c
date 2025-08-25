#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <asm.h>
#include <assert.h>
#include <timer.h>
#include <arch_thread.h>
#include <atomic.h>
#include <common/common.h>
#include <common/thread.h>
#include <common/spinlock.h>

static LIST_HEAD(thread_list);
static spinlock_t thread_lock = SPIN_LOCK_INITIALIZER;

static struct thread *current;

struct thread *sched_get_current(void)
{
	return current;
}

static void thread_end_point(int exit_code)
{
	pr_info("Thread %s end\n", current->name);

	current->exit_code = exit_code;
	current->state = THREAD_STATE_DEAD;

	spin_lock(&thread_lock);

	list_del(&current->list);

	current = NULL;

	spin_unlock(&thread_lock);

	/* reschedule, because we have already remove current thread from thread_list
	 * so this thread will never be scheduled again
	 */
	sched_yield();
}

struct thread *thread_create(const char *name, void *stack_base, unsigned long stack_size,
			     int (*func)(void *), void *arg)
{
	struct thread *t = malloc(sizeof(struct thread));

	if (!t)
		return NULL;

	t->cpu_ctx = arch_thread_create_cpu_ctx(stack_base, stack_size,
						func, arg, thread_end_point);
	if (!t->cpu_ctx) {
		free(t);
		return NULL;
	}

	t->name = name;
	t->priority = 0;
	t->stack_base = stack_base;
	t->stack_size = stack_size;
	t->func = func;
	t->arg = arg;
	t->state = THREAD_STATE_NEW;

	spin_lock(&thread_lock);
	list_add_tail(&t->list, &thread_list);
	spin_unlock(&thread_lock);

	return t;
}

int thread_destroy(struct thread *t)
{
	int exit_code = t->exit_code;

	/* only dead thread can be destroyed, running or runnable thread can't */
	assert(t->state == THREAD_STATE_DEAD);

	spin_lock(&thread_lock);
	free(t->cpu_ctx);
	free(t);
	spin_unlock(&thread_lock);

	return exit_code;
}

struct arch_cpu_ctx *sched_thread(struct arch_cpu_ctx *cpu_ctx)
{
	struct thread *next = NULL;

	/* we are now in init context, are not in any thread context */
	if (!current) {
		/* select 1st thread and schedule it on cpu */
		current = list_first_entry(&thread_list, struct thread, list);
		current->state = THREAD_STATE_RUNNING;
		return current->cpu_ctx;
	}

	/* save context to the previous thread */
	arch_thread_save_ctx(current->cpu_ctx, cpu_ctx);

	if (list_is_last(&current->list, &thread_list))
		next = list_first_entry(&thread_list, struct thread, list);
	else
		next = list_next_entry(current, list);

	/* this thread has finished all jobs */
	/* remove it from list */
	/* we should have an idle thread, and idle thread never exit */
	if (current->state == THREAD_STATE_DEAD)
		list_del(&current->list);

	current->state = THREAD_STATE_RUNNABLE;
	next->state = THREAD_STATE_RUNNING;

	/* new current */
	current = next;

	return current->cpu_ctx;
}

static int sched_idle_thread(void *data)
{
	int i = 0;

	/* idle thread never exit */
	while (true) {
		mdelay(1000);
		printf("reschedule in idle %d\n", i++);
		sched_yield();
	}

	return 0;
}

void sched_yield(void)
{
	arch_thread_yield();
}

void sched_tick(void *unused)
{
}

/* scheduler start and never return */
int sched_start(void)
{
	int err;
	/* create an idle thread */
	struct thread *idle_thread;
	void *idle_thread_stack_base;
	const unsigned int idle_thread_stack_size = 4096;
	const unsigned int sched_tick_hz = 100;

	idle_thread_stack_base = malloc(idle_thread_stack_size);
	if (!idle_thread_stack_base) {
		err = -ENOMEM;
		goto error_alloc_idle_stack;
	}

	idle_thread = thread_create("idle", idle_thread_stack_base, idle_thread_stack_size,
				    sched_idle_thread, NULL);
	if (!idle_thread) {
		err = -ENOMEM;
		goto error_create_thread;
	}

	/* start timer */
	err = timer_enable_irq(timer_frequency() / sched_tick_hz, sched_tick, NULL);
	if (err) {
		pr_err("Failed to start schedule timer\n");
		goto error_timer_start;
	}

	sched_yield();

	return 0;

error_timer_start:
	thread_destroy(idle_thread);

error_create_thread:
	free(idle_thread_stack_base);

error_alloc_idle_stack:

	assert(false);

	return err;
}

static atomic_t preempt_count;

void sched_preempt_disable(void)
{
	arch_preempt_disable();

	atomic_add_return(&preempt_count, 1);
}

void sched_preempt_enable(void)
{
	long count;

	count = atomic_sub_return(&preempt_count, 1);

	if (count == 0)
		arch_preempt_enable();
}


