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

#include <arch/boot/multiboot.h>
#include <arch/cpu/cpu.h>
#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/elf.h>
#include <kernel/init.h>
#include <kernel/symbol.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <string.h>
#include <types.h>

extern addr_t __kernel_start;
extern addr_t __kernel_end;

extern int x86_64_init_console(void);
extern void kmain(struct mint_bootinfo *);

extern void gdt_init(void);
extern void idt_init(void);

static struct mint_bootinfo bootinfo;

extern uint64_t __bss_start;
extern uint64_t __bss_end;

extern uint64_t __kernel_end;

static void multiboot_parse_symbols(
    struct multiboot_elf_section_header_table *table)
{
    printk(INFO, "%u ELF table entries to load, size %u, address %p\n",
           table->num, table->size, table->addr);
    for (uint32_t i = 0; i < table->num; i++) {
        struct elf64_shdr *shdr =
            (struct elf64_shdr *)(table->addr + (table->size * i));
        if (shdr->sh_type == SHT_SYMTAB) {
            struct elf64_sym *symtab = (struct elf64_sym *)shdr->sh_addr;
            if (!symtab)
                continue;
            int num_syms = shdr->sh_size / shdr->sh_entsize;
            struct elf64_shdr *string_table_header =
                (struct elf64_shdr *)((uint64_t)table->addr +
                                      shdr->sh_link * table->size);
            char *string_table = (char *)string_table_header->sh_addr;
            for (int j = 1; j <= num_syms; j++) {
                symtab++;
                if (ELF32_ST_TYPE(symtab->st_info) != STT_FUNC)
                    continue;
                char *name;
                if (symtab->st_name != 0) {
                    name = (char *)&string_table[symtab->st_name];
                } else {
                    name = "N/A";
                }
                ksymbol_register(symtab->st_value, name, strlen(name));
            }
        }
    }
}

static int multiboot_has_symbols(struct multiboot_info *mboot)
{
    return (mboot->flags & MULTIBOOT_INFO_ELF_SHDR) ? 1 : 0;
}

/*
 * Multiboot's memory region comes mainly from BIOS calls, and it doesn't
 * do anything to mark kernel areas as used. Thus, when we blindly pass
 * in free regions to the physical manager, the physical manager happily
 * marks the kernel memory as free to. Eventually, the kernel will get
 * clobbered once that memory gets handed out to some unsuspecting
 * function requesting memory. This function loops through the memory
 * regions and fixes any overlap between the free regions and the kernel
 * space.
 */
static void x86_64_fix_multiboot(struct multiboot_info *mboot)
{
    addr_t kernel_start = ROUND_DOWN((addr_t)&__kernel_start, PAGE_SIZE);
    /*
     * Yes, this kmalloc(0) is intentional. It's a pretty cool trick to get
     * the current location of the heap without actually allocating stuff.
     * Of course, this works only because malloc (early_malloc at this stage)
     * is a simple watermark allocator. Something more advanced may not
     * work with this trick
     */
    addr_t kernel_end = ROUND_UP((addr_t)&__kernel_end, PAGE_SIZE);
    printk(INFO, "Kernel between %p and %p\n", kernel_start, kernel_end);
    uint32_t mmap = mboot->mmap_addr;
    while (mmap < mboot->mmap_addr + mboot->mmap_length) {
        multiboot_memory_map_t *tmp = (multiboot_memory_map_t *)(uint64_t)mmap;
        if (tmp->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (tmp->addr <= kernel_start &&
                tmp->addr + tmp->len >= kernel_end) {
                printk(
                    WARNING,
                    "Kernel located in reported 'free' region, fixing up...\n");
                /*
                 * There are four possible cases in regards to kernel and the
                 * region. |--| denotes the kernel, and xxx denotes free space.
                 *
                 * Case 1: xxxxx|xx---|. The free space is before the kernel and
                 * ends before or equal to the kernel end. We only need to
                 * reduce the existing size to the beginning of the kernel.
                 *
                 * Case 2: |----xxx|xxxxx. The free space is after the kernel
                 * and starts in the kernel. We only need to move the start of
                 * the region to the end of the kernel, maybe adding some buffer
                 * space.
                 *
                 * Case 3: xxxxx|xxxxxx|xxxxx. The free space surrounds the
                 * kernel. For this, we split the region into 2 regions, and set
                 * the first region end to the start of the kernel, and the
                 * start of the second region to the end of the kernel.
                 *
                 * Case 4: |--xxxx--|. The free space is in the kernel. In this
                 * case, we just delete the region :)
                 */
                if (tmp->addr < kernel_start &&
                    tmp->addr + tmp->len <= kernel_end) {
                    // Case 1
                    size_t overlap = (tmp->addr + tmp->len) - kernel_start;
                    tmp->len -= overlap;
                } else if (tmp->addr >= kernel_start &&
                           tmp->addr <= kernel_end &&
                           tmp->addr + tmp->len > kernel_end) {
                    size_t overlap = (kernel_end - tmp->addr);
                    // Case 2
                    tmp->addr = kernel_end;
                    tmp->len -= overlap;
                } else if (tmp->addr < kernel_start &&
                           tmp->addr + tmp->len >= kernel_end) {
                    panic("Free memory region located completely inside "
                          "kernel, unable to handle!\n");
                    // // Case 3
                    // size_t orig = tmp->len;
                    // size_t overlap = (tmp->addr + tmp->len) - kernel_start;
                    // tmp->len -= overlap;
                    // fixup_regions[free_fixup_region].addr = kernel_end;
                    // fixup_regions[free_fixup_region].len =
                    //     (tmp->addr + orig) -
                    //     fixup_regions[free_fixup_region].addr;
                    // fixup_regions[free_fixup_region].type =
                    //     MEMORY_TYPE_AVAILABLE;
                    // fixup_regions[free_fixup_region].next = tmp->next;
                    // tmp->next = &fixup_regions[free_fixup_region];
                    // free_fixup_region++;
                    // bootinfo->num_memregions++;
                } else if (tmp->addr >= kernel_start &&
                           tmp->addr + tmp->len <= kernel_end) {
                    tmp->type = MULTIBOOT_MEMORY_RESERVED;
                }
            }
        }
        mmap += (tmp->size + sizeof(tmp->size));
    }
}

