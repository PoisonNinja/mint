#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <mm/physical.h>
#include <mm/virtual.h>

static inline void* __virtual_get_address(struct page* page)
{
    if (!page->present) {
        return physical_alloc(0x1000, 0);
    } else {
        return (void*)(page->address * 0x1000);
    }
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
    struct page_table* pml4 =
        (struct page_table*)(context->page_table + PHYS_START);
    struct page_table* pdpt = NULL;
    struct page_table* pd = NULL;
    struct page_table* pt = NULL;
    pdpt = __virtual_get_address(&pml4->pages[PML4_INDEX(virtual)]);
    __virtual_set_flags(&pml4->pages[PML4_INDEX(virtual)], flags);
    pml4->pages[PML4_INDEX(virtual)].address = (addr_t)pdpt / 0x1000;
    pdpt = (struct page_table*)((addr_t)pdpt + PHYS_START);
    pd = __virtual_get_address(&pdpt->pages[PDPT_INDEX(virtual)]);
    __virtual_set_flags(&pdpt->pages[PDPT_INDEX(virtual)], flags);
    pdpt->pages[PDPT_INDEX(virtual)].address = (addr_t)pd / 0x1000;
    pd = (struct page_table*)((addr_t)pd + PHYS_START);
    if (!(flags & PAGE_HUGE)) {
        pt = __virtual_get_address(&pd->pages[PD_INDEX(virtual)]);
        __virtual_set_flags(&pd->pages[PD_INDEX(virtual)], flags);
        pd->pages[PD_INDEX(virtual)].address = (addr_t)pt / 0x1000;
        pt = (struct page_table*)((addr_t)pt + PHYS_START);
        __virtual_set_flags(&pt->pages[PT_INDEX(virtual)], flags);
        pt->pages[PT_INDEX(virtual)].address = physical / 0x1000;
    } else {
        __virtual_set_flags(&pd->pages[PD_INDEX(virtual)], flags);
        pd->pages[PD_INDEX(virtual)].huge_page = 1;
        pd->pages[PD_INDEX(virtual)].address = physical / 0x1000;
    }
}
