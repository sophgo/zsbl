#ifndef __RISCV_LOCKS_H__
#define __RISCV_LOCKS_H__

struct arch_spinlock{
	volatile long lock;
};

#define __RISCV_SPIN_UNLOCKED 0

#define ARCH_SPIN_LOCK_INIT(_lptr) (_lptr)->lock = __RISCV_SPIN_UNLOCKED

#define ARCH_SPIN_LOCK_INITIALIZER			\
	{						\
		.lock = __RISCV_SPIN_UNLOCKED,		\
	}

int arch_spin_lock_check(struct arch_spinlock *lock);
int arch_spin_trylock(struct arch_spinlock *lock);
void arch_spin_unlock(struct arch_spinlock *lock);

#endif
