/*
 * Copyright (C) 2017 Jason Lu (PoisonNinja) and contributors
 *
 * This file is part of Strawberry. Strawberry is free software: you can
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

#include <kernel/lock/spinlock.h>

void spinlock_lock(spinlock_t p)
{
    while (!__sync_bool_compare_and_swap(p, 0, 1))
        ;
}

void spinlock_unlock(spinlock_t p)
{
    asm volatile("");  // acts as a memory barrier.
    *p = 0;
}
