#pragma once

#include <types.h>

#define PHYS_DMA 0x1
#define PHYS_ALIGN 0x2

extern void* physical_alloc(size_t size, uint8_t region);
extern void physical_free(void* addr, size_t size);

extern void physical_init(size_t size, addr_t dma_boundary);
extern void physical_free_region(addr_t start, size_t size);
