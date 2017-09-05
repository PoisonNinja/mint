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
#include <kernel.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <string.h>

#define HEAP_UNINITIALIZED 0
#define HEAP_EARLY 1
#define HEAP_INITIALIZED 2

static uint8_t heap_status = HEAP_UNINITIALIZED;

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

static void *__attribute__((malloc)) early_malloc(size_t size)
{
    // Make sure we don't overshoot the extent
    if (early_heap_watermark + size > early_heap_end) {
        printk(INFO, "early_malloc: Request too large :(\n");
        return NULL;
    }
    // Convert watermark into a void pointer to return
    void *ret = (void *)early_heap_watermark;
    early_heap_watermark += size;
    return ret;
}

static uint64_t heap_end = HEAP_START;

static void *map_heap(size_t size)
{
    size *= PAGE_SIZE;
    virtual_map(&kernel_context, heap_end, (addr_t)physical_alloc(size, 0),
                size, PAGE_WRITABLE | PAGE_PRESENT);
    heap_end += size;
    return (void *)(heap_end - size);
}

static void free_heap(void *start, size_t size)
{
    size *= PAGE_SIZE;
    virtual_unmap(&kernel_context, (addr_t)start, size);
}

static int liballoc_lock(void)
{
    // spin_lock(heap_lock);
    return 0;
}

static int liballoc_unlock(void)
{
    // spin_unlock(heap_lock);
    return 0;
}

#define VERSION "1.1"
#define ALIGNMENT \
    16ul  // 4ul				///< This is the byte alignment that memory must
          // be
          // allocated on. IMPORTANT for GTK and other stuff.

#define ALIGN_TYPE char  /// unsigned char[16] /// unsigned short
#define ALIGN_INFO \
    sizeof(ALIGN_TYPE) * 16  ///< Alignment information is stored right before
                             /// the pointer. This is the number of bytes of
/// information stored there.

/** This macro will conveniently align our pointer upwards */
#define ALIGN(ptr)                                                          \
    if (ALIGNMENT > 1) {                                                    \
        uintptr_t diff;                                                     \
        ptr = (void *)((uintptr_t)ptr + ALIGN_INFO);                        \
        diff = (uintptr_t)ptr & (ALIGNMENT - 1);                            \
        if (diff != 0) {                                                    \
            diff = ALIGNMENT - diff;                                        \
            ptr = (void *)((uintptr_t)ptr + diff);                          \
        }                                                                   \
        *((ALIGN_TYPE *)((uintptr_t)ptr - ALIGN_INFO)) = diff + ALIGN_INFO; \
    }

#define UNALIGN(ptr)                                                     \
    if (ALIGNMENT > 1) {                                                 \
        uintptr_t diff = *((ALIGN_TYPE *)((uintptr_t)ptr - ALIGN_INFO)); \
        if (diff < (ALIGNMENT + ALIGN_INFO)) {                           \
            ptr = (void *)((uintptr_t)ptr - diff);                       \
        }                                                                \
    }

#define LIBALLOC_MAGIC 0xc001c0de
#define LIBALLOC_DEAD 0xdeaddead

/** A structure found at the top of all system allocated
 * memory blocks. It details the usage of the memory block.
 */
struct liballoc_major {
    struct liballoc_major *prev;  ///< Linked list information.
    struct liballoc_major *next;  ///< Linked list information.
    unsigned int pages;           ///< The number of pages in the block.
    unsigned int size;            ///< The number of pages in the block.
    unsigned int usage;           ///< The number of bytes used in the block.
    struct liballoc_minor
        *first;  ///< A pointer to the first allocated memory in the block.
};

/** This is a structure found at the beginning of all
 * sections in a major block which were allocated by a
 * malloc, calloc, realloc call.
 */
struct liballoc_minor {
    struct liballoc_minor *prev;  ///< Linked list information.
    struct liballoc_minor *next;  ///< Linked list information.
    struct liballoc_major
        *block;  ///< The owning block. A pointer to the major structure.
    unsigned int magic;  ///< A magic number to idenfity correctness.
    unsigned int
        size;  ///< The size of the memory allocated. Could be 1 byte or more.
    unsigned int req_size;  ///< The size of memory requested.
};

static struct liballoc_major *l_memRoot =
    NULL;  ///< The root memory block acquired from the system.
static struct liballoc_major *l_bestBet =
    NULL;  ///< The major with the most free memory.

static unsigned int l_pageSize =
    4096;  ///< The size of an individual page. Set up in liballoc_init.
