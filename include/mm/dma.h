#pragma once

#include <types.h>

struct dma_region {
    addr_t physical_base;
    addr_t virtual_base;
    size_t extent;
};

extern struct dma_region* dma_alloc(size_t size);

extern void dma_init();
