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

#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/time/time.h>

#define INTEL_8253_IRQ 0

static int intel_8253_handler(struct interrupt_ctx* regs __attribute__((unused)),
                              void* dev_id __attribute__((unused)))
{
    // Call the generic tick handler
    tick_handler();
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
    // We want a frequency of HZ (currently 1000)
    int divisor = 1193180 / HZ;
    outb(0x43, 0x34);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
    return 0;
}
EARLY_INITCALL(init_intel_8253);
