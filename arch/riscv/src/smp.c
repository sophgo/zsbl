#include <smp.h>
#include <stdlib.h>
#include <arch.h>
#include <stdio.h>
#ifndef CONFIG_SMP_NUM
#error "we need smp number"
#endif

long secondary_core_runtime_address;

typedef void (*jump_fun)(long hartid, long dtb_addr, long dynamic_info_addr);

void release_other_core(long jump_addr)
{
        secondary_core_runtime_address = jump_addr;
}

void jump_to(long jump_addr, long hartid, long dtb_addr, long dynamic_info_addr)
{
	jump_fun fun = (jump_fun)jump_addr;

	asm volatile ("fence.i");
	fun(hartid, dtb_addr, dynamic_info_addr);
}

struct smp_context{
	void *sp;
	void (*fn)(void *priv);
	void *priv;
	unsigned long stack_size;
	char context[12*8];
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
	__asm__ volatile("fence rw, rw":::);

	smp_context[core_id].fn = fn;
	__asm__ volatile("fence rw, rw":::);

}
