#include <arch/cpu/registers.h>
#include <kernel.h>

void interrupt_dispatch(struct registers* regs)
{
    printk(INFO, "Received interrupt\n");
}
