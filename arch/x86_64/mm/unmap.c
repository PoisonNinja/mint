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
#include <mm/physical.h>
#include <mm/virtual.h>
#include <string.h>

#include <kernel.h>

static inline int __virtual_table_is_empty(struct page_table* table)
{
    for (int i = 0; i < 512; i++) {
        if (table->pages[i].present)
            return 0;
    }
    return 1;
}

void arch_virtual_unmap(struct memory_context* context, addr_t virtual)
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
    // Flush the TLB entries by writing to CR3
    write_cr3(kernel_context.physical_base);
    struct page_table* pml4 = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    struct page_table* pdpt = entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(virtual));
    struct page_table* pd =
        entry_to_address(RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(virtual),
                         PDPT_INDEX(virtual));
    struct page_table* pt =
        entry_to_address(RECURSIVE_ENTRY, PML4_INDEX(virtual),
                         PDPT_INDEX(virtual), PD_INDEX(virtual));
    invlpg(pml4);
    invlpg(pdpt);
    invlpg(pd);
    invlpg(pt);
    if (!pml4->pages[PML4_INDEX(virtual)].present)
        return;
    if (!pdpt->pages[PDPT_INDEX(virtual)].present)
        return;
    if (!pd->pages[PD_INDEX(virtual)].present)
        return;
    if (!pt->pages[PT_INDEX(virtual)].present)
        return;
    memset(&pt->pages[PT_INDEX(virtual)], 0, sizeof(struct page));
    if (__virtual_table_is_empty(pt)) {
        physical_free((void*)(pd->pages[PD_INDEX(virtual)].address * 0x1000),
                      0x1000);
        memset(&pd->pages[PD_INDEX(virtual)], 0, sizeof(struct page));
    }
    if (__virtual_table_is_empty(pd)) {
        physical_free(
            (void*)(pdpt->pages[PDPT_INDEX(virtual)].address * 0x1000), 0x1000);
        memset(&pdpt->pages[PDPT_INDEX(virtual)], 0, sizeof(struct page));
    }
    if (__virtual_table_is_empty(pdpt)) {
        physical_free(
            (void*)(pml4->pages[PML4_INDEX(virtual)].address * 0x1000), 0x1000);
        memset(&pml4->pages[PML4_INDEX(virtual)], 0, sizeof(struct page));
    }
    // Restore the original fractal mapping
    current->pages[RECURSIVE_ENTRY].address = original_fractal;
    invlpg(pml4);
    invlpg(pdpt);
    invlpg(pd);
    invlpg(pt);
}
