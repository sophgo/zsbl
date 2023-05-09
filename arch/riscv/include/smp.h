#ifndef __SMP_H__
#define __SMP_H__

#define MAIN_BOOT_CORE	1
#define DEFAULT_STACK_SIZE 4096

#ifndef __ASSEMBLER__

void wake_up_other_core(int core_id, void (*fn)(void *priv),
			void *priv, void *sp, int stack_size);
void jump_to(long jump_addr, long hartid, long dtb_addr);

#endif

#endif