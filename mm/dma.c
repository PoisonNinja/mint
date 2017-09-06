#include <arch/mm/mmap.h>
#include <lib/math.h>
#include <mm/dma.h>
#include <mm/heap.h>
#include <mm/physical.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

static struct valloc_region* dma_region = NULL;

struct dma_region* dma_alloc(size_t size)
{
    size = ROUND_UP(size, 0x1000);
    addr_t physical = (addr_t)physical_alloc(size, PHYS_DMA);
    addr_t virtual = valloc_alloc(dma_region, size);
    virtual_map(&kernel_context, virtual, physical, size,
                PAGE_PRESENT | PAGE_WRITABLE);
    struct dma_region* region = kmalloc(sizeof(struct dma_region));
    region->physical_base = physical;
    region->virtual_base = virtual;
    region->extent = size;
    return region;
}

void dma_init(void)
{
    dma_region = valloc_create(DMA_START, DMA_END - DMA_START);
}
