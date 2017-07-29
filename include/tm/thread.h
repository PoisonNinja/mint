#pragma once

#include <arch/cpu/registers.h>
#include <arch/tm/thread.h>
#include <tm/process.h>

#define STACK_SIZE 0x4000

struct thread {
    struct process* process;
    struct registers* registers;
    addr_t kernel_stack;
    struct thread *next, *prev;
};

extern struct thread* current_thread;

extern void thread_switch(struct interrupt_ctx* ctx, struct thread* current,
                          struct thread* next);

extern void sched_init(void);
