#pragma once

#include <lib/list.h>
#include <types.h>

struct console {
    const char *name;
    int (*write)(const char *, size_t);
    struct list_head list;
};

extern void console_register(struct console *console);
extern void console_write(const char *message, size_t length);
