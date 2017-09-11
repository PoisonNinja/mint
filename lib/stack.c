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

#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <lib/stack.h>
#include <mm/virtual.h>

// A known virtual address to map into
__attribute__((aligned(0x1000))) static uint8_t phys_page_virt[0x1000];

static volatile addr_t* phys_page = (addr_t*)&phys_page_virt;

static inline void stack_map(addr_t physical)
{
    return virtual_map(&kernel_context, (addr_t)&phys_page_virt, physical,
                       0x1000, PAGE_PRESENT | PAGE_WRITABLE);
}

void stack_init(struct stack* stack)
{
    stack->size = 0;
    stack->top = 0;
}

void stack_push(struct stack* stack, addr_t address)
{
    stack_map(address);
    *phys_page = stack->top;
    stack->top = address;
    stack->size++;
}

addr_t stack_pop(struct stack* stack)
{
    if (stack->size == 0)
        return 0;
    stack_map(stack->top);
    addr_t ret = stack->top;
    stack->top = *phys_page;
    stack->size--;
    return ret;
}
