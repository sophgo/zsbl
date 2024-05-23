#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <rwonce.h>

#ifdef CONFIG_SMP
#error "smp not supported yet"
#endif

#define SPIN_LOCK_INITIALIZER   \
	{                       \
		.lock = 0,	\
	}


typedef struct spinlock {
	int lock;
} spinlock_t;

static inline void spin_lock_init(spinlock_t *lock)
{
	WRITE_ONCE(lock->lock, 0);
}

int spin_trylock(spinlock_t *lock);

static inline int spin_is_locked(spinlock_t *lock)
{
	return READ_ONCE(lock->lock);
}

static inline void spin_lock(spinlock_t *lock)
{
	while (1) {
		if (spin_is_locked(lock))
			continue;

		if (spin_trylock(lock))
			break;
	}
}

static inline void spin_unlock(spinlock_t *lock)
{
	WRITE_ONCE(lock->lock, 0);
}

/* disable irq is enough for uniprocessor system */
#define spin_lock_irqsave(lock, flags)		\
	do {					\
		flags = arch_local_irq_save();	\
		spin_lock(lock);		\
	} while (0)

#define spin_unlock_irqrestore(lock, flags)	\
	do {					\
		arch_local_irq_restore(flags);	\
		spin_unlock(lock);		\
	} while (0)


#endif
