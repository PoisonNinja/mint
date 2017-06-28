#include <arch/mm/mmap.h>
#include <boot/bootinfo.h>
#include <kernel.h>
#include <mm/physical.h>

extern void arch_mm_init(void);

void mm_init(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%llu KiB of memory available\n", bootinfo->total_mem);
    printk(INFO, "Highest address is %p\n", bootinfo->highest_mem);
    arch_mm_init();
    physical_init(bootinfo->highest_mem, DMA_MAX);
    for (struct mint_memory_region* region = bootinfo->memregions; region;
         region = region->next) {
        printk(INFO, "  Start: %p. Size: %p. Type: 0x%X\n", region->addr,
               region->size, region->type);
        if (region->type == MEMORY_TYPE_AVAILABLE) {
            physical_free_region(region->addr, region->size);
        }
    }
}
