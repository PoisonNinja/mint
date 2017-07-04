#pragma once

#include <types.h>

struct memory_context {
    addr_t page_table;
};

void virtual_map(struct memory_context* context, addr_t virtual,
                 addr_t physical, size_t size, uint8_t flags);
