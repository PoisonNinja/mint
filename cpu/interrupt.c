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
#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>
#include <lib/list.h>
#include <stdatomic.h>

extern void arch_interrupt_disable(void);
extern void arch_interrupt_enable(void);

_Atomic int interrupt_depth = 0;

struct exception_handler* exception_handlers[EXCEPTIONS_MAX];
struct interrupt_handler* interrupt_handlers[INTERRUPTS_MAX];

void interrupt_disable(void)
{
    if (!atomic_fetch_add(&interrupt_depth, 1))
        arch_interrupt_disable();
}

void interrupt_enable(void)
{
    if (atomic_fetch_sub(&interrupt_depth, 1) == 1)
        arch_interrupt_enable();
}

void exception_handler_register(int exception_number,
                                struct exception_handler* handler)
{
    LIST_APPEND(exception_handlers[exception_number], handler);
}

void exception_handler_unregister(int exception_number,
                                  struct exception_handler* handler)
{
    LIST_REMOVE(exception_handlers[exception_number], handler);
}

void interrupt_handler_register(int interrupt_number,
                                struct interrupt_handler* handler)
{
    LIST_APPEND(interrupt_handlers[interrupt_number], handler);
}

void interrupt_handler_unregister(int interrupt_number,
                                  struct interrupt_handler* handler)
{
    LIST_REMOVE(interrupt_handlers[interrupt_number], handler);
}

extern char* arch_exception_translate(int);
void exception_dispatch(struct registers* regs)
{
    if (exception_handlers[regs->int_no]) {
        struct exception_handler* handler = NULL;
        LIST_FOR_EACH(exception_handlers[regs->int_no], handler)
        {
            if (handler->handler)
                handler->handler(regs, handler->dev_id);
        }
    } else {
        printk(INFO, "Unhandled exception %d: %s\n", regs->int_no,
               arch_exception_translate(regs->int_no));
    }
}

void interrupt_dispatch(struct registers* regs)
{
    if (interrupt_handlers[regs->int_no]) {
        struct interrupt_handler* handler = NULL;
        LIST_FOR_EACH(interrupt_handlers[regs->int_no], handler)
        {
            if (handler->handler)
                handler->handler(regs, handler->dev_id);
        }
    }
    interrupt_controller_ack(regs->int_no);
}
