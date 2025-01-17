#ifndef __TYPES_H__
#define __TYPES_H__

typedef char			s8;
typedef unsigned char		u8;
typedef unsigned char		uint8_t;

typedef short			s16;
typedef unsigned short		u16;
typedef short			int16_t;
typedef unsigned short		uint16_t;

typedef int			s32;
typedef unsigned int		u32;
typedef int			int32_t;
typedef unsigned int		uint32_t;

typedef long long		s64;
typedef unsigned long long	u64;
typedef long long		int64_t;
typedef unsigned long long	uint64_t;
#define PRILX			"016llx"

typedef int			bool;
typedef unsigned long		ulong;
typedef unsigned long		uintptr_t;
typedef unsigned long		size_t;
typedef long			ssize_t;
typedef unsigned long		virtual_addr_t;
typedef unsigned long		virtual_size_t;
typedef unsigned long		physical_addr_t;
typedef unsigned long		physical_size_t;

#define NULL			((void *)0)

#define __packed		__attribute__((packed))
#define __noreturn		__attribute__((noreturn))
#define __aligned(x)		__attribute__((aligned(x)))

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })

#define array_size(x) 	(sizeof(x) / sizeof((x)[0]))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(a, lo, hi) MIN(MAX(a, lo), hi)

#define STR(x) XSTR(x)
#define XSTR(x) #x

#define ROUNDUP(a, b) ((((a)-1) / (b) + 1) * (b))
#define ROUNDDOWN(a, b) ((a) / (b) * (b))

#endif
