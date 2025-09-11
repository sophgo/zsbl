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

	spin_lock(&thread_lock);

	current->exit_code = exit_code;
	current->state = THREAD_STATE_DEAD;

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

	memset(t, 0, sizeof(struct thread));

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
	t->state = THREAD_STATE_RUNNABLE;

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

static struct thread *next_thread(struct thread *t)
{
	if (list_is_last(&t->list, &thread_list))
		return list_first_entry(&thread_list, struct thread, list);
	else
		return list_next_entry(t, list);
}

struct arch_cpu_ctx *sched_thread(struct arch_cpu_ctx *cpu_ctx)
{
	struct thread *next = NULL, *t = NULL;
	int hp;

	/* no thread created */
	if (current == NULL)
		return cpu_ctx;

	if (current->state == THREAD_STATE_RUNNING || current->state == THREAD_STATE_BLOCK)
		arch_thread_save_ctx(current->cpu_ctx, cpu_ctx);

	/* select the highest priority thread and is runnable */
	/* idle thread always runnable */
	hp = -1;

	for(t = next_thread(current); (t != current); t = next_thread(t)) {
		if (!(t->state == THREAD_STATE_RUNNABLE || t->state == THREAD_STATE_RUNNING))
			continue;

		if (t->priority > hp) {
			next = t;
			hp = t->priority;
		}
	}

	if (t->state == THREAD_STATE_RUNNABLE || t->state == THREAD_STATE_RUNNING)
		if (t->priority > hp)
			next = t;


	/* clean up finished thread */
	if (current->state == THREAD_STATE_DEAD)
		list_del(&current->list);
	else if (current->state == THREAD_STATE_RUNNING)
		current->state = THREAD_STATE_RUNNABLE;

	next->state = THREAD_STATE_RUNNING;

	/* new current */
	current = next;

	// printf("running %s\n", current->name);

	return current->cpu_ctx;
}

static int sched_idle_thread(void *data)
{
	int i = 0;

	i = i;

	/* idle thread never exit */
	while (true) {
		// printf("idle thread on cpu%d %d times\n", current_hartid(), i++);
		wfi();
	}

	return 0;
}

void sched_yield(void)
{
	arch_thread_yield();
}

static const unsigned int sched_tick_hz = 100;
static uint64_t sched_tick_counter;

static void sched_tick(void *unused)
{
	struct thread *t;

	++sched_tick_counter;

	list_for_each_entry(t, &thread_list, list) {
		if (t->sleep_time) {
			--t->sleep_time;
			if (t->sleep_time == 0)
				t->state = THREAD_STATE_RUNNABLE;
		}
	}
}

/* must be called in thread context, and only apply for current thread */
void sched_msleep(unsigned long time)
{
	if (time) {
		sched_preempt_disable();
		current->sleep_time = time * sched_tick_hz / 1000 + 1;
		current->state = THREAD_STATE_BLOCK;
		sched_preempt_enable();
	}

	sched_yield();
}

/* scheduler start and never return */
int sched_start(void)
{
	int err;
	/* create an idle thread */
	struct thread *idle_thread;
	void *idle_thread_stack_base;
	const unsigned int idle_thread_stack_size = 4096;

	/* check if thread list is empty */
	if (list_empty(&thread_list))
		return -EINVAL;

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

	current = idle_thread;

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


