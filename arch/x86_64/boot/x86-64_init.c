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
#include <boot/bootinfo.h>
#include <kernel.h>
#include <string.h>
#include <types.h>

extern int x86_64_init_console(void);
extern void kmain(struct mint_bootinfo *);

extern void gdt_init(void);
extern void idt_init(void);

extern uint64_t __start_bss;
extern uint64_t __stop_bss;

void zero_bss(void)
{
    memset(&__start_bss, 0, &__stop_bss - &__start_bss);
}

void x86_64_init(uint32_t magic, struct multiboot_info *mboot)
{
    x86_64_init_console();
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        printk(INFO, "WTF magic mismatch!\n");
    gdt_init();
    idt_init();
    kmain(NULL);
}