static unsigned int l_pageCount =
    16;  ///< The number of pages to request per chunk. Set up in liballoc_init.
static unsigned long long l_allocated =
    0;                                  ///< Running total of allocated memory.
static unsigned long long l_inuse = 0;  ///< Running total of used memory.

static long long l_warningCount = 0;      ///< Number of warnings encountered
static long long l_errorCount = 0;        ///< Number of actual errors
static long long l_possibleOverruns = 0;  ///< Number of possible overruns

// ***********   HELPER FUNCTIONS  *******************************

static void *liballoc_memset(void *s, int c, size_t n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
        ((char *)s)[i] = c;

    return s;
}
static void *liballoc_memcpy(void *s1, const void *s2, size_t n)
{
    char *cdest;
    char *csrc;
    unsigned int *ldest = (unsigned int *)s1;
    unsigned int *lsrc = (unsigned int *)s2;

    while (n >= sizeof(unsigned int)) {
        *ldest++ = *lsrc++;
        n -= sizeof(unsigned int);
    }

    cdest = (char *)ldest;
    csrc = (char *)lsrc;

    while (n > 0) {
        *cdest++ = *csrc++;
        n -= 1;
    }

    return s1;
}

// ***************************************************************

static struct liballoc_major *allocate_new_page(unsigned int size)
{
    unsigned int st;
    struct liballoc_major *maj;

    // This is how much space is required.
    st = size + sizeof(struct liballoc_major);
    st += sizeof(struct liballoc_minor);

    // Perfect amount of space?
    if ((st % l_pageSize) == 0)
        st = st / (l_pageSize);
    else
        st = st / (l_pageSize) + 1;
    // No, add the buffer.

    // Make sure it's >= the minimum size.
    if (st < l_pageCount)
        st = l_pageCount;

    maj = (struct liballoc_major *)map_heap(st);

    if (maj == NULL) {
        l_warningCount += 1;
        return NULL;  // uh oh, we ran out of memory.
    }

    maj->prev = NULL;
    maj->next = NULL;
    maj->pages = st;
    maj->size = st * l_pageSize;
    maj->usage = sizeof(struct liballoc_major);
    maj->first = NULL;

    l_allocated += maj->size;

    return maj;
}

