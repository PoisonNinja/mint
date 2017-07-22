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

#include <kernel.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/slab.h>
#include <string.h>

#define HEAP_UNINITIALIZED 0
#define HEAP_EARLY 1
#define HEAP_INITIALIZED 2

static uint8_t heap_status = HEAP_UNINITIALIZED;

#define KMALLOC_SLAB_MIN_ORDER 5   // 32 bytes
#define KMALLOC_SLAB_MAX_ORDER 17  // 128 KiB

static struct slab_cache*
    kmalloc_caches[(KMALLOC_SLAB_MAX_ORDER - KMALLOC_SLAB_MIN_ORDER) + 1];

// Variables for early heap
static addr_t early_heap_start = 0;
static addr_t early_heap_end = 0;
static addr_t early_heap_extent = 0;
static addr_t early_heap_watermark = 0;

void early_malloc_set_properties(addr_t start, addr_t extent)
{
    if (!start || !extent)
        return;
    // No, I don't want to configure this twice
    if (heap_status) {
        printk(WARNING, "early_malloc: Already configured! Allowing this "
                        "action, but unexpected behavior may occur\n");
    }
    // Set the early heap state
    early_heap_start = start;
    early_heap_extent = extent;
    // Precalculate the heap end
    early_heap_end = start + extent;
    // Set the heap pointer to the start of the memory region
    early_heap_watermark = start;
    printk(INFO, "early_malloc: Start at %p, extent is %p\n", start, extent);
    heap_status = HEAP_EARLY;
}

static void* __attribute__((malloc)) early_malloc(size_t size)
{
    // Make sure we don't overshoot the extent
    if (early_heap_watermark + size > early_heap_end) {
        printk(INFO, "early_malloc: Request too large :(\n");
        return NULL;
    }
    // Convert watermark into a void pointer to return
    void* ret = (void*)early_heap_watermark;
    early_heap_watermark += size;
    return ret;
}

void* __attribute__((malloc)) kmalloc(size_t size)
{
    if (heap_status == HEAP_UNINITIALIZED) {
        printk(WARNING, "Attempting to use kmalloc before initializing?\n");
        return NULL;
    }
    if (heap_status == HEAP_EARLY) {
        return early_malloc(size);
    } else {
        /*
         * The real logic behind kmalloc is the slab allocator, so this is
         * pretty barebones as we really only need to select the correct slab
         * cache to allocate from.
         *
         * Use POW_2 instead of log_2 because the compiler can optimize
         * the macro result into a constant
         */
        if (size > POW_2(KMALLOC_SLAB_MAX_ORDER)) {
            printk(WARNING,
                   "Attempted to allocate more memory than supported!\n");
            return NULL;
        }
        uint8_t order = log_2(size);
        if (order < KMALLOC_SLAB_MIN_ORDER)
            order = KMALLOC_SLAB_MIN_ORDER;
        return slab_allocate(kmalloc_caches[order - KMALLOC_SLAB_MIN_ORDER]);
    }
}

void* __attribute__((malloc)) kzalloc(size_t size)
{
    void* ptr = kmalloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void kfree(void* ptr)
{
    if (heap_status != HEAP_INITIALIZED) {
        printk(WARNING, "You can't use kfree until the heap is initialized!\n");
        return;
    }
    return slab_free(ptr);
}

void kmalloc_init(void)
{
    for (int order = KMALLOC_SLAB_MIN_ORDER; order <= KMALLOC_SLAB_MAX_ORDER;
         order++) {
        kmalloc_caches[order - KMALLOC_SLAB_MIN_ORDER] =
            slab_create("kmalloc_cache", POW_2(order), 0);
    }
    heap_status = HEAP_INITIALIZED;
}
