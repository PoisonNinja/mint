#include <kernel.h>
#include <mm/heap.h>
#include <mm/valloc.h>

struct valloc_region {
    addr_t base;
    size_t extent;
    addr_t current;
};

struct valloc_region* valloc_create(addr_t base, size_t extent)
{
    struct valloc_region* region = kmalloc(sizeof(struct valloc_region));
    if (!region) {
        return NULL;
    }
    region->base = base;
    region->extent = extent;
    region->current = base;
    return region;
}

addr_t valloc_alloc(struct valloc_region* region, size_t size)
{
    if (region->current + size > (region->base + region->extent)) {
        return 0;
    }
    addr_t ret = region->current;
    region->current += size;
    return ret;
}

void valloc_free(struct valloc_region* region, addr_t addr)
{
    // A NOOP
}
