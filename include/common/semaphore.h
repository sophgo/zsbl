#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <lib/list.h>
#include <atomic.h>
#include <common/spinlock.h>

struct sem_thread {
	struct list_head list;
	struct thread *thread;
};

struct sem {
	spinlock_t lock;
	long count;
	struct list_head wait_list;
};

int sem_init(struct sem *sem, long value);
int sem_wait(struct sem *sem);
int sem_post(struct sem *sem);

#endif
