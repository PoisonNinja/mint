#include <arch/cpu/registers.h>
#include <arch/mm/mm.h>
#include <kernel.h>
#include <kernel/stacktrace.h>
#include <lib/math.h>
#include <mm/virtual.h>

extern void arch_virtual_map(struct memory_context* context, addr_t virtual,
                             addr_t physical, uint8_t flags);

void virtual_map(struct memory_context* context, addr_t virtual,
                 addr_t physical, size_t size, uint8_t flags)
{
    if (!size || !context)
        return;
    size_t mapped = 0;
    while (mapped < size) {
        arch_virtual_map(context, virtual + mapped, physical + mapped, flags);
        mapped += (flags & PAGE_HUGE) ? PAGE_HUGE_SIZE : PAGE_SIZE;
    }
}

extern void arch_virtual_unmap(struct memory_context* context, addr_t virtual);

void virtual_unmap(struct memory_context* context, addr_t virtual)
{
    if (!context)
        return;
    arch_virtual_unmap(context, virtual);
}

void virtual_fault(struct registers* __attribute__((unused)) registers,
                   addr_t address, uint8_t reason)
{
    printk(ERROR, "PAGE FAULT\n");
    printk(ERROR, "  Address: %p\n", address);
    printk(ERROR, "  Present: %u\n", (reason & FAULT_PRESENT) ? 1 : 0);
    printk(ERROR, "  Write: %u\n", (reason & FAULT_WRITE) ? 1 : 0);
    printk(ERROR, "  User: %u\n", (reason & FAULT_USER) ? 1 : 0);
    printk(ERROR, "  Reserved: %u\n", (reason & FAULT_RESERVED) ? 1 : 0);
    printk(ERROR, "  Instruction fetch: %u\n", (reason & FAULT_IF) ? 1 : 0);
    stacktrace();
    for (;;)
        __asm__("hlt");
}
