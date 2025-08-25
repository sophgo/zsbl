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
int thread_destroy(struct thread *t);
struct thread *sched_get_current(void);

void sched_preempt_disable(void);
void sched_preempt_enable(void);

void sched_msleep(unsigned long time);

#endif
