#include <lib/hashmap.h>
#include <lib/list.h>
#include <lib/murmur.h>
#include <mm/heap.h>
#include <string.h>

struct bucket {
    const void* key;
    void* value;
    struct list_element list;
};

struct hashmap {
    size_t size;
    struct list_element buckets[];
};

static size_t hashmap_hash(const void* key, size_t keylen, size_t hashmap_size)
{
    size_t hash = murmur(key, keylen);
    return hash % hashmap_size;
}

struct hashmap* hashmap_create(size_t size)
{
    struct hashmap* hashmap =
        kzalloc(sizeof(struct hashmap) + (size * sizeof(struct list_element)));
    hashmap->size = size;
    for (size_t i = 0; i < size; i++) {
        list_runtime_init(&hashmap->buckets[i]);
    }
    return hashmap;
}

void* hashmap_get(struct hashmap* hashmap, const void* key,
                  const size_t key_size)
{
    size_t index = hashmap_hash(key, key_size, hashmap->size);
    if (list_empty(&hashmap->buckets[index]))
        return NULL;
    struct bucket* bucket = NULL;
    list_for_each(&hashmap->buckets[index], list, bucket)
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
    list_add(&hashmap->buckets[index], &bucket->list);
    return index;
}
