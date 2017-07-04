#include <mm/virtual.h>

void arch_virtual_map(struct memory_context* context, addr_t virtual,
                      addr_t physical, uint8_t flags);

void virtual_map(struct memory_context* context, addr_t virtual,
                 addr_t physical, size_t size, uint8_t flags)
{
    if (!size || !context)
        return;
    return arch_virtual_map(context, virtual, physical, flags);
}
