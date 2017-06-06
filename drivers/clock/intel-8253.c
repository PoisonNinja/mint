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

#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/time/time.h>

#define INTEL_8253_IRQ 0

static int intel_8253_handler(struct registers* regs __attribute__((unused)),
                              void* dev_id __attribute__((unused)))
{
    uptime_increment();
    return 0;
}

static struct interrupt_handler intel_8253_interrupt = {
    .handler = &intel_8253_handler,
    .dev_name = "Intel 8253",
    .dev_id = &intel_8253_interrupt,
};

static int init_intel_8253(void)
{
    interrupt_handler_register(INTEL_8253_IRQ, &intel_8253_interrupt);
    int divisor = 1193180 / 1000;
    outb(0x43, 0x34);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
    return 0;
}
CORE_INITCALL(init_intel_8253);
