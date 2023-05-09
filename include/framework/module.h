#ifndef __MODULE_H__
#define __MODULE_H__

typedef int (*module_init_func)(void);

#define module_init(fn)						\
	static const module_init_func				\
	__attribute__((section(".module_init"), used))		\
	__module_init_ ##fn = (fn)

#define early_init(fn)						\
	static const module_init_func				\
	__attribute__((section(".early_init"), used))		\
	__module_init_ ##fn = (fn)

#define arch_init(fn)						\
	static const module_init_func				\
	__attribute__((section(".arch_init"), used))		\
	__module_init_ ##fn = (fn)

#define plat_init(fn)						\
	static const module_init_func				\
	__attribute__((section(".plat_init"), used))		\
	__module_init_ ##fn = (fn)

#define test_case(fn)						\
	static const module_init_func				\
	__attribute__((section(".test_case"), used))		\
	__module_init_ ##fn = (fn)


#endif
