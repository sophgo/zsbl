#ifndef __STDINT_H__
#define __STDINT_H__

/* TODO: this may in arch directory and various by length of processor */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;

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
