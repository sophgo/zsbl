#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <arch_spinlock.h>

typedef struct {
	struct arch_spinlock lock;
} spinlock_t;

#define SPIN_LOCK_INIT(_lptr)	ARCH_SPIN_LOCK_INIT(_lptr)

#define SPIN_LOCK_INITIALIZER	ARCH_SPIN_LOCK_INITIALIZER

int spin_lock_check(spinlock_t *lock);
int spin_trylock(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif
