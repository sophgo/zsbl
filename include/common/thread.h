#ifndef __THREAD_H__
#define __THREAD_H__

#include <lib/list.h>
#include <arch_thread.h>

struct thread {
	struct arch_cpu_ctx *cpu_ctx;
	struct list_head list;
	int priority;
	int state;
	uint64_t sleep_time;
	int exit_code;
	const char *name;
	void *stack_base;
	unsigned long stack_size;
	int (*func)(void *);
	void *arg;
};

/*
 * RUNNING->RUNNABLE: sched_thread schedule a thread out of cpu
 * RUNNABLE->RUNNING: sched_thread schedule a thread on cpu
 * RUNNING->BLOCK: thread_sleep put current thread in sleep mode
 * BLOCK->RUNNABLE: sched_thread sleep time is over, but not schedule it on cpu
 * BLOCK->RUNNING: sched_thread sleep time is over, and schedule it on cpu
 * RUNNING->DEAD: exited from its routine
 */
enum {
	THREAD_STATE_RUNNABLE,
	THREAD_STATE_RUNNING,
	THREAD_STATE_BLOCK,
	THREAD_STATE_DEAD,
};

struct arch_cpu_ctx *sched_thread(struct arch_cpu_ctx *cpu_ctx);
void sched_yield(void);
int sched_start(void);
struct thread *thread_create(const char *name, void *stack_base, unsigned long stack_size,
			     int (*func)(void *), void *arg);

/*
 * negative: invalid priority.
 * 0: idle thread priority, we should make sure idle thread always use lowest priority, and
 * no one have the same priority with idle thread.
 * 1: lowest valid priority and this is the default value when thread just created.
 * others: the bigger the higher.
 * so the valid range of p is from 1 to max integer
 *
 * This API causes a thread schedule immediately, that means if you set the priority to
 * a lower one(less one), this thread may be put into background immediately, vice versa.
 */
int thread_set_priority(struct thread *t, int p);
int thread_destroy(struct thread *t);
struct thread *sched_get_current(void);

void sched_preempt_disable(void);
void sched_preempt_enable(void);

void sched_msleep(unsigned long time);

#endif
