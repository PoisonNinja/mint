#pragma once

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define container_of(ptr, type, member)                    \
    ({                                                     \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })

enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CONTINUE,
};
#define printk(level, ...) _printk(level, __VA_ARGS__)
int _printk(int level, const char *format, ...);
