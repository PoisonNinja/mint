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

__attribute__((aligned(0x1000))) static struct page_table final_pml4;

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
    context->physical_base = (addr_t)pml4 - VMA_BASE;
    context->virtual_base = (addr_t)pml4;
}

/*
 * Construct the final kernel page table using the newly intialized kernel
 * physical memory manager and virtual memory mapping functions.
 */
static void x86_64_finalize_paging(struct memory_context *context)
{
    memset(&final_pml4, 0, sizeof(struct page_table));
    struct memory_context new_context;
    new_context.physical_base = (addr_t)&final_pml4 - VMA_BASE;
    new_context.virtual_base = (addr_t)&final_pml4;
    final_pml4.pages[RECURSIVE_ENTRY].address =
        new_context.physical_base / 0x1000;
    final_pml4.pages[RECURSIVE_ENTRY].present = 1;
    final_pml4.pages[RECURSIVE_ENTRY].writable = 1;
    virtual_map(&new_context, KERNEL_START, KERNEL_PHYS,
                KERNEL_END - KERNEL_START, PAGE_PRESENT | PAGE_WRITABLE);
    virtual_map(&new_context, VGA_START, VGA_PHYS, VGA_END - VGA_START,
                PAGE_PRESENT | PAGE_WRITABLE);
    context->physical_base = new_context.physical_base;
    context->virtual_base = new_context.virtual_base;
    write_cr3(context->physical_base);
}

static void x86_64_update_map(struct mint_bootinfo *bootinfo)
{
    addr_t heap_end = ROUND_UP((addr_t)kmalloc(0), 0x1000) - VMA_BASE;
    for (struct mint_memory_region *region = bootinfo->memregions; region;
         region = region->next) {
        if (region->addr < heap_end && region->addr + region->size > heap_end &&
            region->type == MEMORY_TYPE_AVAILABLE) {
            printk(INFO, "Updating region to %p\n", heap_end);
            size_t overlap = heap_end - region->addr;
            region->addr = heap_end;
            region->size -= overlap;
        }
    }
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
    x86_64_update_map(bootinfo);
    printk(INFO, "%d memory regions:\n", bootinfo->num_memregions);
    for (struct mint_memory_region *region = bootinfo->memregions; region;
         region = region->next) {
        region->addr = ROUND_UP(region->addr, 0x1000);
        region->size = ROUND_DOWN(region->size, 0x1000);
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
