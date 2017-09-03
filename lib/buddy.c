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

#include <lib/bitset.h>
#include <lib/buddy.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <string.h>

#define BUDDY_ADDRESS(x, order) ((x) ^ (1 << (order)))
#define BUDDY_INDEX(x, order) ((x) / (POW_2(order)))

struct buddy* buddy_init(addr_t base, addr_t virtual_base, size_t size,
                         uint8_t min, uint8_t max)
{
    if (!size || !min || !max)
        return NULL;
    struct buddy* buddy = kmalloc(sizeof(struct buddy));
    // No zero checks for base because 0 is a valid base (e.g. low memory)
    buddy->base = base;
    buddy->virtual_base = virtual_base;
    buddy->size = size;
    buddy->min_order = min;
    buddy->max_order = max;
    for (int i = min; i <= max; i++) {
        void* test = kmalloc(0x1000);
        stack_init(&buddy->orders[i].free, test);
        size_t space = BITSET_SIZE_CALC(size / POW_2(i));
        buddy->orders[i].bitset = kmalloc(space);
        memset(buddy->orders[i].bitset, BITSET_FULL, space);
    }
    return buddy;
}

void* buddy_alloc(struct buddy* buddy, size_t size)
{
    if (!buddy)
        return NULL;
    uint32_t order = log_2(size);
    if (order < buddy->min_order)
        order = buddy->min_order;
    if (order > buddy->max_order)
        return NULL;
    uint32_t original_order = order;
    if (!buddy->orders[order].free.size) {
        int found = 0;
        while (order++ <= buddy->max_order) {
            if (buddy->orders[order].free.size) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return NULL;
        }
        void* addr = (void*)stack_pop(&buddy->orders[order].free);
        for (; order > original_order; order--) {
            bitset_set(buddy->orders[order].bitset,
                       BUDDY_INDEX((addr_t)addr - buddy->base, order));
            bitset_set(buddy->orders[order - 1].bitset,
                       BUDDY_INDEX((addr_t)addr - buddy->base, order - 1));
            stack_push(&buddy->orders[order - 1].free,
                       BUDDY_ADDRESS((addr_t)addr, order - 1));
        }
        return addr;
    } else {
        void* addr = (void*)stack_pop(&buddy->orders[order].free);
        bitset_set(buddy->orders[order].bitset,
                   BUDDY_INDEX((addr_t)addr - buddy->base, order));
        return addr;
    }
}

void buddy_free(struct buddy* buddy, void* addr, size_t size)
{
    uint32_t order = log_2(size);
    for (; order <= buddy->max_order; order++) {
        bitset_unset(buddy->orders[order].bitset,
                     BUDDY_INDEX((addr_t)addr - buddy->base, order));
        addr_t buddy_addr = BUDDY_ADDRESS((addr_t)addr, order);
        if (bitset_test(buddy->orders[order].bitset,
                        BUDDY_INDEX(buddy_addr - buddy->base, order)) ||
            order == buddy->max_order) {
            stack_push(&buddy->orders[order].free, (addr_t)addr);
            break;
        }
    }
}

void buddy_free_region(struct buddy* buddy, addr_t start, size_t size)
{
    addr_t end = start + size;
    addr_t addr = start;
    for (uint32_t order = buddy->max_order; order >= buddy->min_order;
         order--) {
        size_t chunk = POW_2(order);
        while (addr + chunk <= end) {
            buddy_free(buddy, (void*)addr, chunk);
            addr += chunk;
        }
    }
}
