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

#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFFFFFFFFFF000

#define PML4_INDEX(x) ((x >> 39) & 0x1FF)
#define PDPT_INDEX(x) ((x >> 30) & 0x1FF)
#define PD_INDEX(x) ((x >> 21) & 0x1FF)
#define PT_INDEX(x) ((x >> 12) & 0x1FF)

#define RECURSIVE_ENTRY 510

struct page {
    uint32_t present : 1;
    uint32_t writable : 1;
    uint32_t user : 1;
    uint32_t writethrough_cache : 1;
    uint32_t disable_cache : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t huge_page : 1;
    uint32_t global : 1;
    uint32_t cow : 1;
    uint32_t available_1 : 2;
    uint64_t address : 40;
    uint32_t available_2 : 11;
    uint32_t nx : 1;
};

struct page_table {
    struct page pages[512];
};

static inline uint64_t read_cr3(void)
{
    uint64_t value;
    __asm__("mov %%cr3, %%rax" : "=a"(value));
    return value;
}

static inline void write_cr3(uint64_t value)
{
    __asm__("mov %%rax, %%cr3" : : "a"(value));
}

static inline void invlpg(addr_t addr)
{
    __asm__ __volatile__("invlpg (%0)" ::"r"(addr) : "memory");
}

/*
 * Convert a table entry into the recursive mapping address. Taken from
 * some forum post on osdev.org
 */
static inline void* entry_to_address(uint64_t pml4, uint64_t pdp, uint64_t pd,
                                     uint64_t pt)
{
    uint64_t address = (pml4 << 39);

    if ((address & (1ll << 47)) > 0) {
        // We need to sign extend
        address |= 0xFFFF000000000000UL;
    }

    address |= pdp << 30;
    address |= pd << 21;
    address |= pt << 12;
    return (void*)address;
}
