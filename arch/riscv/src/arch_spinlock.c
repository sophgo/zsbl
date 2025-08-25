#include <barrier.h>
#include <arch_spinlock.h>

int arch_spin_lock_check(struct arch_spinlock *lock)
{
	return (lock->lock == __RISCV_SPIN_UNLOCKED) ? 0 : 1;
}

int arch_spin_trylock(struct arch_spinlock *lock)
{
	int tmp = 1, busy;

	__asm__ __volatile__(
		"	amoswap.w %0, %2, %1\n" RISCV_ACQUIRE_BARRIER
		: "=r"(busy), "+A"(lock->lock)
		: "r"(tmp)
		: "memory");

	return !busy;
}

void arch_spin_unlock(struct arch_spinlock *lock)
{
	__smp_store_release(&lock->lock, __RISCV_SPIN_UNLOCKED);
}
