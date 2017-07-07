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
        (struct page_table*)(context->page_table + PHYS_START);
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
