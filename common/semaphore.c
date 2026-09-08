#include <common/common.h>
#include <common/semaphore.h>
#include <common/thread.h>
#include <errno.h>

int sem_init(struct sem *sem, long value)
{
	sem->count = value;
	INIT_LIST_HEAD(&sem->wait_list);

	return 0;
}

int sem_wait(struct sem *sem)
{
	struct sem_thread t;
	int in_list;

	t.thread = sched_get_current();
	INIT_LIST_HEAD(&t.list);

	in_list = false;
	while (true) {
		spin_lock(&sem->lock);
		if (sem->count > 0) {
			--sem->count;

			if (in_list)
				list_del(&t.list);

			spin_unlock(&sem->lock);
			break;
		}
		in_list = true;
		t.thread->state = THREAD_STATE_BLOCK;
		list_add_tail(&t.list, &sem->wait_list);
		spin_unlock(&sem->lock);

		sched_yield();
	}

	return 0;
}

int sem_post(struct sem *sem)
{
	struct sem_thread *t, *best = NULL;
	int best_prio = -1;

	spin_lock(&sem->lock);

	++sem->count;

	list_for_each_entry(t, &sem->wait_list, list) {
		if (t->thread->state != THREAD_STATE_BLOCK)
			continue;
		if (t->thread->priority > best_prio) {
			best_prio = t->thread->priority;
			best = t;
		}
	}

	if (best) {
		best->thread->state = THREAD_STATE_RUNNABLE;
		list_del_init(&best->list);
	}

	spin_unlock(&sem->lock);

	sched_yield();

	return 0;
}

