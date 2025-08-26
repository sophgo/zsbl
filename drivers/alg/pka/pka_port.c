#include <stdlib.h>
#include <driver/pka/pka_port.h>

int pka_atomic_readi(int *x)
{
	int tmp = *(int *)x;

	pka_barrier();
	return tmp;
}

void pka_atomic_writei(int *x, int v)
{
	*(int *)x = v;
	pka_barrier();
}

int pka_mutex_init(struct pka_mutex *mutex)
{
	return 0;
}
int pka_mutex_destroy(struct pka_mutex *mutex)
{
	return 0;
}
int pka_mutex_lock(struct pka_mutex *mutex)
{
	return 0;
}
int pka_mutex_unlock(struct pka_mutex *mutex)
{
	return 0;
}

int pka_sem_init(struct pka_sem *sem)
{
	pka_atomic_writei(&sem->sem, 0);
	return 0;
}
int pka_sem_destroy(struct pka_sem *sem)
{
	pka_atomic_writei(&sem->sem, 0);
	return 0;
}
int pka_sem_post(struct pka_sem *sem)
{
	sem->sem++;
	pka_barrier();
	return 0;
}
int pka_sem_wait(struct pka_sem *sem)
{
	while (pka_atomic_readi(&sem->sem) <= 0)
		;
	sem->sem--;
	pka_barrier();
	return 0;
}

