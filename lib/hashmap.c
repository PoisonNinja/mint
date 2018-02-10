/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
    bucket->key = kzalloc(key_size);
    memcpy(bucket->key, key, key_size);
    bucket->value = value;
    list_add(&hashmap->buckets[index], &bucket->list);
    return index;
}
