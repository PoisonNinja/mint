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

/*
 * Macros for possibly useful functions :)
 */
#pragma once

#include <types.h>

// Division, but rounding up
#define DIV_ROUND_UP(x, y) (((x) + ((y)-1)) / (y))

// Rounding
// x = value, y = multiple of
#define ROUND_DOWN(x, y) ((x) / (y) * (y))
#define ROUND_UP(x, y) ((((x) + ((y)-1)) / y) * y)

// Upper and lower bits
#define LOWER_32(x) ((x)&0xFFFFFFFF)
#define UPPER_32(x) (((x) >> 32) & 0xFFFFFFFF)
#define MERGE_64(x, y) (((uint64_t)(x) << 32) | (y))

// 2 ^ power
#define POW_2(power) (1 << (power))

static inline uint32_t log_2(uint64_t x)
{
    static const uint64_t t[6] = {0xFFFFFFFF00000000ull, 0x00000000FFFF0000ull,
                                  0x000000000000FF00ull, 0x00000000000000F0ull,
                                  0x000000000000000Cull, 0x0000000000000002ull};

    int y = (((x & (x - 1)) == 0) ? 0 : 1);
    int j = 32;
    int i;

    for (i = 0; i < 6; i++) {
        int k = (((x & t[i]) == 0) ? 0 : j);
        y += k;
        x >>= k;
        j >>= 1;
    }

    return y;
}
