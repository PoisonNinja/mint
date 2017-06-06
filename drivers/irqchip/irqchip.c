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

#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>

static struct interrupt_controller* interrupt_controller = NULL;
static struct list_head interrupt_controllers =
    LIST_HEAD_INIT(interrupt_controllers);
static uint32_t irq_mask = 0;

static struct interrupt_controller* find_interrupt_controller(
    uint32_t controller)
{
    struct interrupt_controller* node = NULL;
    list_for_each_entry(node, &interrupt_controllers, list)
    {
        if (node->identifier == controller)
            return node;
    }
    return NULL;
}

int interrupt_controller_register(struct interrupt_controller* controller)
{
    list_add(&controller->list, &interrupt_controllers);
    return 0;
}

int interrupt_controller_set(uint32_t controller)
{
    interrupt_disable();
    interrupt_controller_disable();
    /*
     * In the future, we will probably update the mask for the controller
     * and maybe even the status when doing this.
     */
    interrupt_controller = find_interrupt_controller(controller);
    if (interrupt_controller->status == IC_STATUS_UNINITIALIZED) {
        // Some chips may not need an init function
        if (interrupt_controller->init)
            interrupt_controller->init();
        interrupt_controller->status = IC_STATUS_DISABLED;
    }
    interrupt_controller_enable();
    interrupt_enable();
    return 0;
}

int interrupt_controller_enable(void)
{
    if (interrupt_controller->status == IC_STATUS_DISABLED) {
        if (interrupt_controller->enable) {
            interrupt_controller->enable();
        }
        for (int i = 0; i < 32; i++) {
            if (irq_mask & (1 << i))
                interrupt_controller_mask(i);
            else
                interrupt_controller_unmask(i);
        }
        interrupt_controller->status = IC_STATUS_ENABLED;
        return 0;
    } else {
        printk(WARNING, "irqchip: Chip already enabled!\n");
        return 1;
    }
}

int interrupt_controller_disable(void)
{
    if (interrupt_controller->status == IC_STATUS_ENABLED) {
        if (interrupt_controller->enable) {
            interrupt_controller->enable();
        }
        interrupt_controller->status = IC_STATUS_DISABLED;
        return 0;
    } else {
        printk(WARNING, "irqchip: Chip already disabled!\n");
        return 1;
    }
}

int interrupt_controller_mask(int irq)
{
    if (!(irq_mask & (1 << irq))) {
        if (interrupt_controller->status == IC_STATUS_ENABLED &&
            interrupt_controller->mask) {
            interrupt_controller->mask(irq);
        }
        irq_mask |= (1 << irq);
    }
    return 0;
}

int interrupt_controller_unmask(int irq)
{
    if (irq_mask & (1 << irq)) {
        if (interrupt_controller->status == IC_STATUS_ENABLED &&
            interrupt_controller->unmask) {
            interrupt_controller->unmask(irq);
        }
        irq_mask &= ~(1 << irq);
    }
    return 0;
}

int interrupt_controller_ack(int int_no)
{
    if (interrupt_controller->status == IC_STATUS_ENABLED &&
        interrupt_controller->ack) {
        interrupt_controller->ack(int_no);
        return 0;
    }
    return 1;
}
