/*
 * Copyright (C) 2017 Jason Lu (PoisonNinja)
 *
 * This file is part of Mint. Mint is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <arch/boot/multiboot.h>
#include <arch/cpu/cpu.h>
#include <boot/bootinfo.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/init.h>
#include <string.h>
#include <types.h>

extern int x86_64_init_console(void);
extern void kmain(struct mint_bootinfo *);

extern void gdt_init(void);
extern void idt_init(void);

static struct mint_bootinfo bootinfo;

extern uint64_t __bss_start;
extern uint64_t __bss_end;

void zero_bss(void)
{
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}

void x86_64_init(uint32_t magic, struct multiboot_info *mboot)
{
    interrupt_disable();
    zero_bss();
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
        }
        bootinfo.num_memregions++;
        mmap += (tmp->size + sizeof(tmp->size));
    }
    kmain(&bootinfo);
}
