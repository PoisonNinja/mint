#pragma once

#define EXCEPTIONS_MAX 64

#include <arch/cpu/registers.h>

struct registers;

typedef int (*exception_handler_t)(struct registers *, void *);

struct exception_handler {
    exception_handler_t handler;
    const char *dev_name;
    void *dev_id;
    struct exception_handler *next, *prev;
};

extern void exception_handler_register(int exception_number,
                                       struct exception_handler *handler);
extern void exception_handler_unregister(int exception_number,
                                         struct exception_handler *handler);
