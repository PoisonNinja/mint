#pragma once

#include <types.h>

#define SLAB_NAME_MAX 16

struct slab_cache {
    char name[SLAB_NAME_MAX];
    size_t objsize;
    uint32_t order;
    uint8_t flags;
    struct slab *full_slabs, *partial_slabs, *empty_slabs;
};

struct slab {
    struct slab_cache* cache;
    struct slab *next, *prev;
    void* base;
    uint16_t used;
    uint8_t bitset[];
};

extern void* slab_allocate(struct slab_cache* cache);
extern void slab_free(void* ptr);
extern struct slab_cache* slab_create(char* name, size_t objsize,
                                      uint8_t flags);
extern void slab_init();
