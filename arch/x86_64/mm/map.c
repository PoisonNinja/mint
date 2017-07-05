#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <mm/physical.h>
#include <mm/virtual.h>

void arch_virtual_map(struct memory_context* context, addr_t virtual,
                      addr_t physical, uint8_t flags)
{
    struct page_table* pml4 =
        (struct page_table*)(context->page_table + PHYS_START);
    struct page_table* pdpt = NULL;
    struct page_table* pd = NULL;
    struct page_table* pt = NULL;
    if (!pml4->pages[PML4_INDEX(virtual)].present) {
        pdpt = (struct page_table*)physical_alloc(0x1000, 0);
        pml4->pages[PML4_INDEX(virtual)].present = 1;
        pml4->pages[PML4_INDEX(virtual)].writable =
            (flags & PAGE_WRITABLE) ? 1 : 0;
        pml4->pages[PML4_INDEX(virtual)].user = (flags & PAGE_USER) ? 1 : 0;
        pml4->pages[PML4_INDEX(virtual)].global = (flags & PAGE_GLOBAL) ? 1 : 0;
        pml4->pages[PML4_INDEX(virtual)].nx = (flags & PAGE_NX) ? 1 : 0;
        pml4->pages[PML4_INDEX(virtual)].address = (addr_t)pdpt / 0x1000;
    } else {
        pdpt = (struct page_table*)(pml4->pages[PML4_INDEX(virtual)].address *
                                    0x1000);
    }
    pdpt = (struct page_table*)((addr_t)pdpt + PHYS_START);
    if (!pdpt->pages[PDPT_INDEX(virtual)].present) {
        pd = (struct page_table*)physical_alloc(0x1000, 0);
        pdpt->pages[PDPT_INDEX(virtual)].present = 1;
        pdpt->pages[PDPT_INDEX(virtual)].writable =
            (flags & PAGE_WRITABLE) ? 1 : 0;
        pdpt->pages[PDPT_INDEX(virtual)].user = (flags & PAGE_USER) ? 1 : 0;
        pdpt->pages[PDPT_INDEX(virtual)].global = (flags & PAGE_GLOBAL) ? 1 : 0;
        pdpt->pages[PDPT_INDEX(virtual)].nx = (flags & PAGE_NX) ? 1 : 0;
        pdpt->pages[PDPT_INDEX(virtual)].address = (addr_t)pd / 0x1000;
    } else {
        pd = (struct page_table*)(pdpt->pages[PDPT_INDEX(virtual)].address *
                                  0x1000);
    }
    pd = (struct page_table*)((addr_t)pd + PHYS_START);
    if (!(flags & PAGE_HUGE)) {
        if (!pd->pages[PD_INDEX(virtual)].present) {
            pt = (struct page_table*)physical_alloc(0x1000, 0);
            pd->pages[PD_INDEX(virtual)].present = 1;
            pd->pages[PD_INDEX(virtual)].writable =
                (flags & PAGE_WRITABLE) ? 1 : 0;
            pd->pages[PD_INDEX(virtual)].user = (flags & PAGE_USER) ? 1 : 0;
            pd->pages[PD_INDEX(virtual)].global = (flags & PAGE_GLOBAL) ? 1 : 0;
            pd->pages[PD_INDEX(virtual)].nx = (flags & PAGE_NX) ? 1 : 0;
            pd->pages[PD_INDEX(virtual)].address = (addr_t)pt / 0x1000;
        } else {
            pt = (struct page_table*)(pd->pages[PD_INDEX(virtual)].address *
                                      0x1000);
        }
        pt = (struct page_table*)((addr_t)pt + PHYS_START);
        if (!pt->pages[PT_INDEX(virtual)].present) {
            pt->pages[PT_INDEX(virtual)].present = 1;
            pt->pages[PT_INDEX(virtual)].writable =
                (flags & PAGE_WRITABLE) ? 1 : 0;
            pt->pages[PT_INDEX(virtual)].user = (flags & PAGE_USER) ? 1 : 0;
            pt->pages[PT_INDEX(virtual)].global = (flags & PAGE_GLOBAL) ? 1 : 0;
            pt->pages[PT_INDEX(virtual)].nx = (flags & PAGE_NX) ? 1 : 0;
            pt->pages[PT_INDEX(virtual)].address = physical / 0x1000;
        }
    } else {
        pd->pages[PD_INDEX(virtual)].present = 1;
        pd->pages[PD_INDEX(virtual)].writable = (flags & PAGE_WRITABLE) ? 1 : 0;
        pd->pages[PD_INDEX(virtual)].user = (flags & PAGE_USER) ? 1 : 0;
        pd->pages[PD_INDEX(virtual)].global = (flags & PAGE_GLOBAL) ? 1 : 0;
        pd->pages[PD_INDEX(virtual)].nx = (flags & PAGE_NX) ? 1 : 0;
        pd->pages[PD_INDEX(virtual)].huge_page = (flags & PAGE_HUGE) ? 1 : 0;
        pd->pages[PD_INDEX(virtual)].address = physical / 0x1000;
    }
}
