/*
 * Copyright (C) 2017 Jason Lu (PoisonNinja) and contributors
 *
 * This file is part of Strawberry. Strawberry is free software: you can
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

typedef int (*initcall_t)(void);

#define EARLY_INIT 1
#define CORE_INIT 2
#define ARCH_INIT 3
#define SUBSYS_INIT 4
#define FS_INIT 5
#define DEVICE_INIT 6
#define LATE_INIT 7

// clang-format off
#define __define_initcall(fn, id)   \
    initcall_t __initcall_##id##fn  \
        __attribute__((section(".initcall"#id), used)) = fn;
// clang-format on

#define EARLY_INITCALL(fn) __define_initcall(fn, 1)
#define CORE_INITCALL(fn) __define_initcall(fn, 2)
#define ARCH_INITCALL(fn) __define_initcall(fn, 3)
#define SUBSYS_INITCALL(fn) __define_initcall(fn, 4)
#define FS_INITCALL(fn) __define_initcall(fn, 5)
#define DEVICE_INITCALL(fn) __define_initcall(fn, 6)
#define LATE_INITCALL(fn) __define_initcall(fn, 7)

extern void do_initcall(int);
extern void setup_arch(void);
