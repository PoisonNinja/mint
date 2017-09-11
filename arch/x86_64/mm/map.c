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
#include <kernel.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <string.h>

static inline int __virtual_set_address(struct page* page)
{
    if (!page->present) {
        addr_t addr = (addr_t)physical_alloc(0x1000, 0);
        page->address = addr / 0x1000;
        return 1;
    }
    return 0;
}

static inline void __virtual_set_flags(struct page* page, uint8_t flags)
{
    page->present = 1;
    page->writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page->user = (flags & PAGE_USER) ? 1 : 0;
    page->global = (flags & PAGE_GLOBAL) ? 1 : 0;
    page->nx = (flags & PAGE_NX) ? 1 : 0;
}

void arch_virtual_map(struct memory_context* context, addr_t virtual,
                      addr_t physical, uint8_t flags)
{
    addr_t original_fractal;
    // TODO: Get current process memory_context
    struct page_table* current =
        (struct page_table*)kernel_context.virtual_base;
    /*
     * Back up original fractal mapping to restore at the end of this call.
     *
     * This is required when virtual_map ends up making a call to
     * physical_alloc to allocate a page frame. When that happens, we
     * end up using whatever physical_alloc maps into this slot, which
     * ends up really badly, because we'll end up mapping into the wrong
     * address space.
     */
    original_fractal = current->pages[RECURSIVE_ENTRY].address;
    // Set up fractal mapping for the new context
    current->pages[RECURSIVE_ENTRY].address = context->physical_base / 0x1000;
    struct page_table* pml4 = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    struct page_table* pdpt = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(virtual));
    struct page_table* pd = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(virtual),
        PDPT_INDEX(virtual));
    struct page_table* pt = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, PML4_INDEX(virtual), PDPT_INDEX(virtual),
        PD_INDEX(virtual));
    invlpg(pml4);
    invlpg(pdpt);
    invlpg(pd);
    invlpg(pt);
    int r = 0;
    r = __virtual_set_address(&pml4->pages[PML4_INDEX(virtual)]);
    __virtual_set_flags(&pml4->pages[PML4_INDEX(virtual)], flags);
    /*
     * __virtual_set_address returns whether it allocated memory or not.
     * If it did, we need to memset it ourselves to 0.
     */
    if (r) {
        memset(pdpt, 0, sizeof(struct page_table));
    }
    r = __virtual_set_address(&pdpt->pages[PDPT_INDEX(virtual)]);
    __virtual_set_flags(&pdpt->pages[PDPT_INDEX(virtual)], flags);
    if (r) {
        memset(pd, 0, sizeof(struct page_table));
    }
    r = __virtual_set_address(&pd->pages[PD_INDEX(virtual)]);
    __virtual_set_flags(&pd->pages[PD_INDEX(virtual)], flags);
    if (r) {
        memset(pt, 0, sizeof(struct page_table));
    }
    __virtual_set_flags(&pt->pages[PT_INDEX(virtual)], flags);
    pt->pages[PT_INDEX(virtual)].address = physical / 0x1000;
    // Restore the original fractal mapping
    current->pages[RECURSIVE_ENTRY].address = original_fractal;
    invlpg(pml4);
    invlpg(pdpt);
    invlpg(pd);
    invlpg(pt);
}
