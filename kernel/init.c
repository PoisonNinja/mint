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
#include <kernel/init.h>
#include <types.h>

extern uint64_t __initcall1_start;
extern uint64_t __initcall2_start;
extern uint64_t __initcall3_start;
extern uint64_t __initcall4_start;
extern uint64_t __initcall5_start;
extern uint64_t __initcall6_start;
extern uint64_t __initcall7_start;
extern uint64_t __initcall_end;

static uint64_t *initcall_levels[8] = {
    &__initcall1_start, &__initcall2_start, &__initcall3_start,
    &__initcall4_start, &__initcall5_start, &__initcall6_start,
    &__initcall7_start, &__initcall_end,
};

void do_initcall(int level)
{
    printk(INFO, "Calling initcalls for level %d\n", level);
    for (uint64_t *i = initcall_levels[level - 1]; i < initcall_levels[level];
         i++) {
        initcall_t fn = *(initcall_t *)i;
        fn();
    }
}
