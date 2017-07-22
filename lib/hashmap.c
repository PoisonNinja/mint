#include <lib/hashmap.h>
#include <lib/list.h>
#include <lib/murmur.h>
#include <mm/heap.h>
#include <string.h>

struct bucket {
    const void* key;
    void* value;
    struct bucket *next, *prev;
};

struct hashmap {
    size_t size;
    struct bucket* buckets[];
};

static size_t hashmap_hash(const void* key, size_t keylen, size_t hashmap_size)
{
    size_t hash = murmur(key, keylen);
    return hash % hashmap_size;
}

struct hashmap* hashmap_create(size_t size)
{
    struct hashmap* hashmap = kzalloc(sizeof(struct hashmap));
    hashmap->size = size;
    return hashmap;
}

void* hashmap_get(struct hashmap* hashmap, const void* key,
                  const size_t key_size)
{
    size_t index = hashmap_hash(key, key_size, hashmap->size);
    if (!hashmap->buckets[index])
        return NULL;
    struct bucket* bucket = NULL;
    LIST_FOR_EACH(hashmap->buckets[index], bucket)
    {
        if (!memcmp(bucket->key, key, key_size)) {
            return bucket->value;
        }
    }
    return NULL;
}

size_t hashmap_set(struct hashmap* hashmap, const void* key,
                   const size_t key_size, void* value)
{
    size_t index = hashmap_hash(key, key_size, hashmap->size);
    struct bucket* bucket = kzalloc(sizeof(struct bucket));
    bucket->key = key;
    bucket->value = value;
    LIST_PREPEND(hashmap->buckets[index], bucket);
    return index;
}
