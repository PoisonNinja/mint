/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <boot/bootinfo.h>
#include <cpu/exception.h>
#include <kernel.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <string.h>

extern addr_t __kernel_start;
extern addr_t __kernel_end;

static struct mint_memory_region fixup_regions[10];
static uint8_t free_fixup_region = 0;

static char *memory_type_strings[] = {
    "Unknown",
    "Available",
    "Reserved",
};

/*
 * Intended for initial bringup. Based on seakernel initial mm bringup, but
 * rewritten to use structs instead of uint64_t. This uses large pages to
 * make mapping easier while the physical memory manager is offline since
 * we can't allocate new pages yet, only using preallocated/compiled in
 */
static void x86_64_patch_pml4(struct memory_context *context)
{
    struct page_table *pml4 = (struct page_table *)(read_cr3() + VMA_BASE);
    pml4->pages[RECURSIVE_ENTRY].present = 1;
    pml4->pages[RECURSIVE_ENTRY].writable = 1;
    pml4->pages[RECURSIVE_ENTRY].address = ((addr_t)pml4 - VMA_BASE) / 0x1000;
    pml4->pages[0].address = 0;
    pml4->pages[0].present = 0;
    context->page_table = (addr_t)pml4 - VMA_BASE;
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
static void x86_64_fix_multiboot(struct mint_bootinfo *bootinfo)
{
    addr_t kernel_start = ROUND_DOWN((addr_t)&__kernel_start, PAGE_SIZE);
    /*
     * Yes, this kmalloc(0) is intentional. It's a pretty cool trick to get
     * the current location of the heap without actually allocating stuff.
     * Of course, this works only because malloc (early_malloc at this stage)
     * is a simple watermark allocator. Something more advanced may not
     * work with this trick
     */
    addr_t kernel_end = ROUND_UP((addr_t)kmalloc(0) - VMA_BASE, PAGE_SIZE);
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
            } else if (region->addr >= kernel_start &&
                       region->addr <= kernel_end) {
                size_t overlap = (kernel_end - region->addr);
                // Case 2
                region->addr = kernel_end;
                region->size -= overlap;
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

/*
 * Construct the final kernel page table using the newly intialized kernel
 * physical memory manager and virtual memory mapping functions.
 */
static void x86_64_finalize_paging(struct memory_context *context)
{
    struct page_table *pml4 =
        (struct page_table *)(physical_alloc(0x1000, 0) + PHYS_START);
    memset(pml4, 0, sizeof(struct page_table));
    context->page_table = (addr_t)pml4 - PHYS_START;
    virtual_map(context, KERNEL_START, KERNEL_PHYS, KERNEL_END - KERNEL_START,
                PAGE_PRESENT | PAGE_WRITABLE);
    virtual_map(context, VGA_START, VGA_PHYS, VGA_END - VGA_START,
                PAGE_PRESENT | PAGE_WRITABLE);
    write_cr3(context->page_table);
}

extern int arch_virtual_fault(struct interrupt_ctx *, void *);
static struct exception_handler page_fault_handler = {
    .handler = &arch_virtual_fault,
    .dev_name = "page fault",
    .dev_id = &arch_virtual_fault,
};

static void x86_64_install_handler(void)
{
    exception_handler_register(14, &page_fault_handler);
}

void arch_mm_init(struct mint_bootinfo *bootinfo,
                  struct memory_context *context)
{
    x86_64_install_handler();
    x86_64_patch_pml4(context);
    physical_init(bootinfo->highest_mem, DMA_MAX);
    x86_64_fix_multiboot(bootinfo);
    printk(INFO, "%d memory regions:\n", bootinfo->num_memregions);
    for (struct mint_memory_region *region = bootinfo->memregions; region;
         region = region->next) {
        printk(
            INFO, "  [%p - %p] Type: %s\n", region->addr,
            region->addr + region->size,
            (region->type < 3) ? memory_type_strings[region->type] : "Unknown");
        if (region->type == MEMORY_TYPE_AVAILABLE) {
            physical_free_region(region->addr, region->size);
        }
    }
    x86_64_finalize_paging(context);
}
