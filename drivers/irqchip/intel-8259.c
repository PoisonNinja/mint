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
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>
#include <kernel/init.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

static int init_intel_8259(void)
{
    /*
     * Remap the PIC so we don't crash ourselves with an IRQ that ends up
     * being interpreted as a ISR exception
     */
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    return 0;
}

static int enable_intel_8259(void)
{
    // Let's just run through the entire init process again
    init_intel_8259();
    return 0;
}

static int disable_intel_8259(void)
{
    // Mask out all the interrupts.
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    return 0;
}

static int mask_intel_8259(int irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
    return 0;
}

static int unmask_intel_8259(int irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
    return 0;
}

static int ack_intel_8259(int int_no)
{
    if (int_no > 7) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
    return 0;
}

static struct interrupt_controller pic_info = {
    .name = "Intel 8259",
    .identifier = IC_INTEL_8259,
    .status = IC_STATUS_UNINITIALIZED,
    .init = init_intel_8259,
    .enable = enable_intel_8259,
    .disable = disable_intel_8259,
    .mask = mask_intel_8259,
    .unmask = unmask_intel_8259,
    .ack = ack_intel_8259,
};

static int register_intel_8259(void)
{
    interrupt_controller_register(&pic_info);
    return 0;
}
EARLY_INITCALL(register_intel_8259);
