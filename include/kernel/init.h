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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
