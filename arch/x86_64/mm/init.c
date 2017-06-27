#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <kernel.h>
#include <string.h>

__attribute__((aligned(0x1000))) static struct page_table pml3;
__attribute__((aligned(0x1000))) static struct page_table pml2[512];

/*
 * Intended for initial bringup. Based on seakernel initial mm bringup, but
 * rewritten to use structs instead of uint64_t. This uses large pages to
 * make mapping easier while the physical memory manager is offline since
 * we can't allocate new pages yet, only using preallocated/compiled in
 */

void x86_64_patch_pml4(void)
{
    struct page_table *pml4 = (struct page_table *)(read_cr3() + KERNEL_START);
    // pml4->pages[0].address = 0;
    // pml4->pages[0].present = 0;
    addr_t address = 0;
    for (int i = 0; i < 512; i++) {
        pml3.pages[i].address = ((addr_t)&pml2[i] - KERNEL_START) / 0x1000;
        pml3.pages[i].present = 1;
        pml3.pages[i].writable = 1;
        for (int j = 0; j < 512; j++) {
            pml2[i].pages[j].address = address / 0x1000;
            pml2[i].pages[j].present = 1;
            pml2[i].pages[j].writable = 1;
            pml2[i].pages[j].huge_page = 1;
            address += 0x200000;
        }
    }
    pml4->pages[PML4_INDEX(PHYS_START)].address =
        ((addr_t)&pml3 - KERNEL_START) / 0x1000;
    pml4->pages[PML4_INDEX(PHYS_START)].present = 1;
    pml4->pages[PML4_INDEX(PHYS_START)].writable = 1;
    printk(INFO, "Done patching PML4\n");
}

void arch_mm_init(void)
{
    x86_64_patch_pml4();
}