void *__attribute__((malloc)) malloc(size_t req_size)
{
    int startedBet = 0;
    unsigned long long bestSize = 0;
    void *p = NULL;
    uintptr_t diff;
    struct liballoc_major *maj;
    struct liballoc_minor *min;
    struct liballoc_minor *new_min;
    unsigned long size = req_size;

    // For alignment, we adjust size so there's enough space to align.
    if (ALIGNMENT > 1) {
        size += ALIGNMENT + ALIGN_INFO;
    }
    // So, ideally, we really want an alignment of 0 or 1 in order
    // to save space.

    liballoc_lock();

    if (size == 0) {
        l_warningCount += 1;
        liballoc_unlock();
        return malloc(1);
    }

    if (l_memRoot == NULL) {
        // This is the first time we are being used.
        l_memRoot = allocate_new_page(size);
        if (l_memRoot == NULL) {
            liballoc_unlock();
            return NULL;
        }
    }
    // Now we need to bounce through every major and find enough space....

    maj = l_memRoot;
    startedBet = 0;

    // Start at the best bet....
    if (l_bestBet != NULL) {
        bestSize = l_bestBet->size - l_bestBet->usage;

        if (bestSize > (size + sizeof(struct liballoc_minor))) {
            maj = l_bestBet;
            startedBet = 1;
        }
    }

    while (maj != NULL) {
        diff = maj->size - maj->usage;
        // free memory in the block

        if (bestSize < diff) {
            // Hmm.. this one has more memory then our bestBet. Remember!
            l_bestBet = maj;
            bestSize = diff;
        }

        // CASE 1:  There is not enough space in this major block.
        if (diff < (size + sizeof(struct liballoc_minor))) {
            // Another major block next to this one?
            if (maj->next != NULL) {
                maj = maj->next;  // Hop to that one.
                continue;
            }

            if (startedBet == 1)  // If we started at the best bet,
            {                     // let's start all over again.
                maj = l_memRoot;
                startedBet = 0;
                continue;
            }

            // Create a new major block next to this one and...
            maj->next = allocate_new_page(size);  // next one will be okay.
            if (maj->next == NULL)
                break;  // no more memory.
            maj->next->prev = maj;
            maj = maj->next;

            // .. fall through to CASE 2 ..
        }

        // CASE 2: It's a brand new block.
        if (maj->first == NULL) {
            maj->first =
                (struct liballoc_minor *)((uintptr_t)maj +
                                          sizeof(struct liballoc_major));

            maj->first->magic = LIBALLOC_MAGIC;
            maj->first->prev = NULL;
            maj->first->next = NULL;
            maj->first->block = maj;
            maj->first->size = size;
            maj->first->req_size = req_size;
            maj->usage += size + sizeof(struct liballoc_minor);

            l_inuse += size;

            p = (void *)((uintptr_t)(maj->first) +
                         sizeof(struct liballoc_minor));

            ALIGN(p);

            liballoc_unlock();  // release the lock
            return p;
        }

        // CASE 3: Block in use and enough space at the start of the block.
        diff = (uintptr_t)(maj->first);
        diff -= (uintptr_t)maj;
        diff -= sizeof(struct liballoc_major);

        if (diff >= (size + sizeof(struct liballoc_minor))) {
            // Yes, space in front. Squeeze in.
            maj->first->prev =
                (struct liballoc_minor *)((uintptr_t)maj +
                                          sizeof(struct liballoc_major));
            maj->first->prev->next = maj->first;
            maj->first = maj->first->prev;

            maj->first->magic = LIBALLOC_MAGIC;
            maj->first->prev = NULL;
            maj->first->block = maj;
            maj->first->size = size;
            maj->first->req_size = req_size;
            maj->usage += size + sizeof(struct liballoc_minor);

            l_inuse += size;

            p = (void *)((uintptr_t)(maj->first) +
                         sizeof(struct liballoc_minor));
            ALIGN(p);

            liballoc_unlock();  // release the lock
            return p;
        }

        // CASE 4: There is enough space in this block. But is it
        // contiguous?
        min = maj->first;

        // Looping within the block now...
        while (min != NULL) {
            // CASE 4.1: End of minors in a block. Space from last and end?
            if (min->next == NULL) {
                // the rest of this block is free...  is it big enough?
                diff = (uintptr_t)(maj) + maj->size;
                diff -= (uintptr_t)min;
                diff -= sizeof(struct liballoc_minor);
                diff -= min->size;
                // minus already existing usage..

                if (diff >= (size + sizeof(struct liballoc_minor))) {
                    // yay....
                    min->next =
                        (struct liballoc_minor *)((uintptr_t)min +
                                                  sizeof(
                                                      struct liballoc_minor) +
                                                  min->size);
                    min->next->prev = min;
                    min = min->next;
                    min->next = NULL;
                    min->magic = LIBALLOC_MAGIC;
                    min->block = maj;
                    min->size = size;
                    min->req_size = req_size;
                    maj->usage += size + sizeof(struct liballoc_minor);

                    l_inuse += size;

                    p = (void *)((uintptr_t)min +
                                 sizeof(struct liballoc_minor));
                    ALIGN(p);

                    liballoc_unlock();  // release the lock
                    return p;
                }
            }

            // CASE 4.2: Is there space between two minors?
            if (min->next != NULL) {
                // is the difference between here and next big enough?
                diff = (uintptr_t)(min->next);
                diff -= (uintptr_t)min;
                diff -= sizeof(struct liballoc_minor);
                diff -= min->size;
                // minus our existing usage.

                if (diff >= (size + sizeof(struct liballoc_minor))) {
                    // yay......
                    new_min =
                        (struct liballoc_minor *)((uintptr_t)min +
                                                  sizeof(
                                                      struct liballoc_minor) +
                                                  min->size);

                    new_min->magic = LIBALLOC_MAGIC;
                    new_min->next = min->next;
                    new_min->prev = min;
                    new_min->size = size;
                    new_min->req_size = req_size;
                    new_min->block = maj;
                    min->next->prev = new_min;
                    min->next = new_min;
                    maj->usage += size + sizeof(struct liballoc_minor);

                    l_inuse += size;

                    p = (void *)((uintptr_t)new_min +
                                 sizeof(struct liballoc_minor));
                    ALIGN(p);

                    liballoc_unlock();  // release the lock
                    return p;
                }
            }  // min->next != NULL

            min = min->next;
        }  // while min != NULL ...

        // CASE 5: Block full! Ensure next block and loop.
        if (maj->next == NULL) {
            if (startedBet == 1) {
                maj = l_memRoot;
                startedBet = 0;
                continue;
            }

            // we've run out. we need more...
            maj->next =
                allocate_new_page(size);  // next one guaranteed to be okay
            if (maj->next == NULL)
                break;  //  uh oh,  no more memory.....
            maj->next->prev = maj;
        }

        maj = maj->next;
    }  // while (maj != NULL)

    liballoc_unlock();  // release the lock
    return NULL;
}

