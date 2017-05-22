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
#include <kernel.h>
#include <types.h>

extern int x86_64_init_console(void);

void x86_64_init(uint32_t magic, struct multiboot_info *mboot)
{
    x86_64_init_console();
    for (int i = 0; i < 26; i++) {
        printk(INFO, "Hello world: %d!\n", i);
    }
    for (;;)
        ;
}
