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

typedef unsigned long uintptr_t;

#define UINT32_C(x)	(x ## U)

#endif
