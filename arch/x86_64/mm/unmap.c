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
    struct page_table* pml4 =
        (struct page_table*)(context->physical_base + PHYS_START);
    struct page_table* pdpt = NULL;
    struct page_table* pd = NULL;
    struct page_table* pt = NULL;
    struct page* page = NULL;
    pdpt =
        (struct page_table*)(pml4->pages[PML4_INDEX(virtual)].address * 0x1000 +
                             PHYS_START);
    if (!pdpt)
        return;
    pd =
        (struct page_table*)(pdpt->pages[PDPT_INDEX(virtual)].address * 0x1000 +
                             PHYS_START);
    if (!pd)
        return;
    pt = (struct page_table*)(pd->pages[PD_INDEX(virtual)].address * 0x1000 +
                              PHYS_START);
    if (!pt)
        return;
    page = (struct page*)&pt->pages[PT_INDEX(virtual)];
    if (!page)
        return;
    memset(page, 0, sizeof(struct page));
    if (__virtual_table_is_empty(pt)) {
        physical_free((void*)((addr_t)pt - PHYS_START), 0x1000);
        memset(&pd->pages[PD_INDEX(virtual)], 0, sizeof(struct page));
    } else {
        return;
    }
    if (__virtual_table_is_empty(pd)) {
        physical_free((void*)((addr_t)pd - PHYS_START), 0x1000);
        memset(&pdpt->pages[PDPT_INDEX(virtual)], 0, sizeof(struct page));
    } else {
        return;
    }
    if (__virtual_table_is_empty(pdpt)) {
        physical_free((void*)((addr_t)pdpt - PHYS_START), 0x1000);
        memset(&pml4->pages[PML4_INDEX(virtual)], 0, sizeof(struct page));
    } else {
        return;
    }
}