static struct multiboot_mmap_entry *multiboot_get_largest_available_block(
    struct multiboot_info *mboot)
{
    size_t largest = 0;
    struct multiboot_mmap_entry *entry = NULL;
    uint32_t mmap = mboot->mmap_addr;
    while (mmap < mboot->mmap_addr + mboot->mmap_length) {
        multiboot_memory_map_t *tmp = (multiboot_memory_map_t *)(uint64_t)mmap;
        if (tmp->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (tmp->len > largest) {
                largest = tmp->len;
                entry = tmp;
            }
        }
        mmap += (tmp->size + sizeof(tmp->size));
    }
    return entry;
}

static void x86_64_protect_symtab(
    struct multiboot_mmap_entry *entry,
    struct multiboot_elf_section_header_table *table)
{
    size_t symtab_size = 0;
    size_t string_table_size = 0;
    for (uint32_t i = 0; i < table->num; i++) {
        struct elf64_shdr *shdr =
            (struct elf64_shdr *)(table->addr + (table->size * i));
        if (shdr->sh_type == SHT_SYMTAB) {
            symtab_size = shdr->sh_size;
            struct elf64_shdr *string_table_header =
                (struct elf64_shdr *)((uint64_t)table->addr +
                                      shdr->sh_link * table->size);
            string_table_size = string_table_header->sh_size;
        }
    }
    size_t total = ROUND_UP(symtab_size + string_table_size, 0x1000);
    entry->addr += total;
    entry->len -= total;
}

void x86_64_init(uint32_t magic, struct multiboot_info *mboot)
{
    interrupt_disable();
    x86_64_init_console();
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Bad multiboot magic! Expected 0x%X, but got 0x%X\n",
              MULTIBOOT_BOOTLOADER_MAGIC, magic);
    printk(INFO, "x86_64-init: Multiboot information structure at 0x%llX\n",
           mboot);
    cpu_initialize_information();
    cpu_print_information(cpu_get_information(0));
    gdt_init();
    idt_init();
    x86_64_fix_multiboot(mboot);
    struct multiboot_mmap_entry *kmalloc_free =
        multiboot_get_largest_available_block(mboot);
    x86_64_protect_symtab(kmalloc_free, &mboot->u.elf_sec);
    /*
     * Tell early_malloc where it can allocate memory from and the
     * extent that it can allocate to
     */
    early_malloc_set_properties((uint64_t)kmalloc_free->addr + VMA_BASE,
                                kmalloc_free->len);
    if (multiboot_has_symbols(mboot)) {
        printk(INFO, "ELF symbols provided by multiboot, :)\n");
        multiboot_parse_symbols(&mboot->u.elf_sec);
    } else {
        printk(WARNING, "ELF symbols not loaded, stack traces will NOT have "
                        "resolved symbols\n");
    }
    memset(&bootinfo, 0, sizeof(struct mint_bootinfo));
    uint32_t mmap = mboot->mmap_addr;
    while (mmap < mboot->mmap_addr + mboot->mmap_length) {
        multiboot_memory_map_t *tmp = (multiboot_memory_map_t *)(uint64_t)mmap;
        if (tmp->type == 1) {
            /*
             * We manually calculate total_mem because upper_mem from
             * multiboot only goes up to the first memory hole at around
             * 3GB. This could be a QEMU bug though...
             */
            bootinfo.total_mem += tmp->len;
            if ((tmp->addr + tmp->len) > bootinfo.highest_mem)
                bootinfo.highest_mem = tmp->addr + tmp->len;
        }
        struct mint_memory_region *memregion = bootinfo.memregions;
        if (memregion) {
            while (memregion->next)
                memregion = memregion->next;
            memregion->next = kzalloc(sizeof(struct mint_memory_region));
            memregion->next->prev = memregion;
            memregion = memregion->next;
        } else {
            bootinfo.memregions = kzalloc(sizeof(struct mint_memory_region));
            memregion = bootinfo.memregions;
        }
        memregion->addr = tmp->addr;
        memregion->size = tmp->len;
        memregion->type = tmp->type;
        bootinfo.num_memregions++;
        mmap += (tmp->size + sizeof(tmp->size));
    }
    kmain(&bootinfo);
}
