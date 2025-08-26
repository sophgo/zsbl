#include <barrier.h>
#include <common/spinlock.h>

#ifdef CONFIG_MULTI_THREAD
#include <common/thread.h>
#endif

int spin_lock_check(spinlock_t *lock)
{
	return arch_spin_lock_check(&lock->lock);
}

int spin_trylock(spinlock_t *lock)
{
#ifdef CONFIG_MULTI_THREAD
	sched_preempt_disable();

	if (arch_spin_trylock(&lock->lock))
		return true;

	sched_preempt_enable();
	return false;
#else
	return arch_spin_trylock(&lock->lock);
#endif
}

void spin_lock(spinlock_t *lock)
{
	while (1) {
		if (spin_lock_check(lock))
			continue;

		if (spin_trylock(lock))
			break;
	}
}

void spin_unlock(spinlock_t *lock)
{
	arch_spin_unlock(&lock->lock);
#ifdef CONFIG_MULTI_THREAD
	sched_preempt_enable();
#endif
}
