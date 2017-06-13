#pragma once

#include <types.h>

extern void early_malloc_set_properties(addr_t start, addr_t extent);
extern void* __attribute__((malloc)) early_malloc(size_t size);
