#include <boot/bootinfo.h>
#include <kernel.h>
#include <mm/virtual.h>

#include <arch/mm/mm.h>

extern void arch_mm_init(struct mint_bootinfo* bootinfo,
                         struct memory_context* context);

struct memory_context kernel_context;

void mm_init(struct mint_bootinfo* bootinfo)
{
    printk(INFO, "%llu KiB of memory available\n", bootinfo->total_mem);
    printk(INFO, "Highest address is %p\n", bootinfo->highest_mem);
    arch_mm_init(bootinfo, &kernel_context);
}
