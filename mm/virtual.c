#include <arch/mm/mm.h>
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
