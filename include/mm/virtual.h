#pragma once

#include <types.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4
#define PAGE_GLOBAL 0x8
#define PAGE_NX 0x10
#define PAGE_HUGE 0x20
#define PAGE_COW 0x40

struct memory_context {
    addr_t page_table;
};

void virtual_map(struct memory_context* context, addr_t virtual,
                 addr_t physical, size_t size, uint8_t flags);
