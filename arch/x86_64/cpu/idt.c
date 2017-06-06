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

#include <arch/cpu/registers.h>
#include <arch/cpu/tables.h>
#include <kernel.h>
#include <string.h>

#define NUM_ENTRIES 256

static struct idt_entry idt_entries[NUM_ENTRIES];
static struct idt_descriptor idt_ptr;

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

static void idt_set_entry(struct idt_entry* entry, uint64_t offset,
                          uint16_t selector, uint8_t attributes)
{
    entry->offset_low = offset & 0xFFFF;
    entry->selector = selector;
    entry->zero_one = 0;
    entry->attributes = attributes;
    entry->offset_middle = (offset >> 16) & 0xFFFF;
    entry->offset_high = (offset >> 32) & 0xFFFFFFFF;
    entry->zero_two = 0;
}

extern int idt_load(uint64_t);

void idt_init(void)
{
    memset(&idt_entries, 0, sizeof(struct idt_entry) * NUM_ENTRIES);
    memset(&idt_ptr, 0, sizeof(struct idt_descriptor));
    idt_set_entry(&idt_entries[0], (uint64_t)isr0, 0x08, 0x8E);
    idt_set_entry(&idt_entries[1], (uint64_t)isr1, 0x08, 0x8E);
    idt_set_entry(&idt_entries[2], (uint64_t)isr2, 0x08, 0x8E);
    idt_set_entry(&idt_entries[3], (uint64_t)isr3, 0x08, 0x8E);
    idt_set_entry(&idt_entries[4], (uint64_t)isr4, 0x08, 0x8E);
    idt_set_entry(&idt_entries[5], (uint64_t)isr5, 0x08, 0x8E);
    idt_set_entry(&idt_entries[6], (uint64_t)isr6, 0x08, 0x8E);
    idt_set_entry(&idt_entries[7], (uint64_t)isr7, 0x08, 0x8E);
    idt_set_entry(&idt_entries[8], (uint64_t)isr8, 0x08, 0x8E);
    idt_set_entry(&idt_entries[9], (uint64_t)isr9, 0x08, 0x8E);
    idt_set_entry(&idt_entries[10], (uint64_t)isr10, 0x08, 0x8E);
    idt_set_entry(&idt_entries[11], (uint64_t)isr11, 0x08, 0x8E);
    idt_set_entry(&idt_entries[12], (uint64_t)isr12, 0x08, 0x8E);
    idt_set_entry(&idt_entries[13], (uint64_t)isr13, 0x08, 0x8E);
    idt_set_entry(&idt_entries[14], (uint64_t)isr14, 0x08, 0x8E);
    idt_set_entry(&idt_entries[15], (uint64_t)isr15, 0x08, 0x8E);
    idt_set_entry(&idt_entries[16], (uint64_t)isr16, 0x08, 0x8E);
    idt_set_entry(&idt_entries[17], (uint64_t)isr17, 0x08, 0x8E);
    idt_set_entry(&idt_entries[18], (uint64_t)isr18, 0x08, 0x8E);
    idt_set_entry(&idt_entries[19], (uint64_t)isr19, 0x08, 0x8E);
    idt_set_entry(&idt_entries[20], (uint64_t)isr20, 0x08, 0x8E);
    idt_set_entry(&idt_entries[21], (uint64_t)isr21, 0x08, 0x8E);
    idt_set_entry(&idt_entries[22], (uint64_t)isr22, 0x08, 0x8E);
    idt_set_entry(&idt_entries[23], (uint64_t)isr23, 0x08, 0x8E);
    idt_set_entry(&idt_entries[24], (uint64_t)isr24, 0x08, 0x8E);
    idt_set_entry(&idt_entries[25], (uint64_t)isr25, 0x08, 0x8E);
    idt_set_entry(&idt_entries[26], (uint64_t)isr26, 0x08, 0x8E);
    idt_set_entry(&idt_entries[27], (uint64_t)isr27, 0x08, 0x8E);
    idt_set_entry(&idt_entries[28], (uint64_t)isr28, 0x08, 0x8E);
    idt_set_entry(&idt_entries[29], (uint64_t)isr29, 0x08, 0x8E);
    idt_set_entry(&idt_entries[30], (uint64_t)isr30, 0x08, 0x8E);
    idt_set_entry(&idt_entries[31], (uint64_t)isr31, 0x08, 0x8E);
    idt_set_entry(&idt_entries[32], (uint64_t)irq0, 0x08, 0x8E);
    idt_set_entry(&idt_entries[33], (uint64_t)irq1, 0x08, 0x8E);
    idt_set_entry(&idt_entries[34], (uint64_t)irq2, 0x08, 0x8E);
    idt_set_entry(&idt_entries[35], (uint64_t)irq3, 0x08, 0x8E);
    idt_set_entry(&idt_entries[36], (uint64_t)irq4, 0x08, 0x8E);
    idt_set_entry(&idt_entries[37], (uint64_t)irq5, 0x08, 0x8E);
    idt_set_entry(&idt_entries[38], (uint64_t)irq6, 0x08, 0x8E);
    idt_set_entry(&idt_entries[39], (uint64_t)irq7, 0x08, 0x8E);
    idt_set_entry(&idt_entries[40], (uint64_t)irq8, 0x08, 0x8E);
    idt_set_entry(&idt_entries[41], (uint64_t)irq9, 0x08, 0x8E);
    idt_set_entry(&idt_entries[42], (uint64_t)irq10, 0x08, 0x8E);
    idt_set_entry(&idt_entries[43], (uint64_t)irq11, 0x08, 0x8E);
    idt_set_entry(&idt_entries[44], (uint64_t)irq12, 0x08, 0x8E);
    idt_set_entry(&idt_entries[45], (uint64_t)irq13, 0x08, 0x8E);
    idt_set_entry(&idt_entries[46], (uint64_t)irq14, 0x08, 0x8E);
    idt_set_entry(&idt_entries[47], (uint64_t)irq15, 0x08, 0x8E);
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.offset = (uint64_t)&idt_entries;
    idt_load((uint64_t)&idt_ptr);
}
