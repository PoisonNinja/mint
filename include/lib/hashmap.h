#pragma once

#include <types.h>

struct hashmap;

struct hashmap* hashmap_create(size_t size);
void* hashmap_get(struct hashmap* hashmap, const void* key,
                  const size_t key_size);
size_t hashmap_set(struct hashmap* hashmap, const void* key,
                   const size_t key_size, void* value);
