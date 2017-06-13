#include <kernel.h>
#include <mm/heap.h>

// Variables for early heap
static addr_t early_heap_start = 0;
static addr_t early_heap_end = 0;
static addr_t early_heap_extent = 0;
static uint8_t early_heap_configured = 0;
static addr_t early_heap_watermark = 0;

void early_malloc_set_properties(addr_t start, addr_t extent)
{
    if (!start || !extent)
        return;
    // No, I don't want to configure this twice
    if (early_heap_configured) {
        printk(WARNING, "early_malloc: Already configured! Allowing this "
                        "action, but unexpected behavior may occur\n");
    }
    // Set the early heap state
    early_heap_start = start;
    early_heap_extent = extent;
    // Precalculate the heap end
    early_heap_end = start + extent;
    // Set the heap pointer to the start of the memory region
    early_heap_watermark = start;
    printk(INFO, "early_malloc: Start at %p, extent is %p\n", start, extent);
    early_heap_configured = 1;
}

void* __attribute__((malloc)) early_malloc(size_t size)
{
    // Make sure we don't overshoot the extent
    if (early_heap_watermark + size > early_heap_end) {
        printk(INFO, "early_malloc: Request too large :(\n");
    }
    // Convert water into a void pointer to return
    void* ret = (void*)early_heap_watermark;
    early_heap_watermark += size;
    return ret;
}
