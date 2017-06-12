#pragma once

#include <lib/math.h>
#include <types.h>

#define BITSET_SIZE_CALC(size) (DIV_ROUND_UP(size, 8))
#define BITSET_INDEX(index) (index / 8)
#define BITSET_DECLARE(name, size) uint8_t name[BITSET_SIZE_CALC(size)];

#define bitset_set(bitset, index) bitset[index / 8] |= (1 << (index % 8))
#define bitset_unset(bitset, index) bitset[index / 8] &= ~(1 << (index % 8))
#define bitset_test(bitset, index) \
    ((bitset[index / 8] & (1 << (index % 8))) ? 1 : 0)

static inline void bitset_set_group(uint8_t* bitset, size_t start, size_t size)
{
    for (size_t index = start; index < start + size; index++) {
        bitset_set(bitset, index);
    }
}
