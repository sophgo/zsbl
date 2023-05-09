#ifndef __SMP_H__
#define __SMP_H__

#define DEFAULT_STACK_SIZE 4096

int get_core_id(void);
void wake_up_other_core(int core_id, void (*fn)(void *priv),
			void *priv, void *sp, int stack_size);


#endif