#ifndef __CDEFS_H__
#define __CDEFS_H__

#define __dead2		__attribute__((__noreturn__))
#define __deprecated	__attribute__((__deprecated__))
#define __packed	__attribute__((__packed__))
#define __used		__attribute__((__used__))
#define __unused	__attribute__((__unused__))
#define __aligned(x)	__attribute__((__aligned__(x)))
#define __section(x)	__attribute__((__section__(x)))

#endif
