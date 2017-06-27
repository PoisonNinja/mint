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

static struct physical_region dma_region;
static struct physical_region normal_region;

void physical_init(size_t size)
{
    dma_region.base = 0x0;
    dma_region.size = 0x1000000;
    dma_region.buddy =
        buddy_init(dma_region.base, PHYS_START, dma_region.size, 12, 16);
    normal_region.base = 0x1000000;
    normal_region.size = size - 0x1000000;
    normal_region.buddy =
        buddy_init(normal_region.base, PHYS_START, normal_region.size, 12, 28);
}

void physical_free_region(addr_t start, size_t size)
{
    if (start < 0x1000000) {
        if (start + size > 0x1000000) {
            size_t dma_size = 0x1000000 - start;
            size -= dma_size;
            buddy_free_region(dma_region.buddy, start, dma_size);
            start = 0x1000000;
            buddy_free_region(normal_region.buddy, start, size);
        } else {
            buddy_free_region(dma_region.buddy, start, size);
        }
    } else {
        buddy_free_region(normal_region.buddy, start, size);
    }
}
