#ifndef __LIBC_H__
#define __LIBC_H__

#define printf sbi_printf
#define thread_safe_printf sbi_printf

#define strcmp sbi_strcmp
#define strncmp sbi_strncmp
#define strnlen sbi_strnlen
#define strncpy sbi_strncpy
#define strchr sbi_strchr
#define strrchr sbi_strrchr
#define memmove sbi_memmove
#define memcmp sbi_memcmp
#define memchr sbi_memchr

#endif
