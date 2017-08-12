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

#include <arch/mm/mmap.h>
#include <kernel/init.h>
#include <mm/physical.h>
#include <tests/tests.h>

static int test_physical(void)
{
    START_TEST("Physical memory manager");
    void* oldptr = physical_alloc(0x1000, 0);
    ASSERT("Physical memory is allocated", oldptr);
    physical_free(oldptr, 0x1000);
    void* ptr = NULL;
    ASSERT("Physical memory returned is same as before",
           oldptr == (ptr = physical_alloc(0x1000, 0)));
    physical_free(ptr, 0x1000);
    ASSERT("Physical memory is DMA-capable with PHYS_DMA",
           (addr_t)(ptr = physical_alloc(0x1000, PHYS_DMA)) < DMA_MAX);
    physical_free(ptr, 0x1000);
    ASSERT("Physical memory is normal without PHYS_DMA",
           (addr_t)(ptr = physical_alloc(0x1000, 0)) >= DMA_MAX);
    physical_free(ptr, 0x1000);
    ASSERT("Large physical allocations return NULL",
           physical_alloc(0xFFFFFFFFFFFFFFFF, 0) == NULL);
    FINISH_TEST();
    return 0;
}
TEST_INITCALL(test_physical);
