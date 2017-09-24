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
#include <arch/mm/mmap.h>
#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/elf.h>
#include <kernel/init.h>
#include <kernel/symbol.h>
#include <mm/heap.h>
#include <string.h>
#include <types.h>

extern int x86_64_init_console(void);
extern void kmain(struct mint_bootinfo *);

extern void gdt_init(void);
extern void idt_init(void);

static struct mint_bootinfo bootinfo;

extern uint64_t __bss_start;
extern uint64_t __bss_end;

extern uint64_t __kernel_end;

void multiboot_parse_symbols(struct multiboot_elf_section_header_table *table)
{
    printk(INFO, "%u ELF table entries to load, size %u, address %p\n",
           table->num, table->size, table->addr);
    for (int i = 0; i < table->num; i++) {
        struct elf64_shdr *shdr = table->addr + (table->size * i);
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

int multiboot_has_symbols(struct multiboot_info *mboot)
{
    return (mboot->flags & MULTIBOOT_INFO_ELF_SHDR) ? 1 : 0;
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
    /*
     * Tell early_malloc where it can allocate memory from and the extent
     * that it can allocate to
     */
    early_malloc_set_properties((uint64_t)&__kernel_end + VMA_BASE, 0x100000);
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
