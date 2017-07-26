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

void __copy_pd_entry(struct page_table* new_pd, struct page_table* old_pd)
{
    for (int i = 0; i < 512; i++) {
        if (old_pd->pages[i].present) {
            memcpy(&new_pd->pages[i], &old_pd->pages[i], sizeof(struct page));
            if (!old_pd->pages[i].huge_page) {
                struct page_table* new_pt = physical_alloc(0x1000, 0);
                new_pd->pages[i].address = (addr_t)new_pt / 0x1000;
                __copy_pt_entry(
                    new_pt,
                    (struct page_table*)(old_pd->pages[i].address * 0x1000));
            }
        }
    }
}

void __copy_pdpt_entry(struct page_table* new_pdpt, struct page_table* old_pdpt)
{
    for (int i = 0; i < 512; i++) {
        if (old_pdpt->pages[i].present) {
            struct page_table* new_pd = physical_alloc(0x1000, 0);
            memcpy(&new_pdpt->pages[i], &old_pdpt->pages[i],
                   sizeof(struct page));
            new_pdpt->pages[i].address = (addr_t)new_pd / 0x1000;
            __copy_pd_entry(
                new_pd,
                (struct page_table*)(old_pdpt->pages[i].address * 0x1000));
        }
    }
}

void arch_virtual_clone(struct memory_context* original,
                        struct memory_context* new)
{
    struct page_table* pml4 =
        (struct page_table*)((addr_t)physical_alloc(0x1000, 0) + PHYS_START);
    memset(pml4, 0, sizeof(struct page_table));
    struct page_table* old_pml4 =
        (struct page_table*)((addr_t)original->page_table + PHYS_START);
    memcpy(pml4, old_pml4, sizeof(struct page_table));
    for (int i = 0; i < 512; i++) {
        if (i >= (int)PML4_INDEX(HIGHER_HALF)) {
            // We are in something kernel related
            memcpy(&pml4->pages[i], &old_pml4->pages[i], sizeof(struct page));
        } else {
            if (old_pml4->pages[i].present) {
                struct page_table* new_pdpt = physical_alloc(0x1000, 0);
                memcpy(&pml4->pages[i], &old_pml4->pages[i],
                       sizeof(struct page));
                pml4->pages[i].address = (addr_t)new_pdpt / 0x1000;
                __copy_pdpt_entry(
                    new_pdpt,
                    (struct page_table*)(old_pml4->pages[i].address * 0x1000));
            }
        }
    }
    new->page_table = (addr_t)pml4 - PHYS_START;
}
