#pragma once

#include <types.h>

struct valloc_region;

extern struct valloc_region* valloc_create(addr_t base, size_t extent);
extern addr_t valloc_alloc(struct valloc_region* region, size_t size);
extern void valloc_free(struct valloc_region* region, addr_t addr);
