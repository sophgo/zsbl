#ifndef __STDINT_H__
#define __STDINT_H__

#include <types.h>

#define UINT32_C(x)	(x ## U)
#define U(x)		(x ## U)
#define UL(x)		(x ## UL)
#define ULL(x)		(x ## ULL)

#define BIT(nr)		(UL(1) << (nr))
#define BIT_32(nr)	(U(1) << (nr))
#define BIT_64(nr)	(ULL(1) << (nr))

#define GENMASK(h, l) \
	(((~UL(0)) << (l)) & (~UL(0) >> (sizeof(unsigned long) * 8 - 1 - (h))))

#endif
