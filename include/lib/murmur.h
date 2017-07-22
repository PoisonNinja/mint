#pragma once

#include <types.h>

#define MURMUR_SEED 0x1234ABCD

uint32_t murmur_hash(const void* key, uint32_t len, uint32_t seed);

#define murmur(key, len) murmur_hash(key, len, MURMUR_SEED)
