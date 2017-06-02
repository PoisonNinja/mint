#pragma once

#define EXCEPTIONS_MAX 64
#define INTERRUPTS_MAX 256

#include <lib/list.h>

struct registers;

typedef int (*exception_handler_t)(struct registers *, void *);
typedef int (*irq_handler_t)(struct registers *, void *);

struct exception_handler {
    exception_handler_t handler;
    const char *dev_name;
    void *dev_id;
    struct list_head list;
};

struct interrupt_handler {
    irq_handler_t handler;
    const char *dev_name;
    void *dev_id;
    struct list_head list;
};

extern void exception_handler_register(int exception_number,
                                       struct exception_handler *handler);
void interrupt_handler_register(int interrupt_number,
                                struct interrupt_handler *handler);

extern void interrupt_disable(void);
extern void interrupt_enable(void);
