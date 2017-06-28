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

void physical_init(size_t size, addr_t dma_boundary)
{
    dma_region.base = 0x0;
    dma_region.size = dma_boundary;
    dma_region.buddy =
        buddy_init(dma_region.base, PHYS_START, dma_region.size, 12, 16);
    normal_region.base = dma_boundary;
    normal_region.size = size - dma_boundary;
    normal_region.buddy =
        buddy_init(normal_region.base, PHYS_START, normal_region.size, 12, 28);
}

void* physical_alloc(size_t size, uint8_t flags)
{
    if (flags & PHYS_DMA) {
        return buddy_alloc(dma_region.buddy, size);
    } else {
        return buddy_alloc(normal_region.buddy, size);
    }
}

void physical_free(void* addr, size_t size)
{
    if ((addr_t)addr < dma_region.size) {
        return buddy_free(dma_region.buddy, addr, size);
    } else {
        return buddy_free(normal_region.buddy, addr, size);
    }
}

void physical_free_region(addr_t start, size_t size)
{
    if (start < dma_region.size) {
        if (start + size > dma_region.size) {
            size_t dma_size = dma_region.size - start;
            size -= dma_size;
            buddy_free_region(dma_region.buddy, start, dma_size);
            start = dma_region.size;
            buddy_free_region(normal_region.buddy, start, size);
        } else {
            buddy_free_region(dma_region.buddy, start, size);
        }
    } else {
        buddy_free_region(normal_region.buddy, start, size);
    }
}
