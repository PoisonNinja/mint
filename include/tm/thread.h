#pragma once

#include <arch/cpu/registers.h>
#include <tm/process.h>

#define STACK_SIZE 0x4000

struct thread {
    struct process* process;
    struct registers* registers;
    addr_t kernel_stack;
    struct thread *next, *prev;
};

extern void thread_switch(struct thread* thread);
