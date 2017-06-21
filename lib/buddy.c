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

struct buddy* buddy_init(size_t size, uint8_t min, uint8_t max)
{
    struct buddy* buddy = kmalloc(sizeof(struct buddy));
    buddy->min_order = min;
    buddy->max_order = max;
    for (int i = min; i <= max; i++) {
        stack_init(&buddy->orders[i].free);
        size_t space = BITSET_SIZE_CALC(size / POW_2(i));
        buddy->orders[i].bitset = kmalloc(space);
        memset(buddy->orders[i].bitset, 0, space);
    }
    for (addr_t i = 0; i < size; i += POW_2(max)) {
        struct stack_item* item = (struct stack_item*)i;
        item->data = (void*)i;
        stack_push(&buddy->orders[max].free, item);
    }
    return buddy;
}

void* buddy_alloc(struct buddy* buddy, size_t size)
{
    uint32_t order = log_2(size);
    if (order < buddy->min_order)
        order = buddy->min_order;
    if (order > buddy->max_order) {
        return NULL;
    }
    uint32_t original_order = order;
    if (!buddy->orders[order].free.size) {
        while (order <= buddy->max_order) {
            if (buddy->orders[order].free.size)
                break;
            order++;
        }
        void* addr = stack_pop(&buddy->orders[order].free);
        for (; order > original_order; order--) {
            bitset_set(buddy->orders[order].bitset,
                       (addr_t)addr / POW_2(order));
            bitset_set(buddy->orders[order - 1].bitset,
                       (addr_t)addr / POW_2(order - 1));
            struct stack_item* item =
                (struct stack_item*)BUDDY_ADDRESS((addr_t)addr, order - 1);
            item->data = (void*)BUDDY_ADDRESS((addr_t)addr, order - 1);
            stack_push(&buddy->orders[order - 1].free, item);
        }
        return addr;
    } else {
        void* addr = stack_pop(&buddy->orders[order].free);
        bitset_set(buddy->orders[order].bitset, (addr_t)addr / POW_2(order));
        return addr;
    }
}
