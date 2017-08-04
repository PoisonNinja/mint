#pragma once

#include <arch/cpu/registers.h>
#include <arch/tm/thread.h>
#include <lib/list.h>
#include <tm/process.h>

#define STACK_SIZE 0x4000

struct thread {
    struct process* process;
    struct registers registers;
    addr_t kernel_stack;
    struct list_element runqueue_list;
    struct list_element process_list;
};

extern struct thread* current_thread;

extern void thread_switch(struct interrupt_ctx* ctx, struct thread* current,
                          struct thread* next);
extern struct thread* kthread_create(void (*fn)(void* data), void* data);
