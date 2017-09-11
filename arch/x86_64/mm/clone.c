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
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <string.h>

void __copy_pt_entry(struct page_table* new_pt, struct page_table* old_pt)
{
    for (int i = 0; i < 512; i++) {
        if (old_pt->pages[i].present) {
            memcpy(&new_pt->pages[i], &old_pt->pages[i], sizeof(struct page));
        }
    }
}

void __copy_pd_entry(struct page_table* new_pd, struct page_table* old_pd,
                     int pml4_index, int pdpt_index)
{
    for (int i = 0; i < 512; i++) {
        if (old_pd->pages[i].present) {
            struct page_table* new_pt = physical_alloc(0x1000, 0);
            memcpy(&new_pd->pages[i], &old_pd->pages[i], sizeof(struct page));
            new_pd->pages[i].address = (addr_t)new_pt / 0x1000;
            __copy_pt_entry((struct page_table*)entry_to_address(
                                COPY_ENTRY, pml4_index, pdpt_index, i),
                            (struct page_table*)entry_to_address(
                                RECURSIVE_ENTRY, pml4_index, pdpt_index, i));
        }
    }
}

void __copy_pdpt_entry(struct page_table* new_pdpt, struct page_table* old_pdpt,
                       int pml4_index)
{
    for (int i = 0; i < 512; i++) {
        if (old_pdpt->pages[i].present) {
            struct page_table* new_pd = physical_alloc(0x1000, 0);
            memcpy(&new_pdpt->pages[i], &old_pdpt->pages[i],
                   sizeof(struct page));
            new_pdpt->pages[i].address = (addr_t)new_pd / 0x1000;
            __copy_pd_entry(
                (struct page_table*)entry_to_address(COPY_ENTRY, COPY_ENTRY,
                                                     pml4_index, i),
                (struct page_table*)entry_to_address(
                    RECURSIVE_ENTRY, RECURSIVE_ENTRY, pml4_index, i),
                pml4_index, i);
        }
    }
}

void arch_virtual_clone(struct memory_context* original,
                        struct memory_context* new)
{
    addr_t original_fractal_recursive, original_fractal_copy;
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
    original_fractal_recursive = current->pages[RECURSIVE_ENTRY].address;
    original_fractal_copy = current->pages[COPY_ENTRY].address;
    // Set up fractal mapping for the new context
    current->pages[RECURSIVE_ENTRY].address = original->physical_base / 0x1000;
    current->pages[COPY_ENTRY].address = new->physical_base / 0x1000;
    // Flush the TLB entries by writing to CR3
    write_cr3(kernel_context.physical_base);
    struct page_table* new_pml4 = (struct page_table*)entry_to_address(
        COPY_ENTRY, COPY_ENTRY, COPY_ENTRY, COPY_ENTRY);
    memset(new_pml4, 0, sizeof(struct page_table));
    struct page_table* old_pml4 = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    memcpy(new_pml4, old_pml4, sizeof(struct page_table));
    for (int i = 0; i < 512; i++) {
        if (i >= (int)PML4_INDEX(HIGHER_HALF)) {
            // We are in something kernel related
            memcpy(&new_pml4->pages[i], &old_pml4->pages[i],
                   sizeof(struct page));
        } else {
            if (old_pml4->pages[i].present) {
                struct page_table* new_pdpt = physical_alloc(0x1000, 0);
                memcpy(&new_pml4->pages[i], &old_pml4->pages[i],
                       sizeof(struct page));
                new_pml4->pages[i].address = (addr_t)new_pdpt / 0x1000;
                __copy_pdpt_entry(
                    (struct page_table*)entry_to_address(COPY_ENTRY, COPY_ENTRY,
                                                         COPY_ENTRY, i),
                    (struct page_table*)entry_to_address(
                        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, i),
                    i);
            }
        }
    }
}
