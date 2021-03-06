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

#pragma once

#include <lib/math.h>
#include <types.h>

#define BITSET_SIZE_CALC(size) (DIV_ROUND_UP(size, 8))
#define BITSET_INDEX(index) (index / 8)
#define BITSET_DECLARE(name, size) uint8_t name[BITSET_SIZE_CALC(size)];

#define BITSET_FULL 0xFF
#define BITSET_EMPTY 0x00

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

static inline size_t bitset_first_unset(uint8_t* bitset, size_t size)
{
    for (uint8_t j = 0; j < size; j++) {
        if (!bitset_test(bitset, j))
            return j;
    }
    return 0;
}
