#include <mm/heap.h>
#include <tm/process.h>
#include <tm/sched.h>
#include <tm/thread.h>

#include <kernel.h>

struct thread* current_thread;
extern struct process* kernel_process;

extern void arch_thread_set_stack(addr_t stack);

void thread_set_stack(addr_t stack)
{
    return arch_thread_set_stack(stack);
}

extern void arch_thread_switch(struct interrupt_ctx* ctx,
                               struct thread* current, struct thread* next);

void thread_switch(struct interrupt_ctx* ctx, struct thread* current,
                   struct thread* next)
{
    current_thread = next;
    thread_set_stack(next->kernel_stack);
    return arch_thread_switch(ctx, current, next);
}

void __attribute__((noreturn)) kthread_exit(void)
{
    struct thread* kthread = current_thread;
    sched_remove(kthread);
    process_remove_thread(kernel_process, kthread);
    // Loop until we're descheduled
    for (;;)
        __asm__("hlt");
}

extern void arch_kthread_setup_registers(struct thread* kthread,
                                         void (*fn)(void* data), void* arg,
                                         addr_t stack, size_t stack_size);

struct thread* kthread_create(void (*fn)(void* data), void* data)
{
    struct thread* kthread = kzalloc(sizeof(struct thread));
    process_add_thread(kernel_process, kthread);
    addr_t stack = (addr_t)kmalloc(0x1000);
    arch_kthread_setup_registers(kthread, fn, data, stack, 0x1000);
    return kthread;
}
