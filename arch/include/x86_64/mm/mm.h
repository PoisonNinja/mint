#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define PAGE_HUGE_SIZE 2097152
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
