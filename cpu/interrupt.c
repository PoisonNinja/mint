#include <arch/cpu/registers.h>
#include <kernel.h>
#include <stdatomic.h>

extern void arch_interrupt_disable(void);
extern void arch_interrupt_enable(void);

_Atomic int interrupt_depth = 0;

void interrupt_disable(void)
{
    if (!atomic_fetch_add(&interrupt_depth, 1))
        arch_interrupt_disable();
}

void interrupt_enable(void)
{
    if (atomic_fetch_sub(&interrupt_depth, 1) == 1)
        arch_interrupt_enable();
}

void interrupt_dispatch(struct registers* regs)
{
    printk(INFO, "Received interrupt %d\n", regs->int_no);
}
