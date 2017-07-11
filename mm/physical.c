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

#include <arch/mm/mmap.h>
#include <kernel.h>
#include <lib/buddy.h>
#include <mm/heap.h>
#include <mm/physical.h>
#include <string.h>

struct physical_region {
    addr_t base;
    size_t size;
    struct buddy* buddy;
};

static struct physical_region dma_region;
static struct physical_region normal_region;

void physical_init(size_t size, addr_t dma_boundary)
{
    dma_region.base = 0x0;
    dma_region.size = dma_boundary;
    dma_region.buddy =
        buddy_init(dma_region.base, PHYS_START, dma_region.size, 12, 16);
    normal_region.base = dma_boundary;
    normal_region.size = size - dma_boundary;
    normal_region.buddy =
        buddy_init(normal_region.base, PHYS_START, normal_region.size, 12, 28);
}

void* physical_alloc(size_t size, uint8_t flags)
{
    void* ret = 0;
    if (flags & PHYS_DMA) {
        ret = buddy_alloc(dma_region.buddy, size);
    } else {
        ret = buddy_alloc(normal_region.buddy, size);
    }
    memset(ret + PHYS_START, 0, size);
    return ret;
}

void physical_free(void* addr, size_t size)
{
    if ((addr_t)addr < dma_region.size) {
        return buddy_free(dma_region.buddy, addr, size);
    } else {
        return buddy_free(normal_region.buddy, addr, size);
    }
}

void physical_free_region(addr_t start, size_t size)
{
    if (start < dma_region.size) {
        if (start < dma_region.size && start + size >= dma_region.size) {
            size_t dma_size = (dma_region.base + dma_region.size) - start;
            size -= dma_size;
            buddy_free_region(dma_region.buddy, start, dma_size);
            start = dma_region.base + dma_region.size;
            buddy_free_region(normal_region.buddy, start, size);
        } else {
            buddy_free_region(dma_region.buddy, start, size);
        }
    } else {
        buddy_free_region(normal_region.buddy, start, size);
    }
}
