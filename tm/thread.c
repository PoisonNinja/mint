#include <tm/process.h>
#include <tm/thread.h>

extern void arch_thread_set_stack(addr_t stack);

void thread_set_stack(addr_t stack)
{
    return arch_thread_set_stack(stack);
}

extern void arch_thread_switch(struct thread* current, struct thread* next);

void thread_switch(struct thread* thread)
{
    thread_set_stack(thread->kernel_stack);
}
