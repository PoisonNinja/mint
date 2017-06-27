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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>

static struct interrupt_controller* interrupt_controller = NULL;
static struct interrupt_controller* interrupt_controllers = NULL;
static uint32_t irq_mask = 0;

static struct interrupt_controller* find_interrupt_controller(
    uint32_t controller)
{
    struct interrupt_controller* node = NULL;
    LIST_FOR_EACH(interrupt_controllers, node)
    {
        if (node->identifier == controller)
            return node;
    }
    return NULL;
}

int interrupt_controller_register(struct interrupt_controller* controller)
{
    LIST_APPEND(interrupt_controllers, controller);
    return 0;
}

int interrupt_controller_unregister(struct interrupt_controller* controller)
{
    if (interrupt_controller == controller)
        interrupt_controller = NULL;
    LIST_REMOVE(interrupt_controllers, controller);
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
