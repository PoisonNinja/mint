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

static char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check?",
    "Machine Check!",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Unknown",
};

void arch_interrupt_disable(void)
{
    __asm__("cli");
}

void arch_interrupt_enable(void)
{
    __asm__("sti");
}

extern void exception_dispatch(struct registers*);
void x86_64_exception_handler(struct registers* regs)
{
    exception_dispatch(regs);
}

extern void interrupt_dispatch(struct registers*);
void x86_64_interrupt_handler(struct registers* regs)
{
    regs->int_no -= 32;  // Translate IRQ into 0 based
    interrupt_dispatch(regs);
}

char* arch_exception_translate(int exception_no)
{
    if (exception_no > 31)
        return exception_messages[32];
    else
        return exception_messages[exception_no];
}
