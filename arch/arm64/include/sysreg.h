#ifndef __SYSREG_H__
#define __SYSREG_H__

#define read_sysreg(sr)							\
	({								\
		unsigned long _sr;					\
		asm volatile ("mrs	%0, " #sr :"=r"(_sr)::);	\
		_sr;							\
	})

#define write_sysreg(sr, val)						\
	({ asm volatile ("msr	" #sr ", %0" : : "r"(val):); })

static inline int current_el(void)
{
	return (read_sysreg(currentel) >> 2) & 0x03;
}

#endif
