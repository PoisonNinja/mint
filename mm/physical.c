#include <arch/mm/mmap.h>
#include <kernel.h>
#include <lib/buddy.h>
#include <mm/heap.h>
#include <mm/physical.h>

struct physical_region {
    addr_t base;
    size_t size;
    struct buddy* buddy;
};

static struct physical_region normal_region;

void physical_init(size_t size)
{
    normal_region.base = 0x0;
    normal_region.size = size;
    normal_region.buddy =
        buddy_init(normal_region.base, PHYS_START, normal_region.size, 12, 28);
}

void physical_free_region(addr_t start, size_t size)
{
    buddy_free_region(normal_region.buddy, start, size);
}
