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

static inline void outb(uint16_t port, uint8_t v)
{
    asm volatile("outb %0,%1" : : "a"(v), "dN"(port));
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile("inb %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void outw(uint16_t port, uint16_t v)
{
    asm volatile("outw %0,%1" : : "a"(v), "dN"(port));
}
static inline uint16_t inw(uint16_t port)
{
    uint16_t v;
    asm volatile("inw %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void outl(uint16_t port, uint32_t v)
{
    asm volatile("outl %0,%1" : : "a"(v), "dN"(port));
}
static inline uint32_t inl(uint16_t port)
{
    uint32_t v;
    asm volatile("inl %1,%0" : "=a"(v) : "dN"(port));
    return v;
}
