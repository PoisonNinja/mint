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

#include <kernel.h>
#include <types.h>

void arch_stacktrace(void)
{
    uint64_t* rbp = NULL;
    __asm__("mov %%rbp, %0" : "=r"(rbp));
    while (rbp) {
        uint64_t rip = rbp[1];
        if (!rip)
            break;
        rbp = (uint64_t*)rbp[0];
        printk(INFO, "<%llX> %s\n", rip, "???????????");
    }
}
