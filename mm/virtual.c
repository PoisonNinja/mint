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

#include <arch/cpu/registers.h>
#include <arch/mm/mm.h>
#include <cpu/power.h>
#include <kernel.h>
#include <kernel/stacktrace.h>
#include <mm/virtual.h>

extern void arch_virtual_map(struct memory_context* context, addr_t virtual,
                             addr_t physical, uint8_t flags);

void virtual_map(struct memory_context* context, addr_t virtual,
                 addr_t physical, size_t size, uint8_t flags)
{
    if (!size || !context)
        return;
    size_t mapped = 0;
    while (mapped < size) {
        arch_virtual_map(context, virtual + mapped, physical + mapped, flags);
        mapped += (flags & PAGE_HUGE) ? PAGE_HUGE_SIZE : PAGE_SIZE;
    }
}

extern void arch_virtual_unmap(struct memory_context* context, addr_t virtual);

void virtual_unmap(struct memory_context* context, addr_t virtual)
{
    if (!context)
        return;
    arch_virtual_unmap(context, virtual);
}

void virtual_fault(struct registers* __attribute__((unused)) registers,
                   addr_t address, uint8_t reason)
{
    printk(ERROR, "PAGE FAULT\n");
    printk(ERROR, "  Address: %p\n", address);
    printk(ERROR, "  Present: %u\n", (reason & FAULT_PRESENT) ? 1 : 0);
    printk(ERROR, "  Write: %u\n", (reason & FAULT_WRITE) ? 1 : 0);
    printk(ERROR, "  User: %u\n", (reason & FAULT_USER) ? 1 : 0);
    printk(ERROR, "  Reserved: %u\n", (reason & FAULT_RESERVED) ? 1 : 0);
    printk(ERROR, "  Instruction fetch: %u\n", (reason & FAULT_IF) ? 1 : 0);
    stacktrace();
    for (;;)
        cpu_halt();
}
