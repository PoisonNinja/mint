#include <cpu/interrupt.h>
#include <kernel/stacktrace.h>

extern void arch_stacktrace(void);

void stacktrace(void)
{
    interrupt_disable();
    arch_stacktrace();
    interrupt_enable();
}
