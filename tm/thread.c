#include <tm/process.h>
#include <tm/thread.h>

struct thread* current_thread;

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
