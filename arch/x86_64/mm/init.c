#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <boot/bootinfo.h>
#include <kernel.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <string.h>

__attribute__((aligned(0x1000))) static struct page_table pml3;
__attribute__((aligned(0x1000))) static struct page_table pml2[512];

extern addr_t __kernel_start;
extern addr_t __kernel_end;

static struct mint_memory_region fixup_regions[10];
static uint8_t free_fixup_region = 0;

/*
 * Intended for initial bringup. Based on seakernel initial mm bringup, but
 * rewritten to use structs instead of uint64_t. This uses large pages to
 * make mapping easier while the physical memory manager is offline since
 * we can't allocate new pages yet, only using preallocated/compiled in
 */

void x86_64_patch_pml4(struct memory_context *context)
{
    struct page_table *pml4 = (struct page_table *)(read_cr3() + KERNEL_START);
    pml4->pages[0].address = 0;
    pml4->pages[0].present = 0;
    addr_t address = 0;
    for (int i = 0; i < 512; i++) {
        pml3.pages[i].address = ((addr_t)&pml2[i] - KERNEL_START) / 0x1000;
        pml3.pages[i].present = 1;
        pml3.pages[i].writable = 1;
        for (int j = 0; j < 512; j++) {
            pml2[i].pages[j].address = address / 0x1000;
            pml2[i].pages[j].present = 1;
            pml2[i].pages[j].writable = 1;
            pml2[i].pages[j].huge_page = 1;
            address += 0x200000;
        }
    }
    pml4->pages[PML4_INDEX(PHYS_START)].address =
        ((addr_t)&pml3 - KERNEL_START) / 0x1000;
    pml4->pages[PML4_INDEX(PHYS_START)].present = 1;
    pml4->pages[PML4_INDEX(PHYS_START)].writable = 1;
    context->page_table = (addr_t)pml4;
}

/*
 * Multiboot's memory region comes mainly from BIOS calls, and it doesn't
 * do anything to mark kernel areas as used. Thus, when we blindly pass
 * in free regions to the physical manager, the physical manager happily
 * marks the kernel memory as free to. Eventually, the kernel will get
 * clobbered once that memory gets handed out to some unsuspecting
 * function requesting memory. This function loops through the memory
 * regions and fixes any overlap between the free regions and the kernel
 * space.
 */
void x86_64_fix_multiboot(struct mint_bootinfo *bootinfo)
{
    addr_t kernel_start = ROUND_DOWN((addr_t)&__kernel_start, PAGE_SIZE);
    addr_t kernel_end = ROUND_UP((addr_t)kmalloc(0) - KERNEL_START, PAGE_SIZE);
    printk(INFO, "Kernel between %p and %p\n", kernel_start, kernel_end);
    for (struct mint_memory_region *region = bootinfo->memregions; region;
         region = region->next) {
        if (region->addr <= kernel_start &&
            region->addr + region->size >= kernel_end &&
            region->type == MEMORY_TYPE_AVAILABLE) {
            printk(WARNING,
                   "Kernel located in reported 'free' region, fixing up...\n");
            /*
             * There are four possible cases in regards to kernel and the
             * region. |--| denotes the kernel, and xxx denotes free space.
             *
             * Case 1: xxxxx|xx---|. The free space is before the kernel and
             * ends before or equal to the kernel end. We only need to reduce
             * the existing size to the beginning of the kernel.
             *
             * Case 2: |----xxx|xxxxx. The free space is after the kernel and
             * starts in the kernel. We only need to move the start of the
             * region to the end of the kernel, maybe adding some buffer space.
             *
             * Case 3: xxxxx|xxxxxx|xxxxx. The free space surrounds the kernel.
             * For this, we split the region into 2 regions, and set the
             * first region end to the start of the kernel, and the start
             * of the second region to the end of the kernel.
             *
             * Case 4: |--xxxx--|. The free space is in the kernel. In this
             * case, we just delete the region :)
             */
            if (region->addr < kernel_start &&
                region->addr + region->size <= kernel_end) {
                // Case 1
                size_t overlap = (region->addr + region->size) - kernel_start;
                region->size -= overlap;
            } else if (region->addr >= kernel_start) {
                // Case 2
                region->addr = kernel_end;
            } else if (region->addr < kernel_start &&
                       region->addr + region->size >= kernel_end) {
                // Case 3
                size_t orig = region->size;
                size_t overlap = (region->addr + region->size) - kernel_start;
                region->size -= overlap;
                fixup_regions[free_fixup_region].addr = kernel_end;
                fixup_regions[free_fixup_region].size =
                    (region->addr + orig) -
                    fixup_regions[free_fixup_region].addr;
                fixup_regions[free_fixup_region].type = MEMORY_TYPE_AVAILABLE;
                fixup_regions[free_fixup_region].next = region->next;
                region->next = &fixup_regions[free_fixup_region];
                free_fixup_region++;
                bootinfo->num_memregions++;
            } else if (region->addr >= kernel_start &&
                       region->addr + region->size <= kernel_end) {
                // Case 4
                if (region->prev)
                    region->prev->next = region->next;
                if (region->next)
                    region->next->prev = region->prev;
            }
        }
    }
}

void arch_mm_init(struct mint_bootinfo *bootinfo,
                  struct memory_context *context)
{
    x86_64_patch_pml4(context);
    physical_init(bootinfo->highest_mem, DMA_MAX);
    x86_64_fix_multiboot(bootinfo);
    for (struct mint_memory_region *region = bootinfo->memregions; region;
         region = region->next) {
        printk(INFO, "  Start: %p. Size: %p. Type: 0x%X\n", region->addr,
               region->size, region->type);
        if (region->type == MEMORY_TYPE_AVAILABLE) {
            physical_free_region(region->addr, region->size);
        }
    }
}
