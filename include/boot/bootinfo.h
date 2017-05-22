/*
 * Copyright (C) 2017 Jason Lu (PoisonNinja)
 *
 * This file is part of Mint. Mint is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include <types.h>

#define MEMORY_TYPE_AVAILABLE 0x1
#define MEMORY_TYPE_RESERVED 0x2
#define MEMORY_TYPE_ACPI 0x3
#define MEMORY_TYPE_BAD 0x5

struct mint_memory_region {
    uint64_t lower;
    uint64_t upper;
    uint64_t type;
    struct mint_memory_region* next;
};

struct mint_bootinfo {
    char* cmdline;
    uint64_t total_mem;
    uint8_t num_memregions;
    struct mint_memory_region* memregions;
};
