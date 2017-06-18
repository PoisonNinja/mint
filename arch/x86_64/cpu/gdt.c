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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
