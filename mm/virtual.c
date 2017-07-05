#include <arch/mm/mm.h>
#include <mm/virtual.h>

void arch_virtual_map(struct memory_context* context, addr_t virtual,
                      addr_t physical, uint8_t flags);

void virtual_map(struct memory_context* context, addr_t virtual,
                 addr_t physical, size_t size, uint8_t flags)
{
    size_t mapped = 0;
    if (!size || !context)
        return;
    while (mapped < size) {
        arch_virtual_map(context, virtual + mapped, physical + mapped, flags);
        mapped += (flags & PAGE_HUGE) ? PAGE_HUGE_SIZE : PAGE_SIZE;
    }
}
