#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <kernel.h>
#include <lib/bitset.h>
#include <lib/list.h>
#include <mm/physical.h>
#include <mm/slab.h>
#include <string.h>

static struct slab_cache slab_cache_cache;

#define SLAB_ARENA_SIZE(cache, slab) (PAGE_SIZE - (addr_t)(slab)->base)
#define SLAB_CACHE_MAX(cache, slab) \
    (SLAB_ARENA_SIZE(cache, slab) / (cache)->objsize)
#define SLAB_BITSET_SIZE(cache, slab) \
    (BITSET_SIZE_CALC(PAGE_SIZE / (cache)->objsize))

static void* __slab_allocate_slab(struct slab_cache* cache, struct slab* slab)
{
    size_t idx = bitset_first_unset(slab->bitset, SLAB_CACHE_MAX(cache, slab));
    bitset_set(slab->bitset, idx);
    slab->used++;
    return (void*)((addr_t)slab->base + cache->objsize * idx);
}

void* slab_allocate(struct slab_cache* cache)
{
    if (cache->partial_slabs) {
        struct slab* slab = cache->partial_slabs;
        void* ret = __slab_allocate_slab(cache, slab);
        if (slab->used == SLAB_CACHE_MAX(cache, slab)) {
            LIST_REMOVE(cache->partial_slabs, slab);
            LIST_PREPEND(cache->full_slabs, slab);
        }
        return ret;
    } else if (cache->empty_slabs) {
        struct slab* slab = cache->empty_slabs;
        void* ret = __slab_allocate_slab(cache, slab);
        LIST_REMOVE(cache->empty_slabs, slab);
        LIST_PREPEND(cache->partial_slabs, slab);
        return ret;
    } else {
        struct slab* slab =
            (void*)((addr_t)physical_alloc(PAGE_SIZE, 0) + PHYS_START);
        memset(slab, 0, PAGE_SIZE);
        slab->base = (void*)((addr_t)slab + sizeof(struct slab) +
                             SLAB_BITSET_SIZE(cache, slab));
        void* ret = __slab_allocate_slab(cache, slab);
        LIST_PREPEND(cache->partial_slabs, slab);
        return ret;
    }
}

struct slab_cache* slab_create(char* name, size_t objsize, uint8_t flags)
{
    struct slab_cache* cache = slab_allocate(&slab_cache_cache);
    strncpy(cache->name, name, SLAB_NAME_MAX);
    cache->objsize = objsize;
    cache->flags = flags;
    // Actual slabs are lazily allocated to avoid memory waste
    return cache;
}

/*
 * slab_init
 *
 * Initialize the slab allocator system itself by initializing the
 * slab_cache_cache struct so we can allocate slab_caches
 */
void slab_init()
{
    memset(&slab_cache_cache, 0, sizeof(struct slab_cache));
    // Basically same as slab_create
    strncpy(slab_cache_cache.name, "slab_cache", SLAB_NAME_MAX);
    slab_cache_cache.objsize = POW_2(log_2(sizeof(struct slab_cache)));
    slab_cache_cache.flags = 0;
}
