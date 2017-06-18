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

#include <kernel.h>
#include <kernel/init.h>
#include <types.h>

extern uint64_t __initcall1_start;
extern uint64_t __initcall2_start;
extern uint64_t __initcall3_start;
extern uint64_t __initcall4_start;
extern uint64_t __initcall5_start;
extern uint64_t __initcall6_start;
extern uint64_t __initcall7_start;
extern uint64_t __initcall_end;

static uint64_t *initcall_levels[8] = {
    &__initcall1_start, &__initcall2_start, &__initcall3_start,
    &__initcall4_start, &__initcall5_start, &__initcall6_start,
    &__initcall7_start, &__initcall_end,
};

void do_initcall(int level)
{
    printk(INFO, "Calling initcalls for level %d\n", level);
    for (uint64_t *i = initcall_levels[level - 1]; i < initcall_levels[level];
         i++) {
        initcall_t fn = *(initcall_t *)i;
        fn();
    }
}
