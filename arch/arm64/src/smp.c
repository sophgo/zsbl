#include <smp.h>
#include <stdlib.h>
#include <arch.h>

struct smp_context{
	void *sp;
	void (*fn)(void *priv);
	void *priv;
	unsigned long stack_size;
	char context[13*8];
};

struct smp_context smp_context[CONFIG_SMP_NUM] = {0};

void wake_up_other_core(int core_id, void (*fn)(void *priv),
			void *priv, void *sp, int stack_size)
{
	if (sp == NULL){
		sp = malloc(DEFAULT_STACK_SIZE);
		if (sp == NULL)
			return;
		smp_context[core_id].sp = sp;
		smp_context[core_id].stack_size = DEFAULT_STACK_SIZE;
	}else{
		smp_context[core_id].sp = sp;
		smp_context[core_id].stack_size = stack_size;
	}

	smp_context[core_id].priv = priv;

	__asm__ volatile("":::"memory");
	__asm__ volatile("isb":::);

	smp_context[core_id].fn = fn;
	__asm__ volatile("isb":::);
	__asm__ volatile("sev");

}

int get_core_id(void)
{
	unsigned long id = 0;
	unsigned long cluster = 0;

	__asm__ volatile("mrs %0, mpidr_el1":"=r"(id)::"memory");

	cluster = id;
	id &= MPIDR_CPU_MASK;
	cluster &= MPIDR_CLUSTER_MASK;

	id = (cluster >> 6) | id;

	return id;
}
