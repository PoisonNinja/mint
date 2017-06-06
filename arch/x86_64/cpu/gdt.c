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

#include <arch/cpu/tables.h>
#include <kernel.h>
#include <string.h>

#define NUM_ENTRIES 3

static struct gdt_entry gdt_entries[NUM_ENTRIES];
static struct gdt_descriptor gdt_ptr;

extern void gdt_load(uint64_t);

static void gdt_set_entry(struct gdt_entry* entry, uint32_t base,
                          uint32_t limit, uint8_t access, uint8_t flags)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access = access;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->flags = flags & 0xF;
    entry->base_high = (base >> 24) & 0xFF;
}

void gdt_init(void)
{
    memset(&gdt_entries, 0, sizeof(struct gdt_entry) * NUM_ENTRIES);
    memset(&gdt_ptr, 0, sizeof(struct gdt_descriptor));
    gdt_set_entry(&gdt_entries[0], 0, 0, 0, 0);
    gdt_set_entry(&gdt_entries[1], 0, 0xFFFFF, 0x9A, 0x0A);
    gdt_set_entry(&gdt_entries[2], 0, 0xFFFFF, 0x92, 0x0A);
    gdt_ptr.limit = sizeof(struct gdt_entry) * NUM_ENTRIES - 1;
    gdt_ptr.offset = (uint64_t)gdt_entries;
    gdt_load((uint64_t)&gdt_ptr);
}