void free(void *ptr)
{
    struct liballoc_minor *min;
    struct liballoc_major *maj;

    if (ptr == NULL) {
        l_warningCount += 1;
        return;
    }

    UNALIGN(ptr);

    liballoc_lock();  // lockit

    min = (struct liballoc_minor *)((uintptr_t)ptr -
                                    sizeof(struct liballoc_minor));

    if (min->magic != LIBALLOC_MAGIC) {
        l_errorCount += 1;

        // Check for overrun errors. For all bytes of LIBALLOC_MAGIC
        if (((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
            ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
            ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))) {
            l_possibleOverruns += 1;
        }

        // being lied to...
        liballoc_unlock();  // release the lock
        return;
    }

    maj = min->block;

    l_inuse -= min->size;

    maj->usage -= (min->size + sizeof(struct liballoc_minor));
    min->magic = LIBALLOC_DEAD;  // No mojo.

    if (min->next != NULL)
        min->next->prev = min->prev;
    if (min->prev != NULL)
        min->prev->next = min->next;

    if (min->prev == NULL)
        maj->first = min->next;
    // Might empty the block. This was the first
    // minor.

    // We need to clean up after the majors now....

    if (maj->first == NULL)  // Block completely unused.
    {
        if (l_memRoot == maj)
            l_memRoot = maj->next;
        if (l_bestBet == maj)
            l_bestBet = NULL;
        if (maj->prev != NULL)
            maj->prev->next = maj->next;
        if (maj->next != NULL)
            maj->next->prev = maj->prev;
        l_allocated -= maj->size;

        free_heap(maj, maj->pages);
    } else {
        if (l_bestBet != NULL) {
            int bestSize = l_bestBet->size - l_bestBet->usage;
            int majSize = maj->size - maj->usage;

            if (majSize > bestSize)
                l_bestBet = maj;
        }
    }

    liballoc_unlock();  // release the lock
}

void *__attribute__((malloc)) calloc(size_t nobj, size_t size)
{
    int real_size;
    void *p;

    real_size = nobj * size;

    p = malloc(real_size);

    liballoc_memset(p, 0, real_size);

    return p;
}

void *__attribute__((malloc)) realloc(void *p, size_t size)
{
    void *ptr;
    struct liballoc_minor *min;
    unsigned int real_size;

    // Honour the case of size == 0 => free old and return NULL
    if (size == 0) {
        free(p);
        return NULL;
    }

    // In the case of a NULL pointer, return a simple malloc.
    if (p == NULL)
        return malloc(size);

    // Unalign the pointer if required.
    ptr = p;
    UNALIGN(ptr);

    liballoc_lock();  // lockit

    min = (struct liballoc_minor *)((uintptr_t)ptr -
                                    sizeof(struct liballoc_minor));

    // Ensure it is a valid structure.
    if (min->magic != LIBALLOC_MAGIC) {
        l_errorCount += 1;

        // Check for overrun errors. For all bytes of LIBALLOC_MAGIC
        if (((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
            ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
            ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))) {
            l_possibleOverruns += 1;
        }

        // being lied to...
        liballoc_unlock();  // release the lock
        return NULL;
    }

    // Definitely a memory block.

    real_size = min->req_size;

    if (real_size >= size) {
        min->req_size = size;
        liballoc_unlock();
        return p;
    }

    liballoc_unlock();

    // If we got here then we're reallocating to a block bigger than us.
    ptr = malloc(size);  // We need to allocate new memory
    liballoc_memcpy(ptr, p, real_size);
    free(p);

    return ptr;
}

void *__attribute__((malloc)) kmalloc(size_t size)
{
    if (heap_status == HEAP_UNINITIALIZED) {
        printk(WARNING, "Attempting to use kmalloc before initializing?\n");
        return NULL;
    }
    if (heap_status == HEAP_EARLY) {
        return early_malloc(size);
    } else {
        return malloc(size);
    }
}

void *__attribute__((malloc)) kzalloc(size_t size)
{
    void *ptr = kmalloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void kfree(void *ptr)
{
    if (heap_status != HEAP_INITIALIZED) {
        printk(WARNING, "You can't use kfree until the heap is initialized!\n");
        return;
    }
    return free(ptr);
}

void kmalloc_init(void)
{
    heap_status = HEAP_INITIALIZED;
}
