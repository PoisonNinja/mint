#include <arch/mm/mm.h>
#include <arch/mm/mmap.h>
#include <kernel.h>
#include <lib/bitset.h>
#include <mm/physical.h>
#include <mm/slab.h>
#include <string.h>

static struct slab_cache slab_cache_cache;

#define SLAB_MIN_OBJECT_PER_SLAB 8
#define SLAB_CALC_ORDER(objsize) \
    log_2(ROUND_UP((objsize)*SLAB_MIN_OBJECT_PER_SLAB, PAGE_SIZE))

#define SLAB_ARENA_SIZE(cache, slab) \
    ((addr_t)slab + POW_2((cache)->order) - (addr_t)(slab)->base)
#define SLAB_CACHE_MAX(cache, slab) \
    (SLAB_ARENA_SIZE(cache, slab) / (cache)->objsize)
#define SLAB_BITSET_SIZE(cache, slab) \
    (BITSET_SIZE_CALC(POW_2((cache)->order) / (cache)->objsize))

static void* __slab_allocate_slab(struct slab_cache* cache, struct slab* slab)
{
    size_t idx = bitset_first_unset(slab->bitset, SLAB_CACHE_MAX(cache, slab));
    bitset_set(slab->bitset, idx);
    slab->used++;
    return (void*)((addr_t)slab->base + cache->objsize * idx);
}

static void __slab_free_slab(struct slab_cache* cache, struct slab* slab,
                             void* ptr)
{
    size_t idx = (ptr - slab->base) / cache->objsize;
    bitset_unset(slab->bitset, idx);
    slab->used--;
}

void* slab_allocate(struct slab_cache* cache)
{
    if (!list_empty(&cache->partial_slabs)) {
        struct slab* slab =
            list_first_entry(&cache->partial_slabs, struct slab, list);
        void* ret = __slab_allocate_slab(cache, slab);
        if (slab->used == SLAB_CACHE_MAX(cache, slab)) {
            list_delete(&slab->list);
            list_add(&cache->full_slabs, &slab->list);
        }
        return ret;
    } else if (!list_empty(&cache->empty_slabs)) {
        struct slab* slab =
            list_first_entry(&cache->empty_slabs, struct slab, list);
        void* ret = __slab_allocate_slab(cache, slab);
        list_delete(&slab->list);
        list_add(&cache->partial_slabs, &slab->list);
        return ret;
    } else {
        struct slab* slab =
            (void*)((addr_t)physical_alloc(POW_2(cache->order), 0) +
                    PHYS_START);
        memset(slab, 0, POW_2(cache->order));
        slab->cache = cache;
        slab->base = (void*)((addr_t)slab + sizeof(struct slab) +
                             SLAB_BITSET_SIZE(cache, slab));
        void* ret = __slab_allocate_slab(cache, slab);
        list_add(&cache->partial_slabs, &slab->list);
        return ret;
    }
}

void slab_free(void* ptr)
{
    struct slab* slab = (struct slab*)((addr_t)ptr & PAGE_MASK);
    struct slab_cache* cache = slab->cache;
    __slab_free_slab(cache, slab, ptr);
    if (!slab->used) {
        list_delete(&slab->list);
        list_add(&cache->empty_slabs, &slab->list);
    } else if (slab->used == SLAB_CACHE_MAX(cache, slab) - 1) {
        list_delete(&slab->list);
        list_add(&cache->partial_slabs, &slab->list);
    }
}

struct slab_cache* slab_create(char* name, size_t objsize, uint8_t flags)
{
    struct slab_cache* cache = slab_allocate(&slab_cache_cache);
    strncpy(cache->name, name, SLAB_NAME_MAX);
    cache->objsize = objsize;
    cache->order = SLAB_CALC_ORDER(objsize);
    cache->flags = flags;
    list_runtime_init(&cache->full_slabs);
    list_runtime_init(&cache->partial_slabs);
    list_runtime_init(&cache->empty_slabs);
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
    slab_cache_cache.order = SLAB_CALC_ORDER(slab_cache_cache.objsize);
    slab_cache_cache.flags = 0;
    list_runtime_init(&slab_cache_cache.full_slabs);
    list_runtime_init(&slab_cache_cache.partial_slabs);
    list_runtime_init(&slab_cache_cache.empty_slabs);
}
