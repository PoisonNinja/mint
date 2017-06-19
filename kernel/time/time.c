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

#include <kernel/lock/spinlock.h>
#include <kernel/time/time.h>

time_t jiffy = 0;

static struct time kernel_time;
static spinlock_t kernel_time_lock = {0};

static inline time_t time_cycle_to_ns(uint64_t cycle, uint32_t frequency)
{
    return cycle * NSEC_PER_SEC / frequency;
}

time_t ktime_get(void)
{
    time_update();
    return (kernel_time.raw_time.tv_sec * NSEC_PER_SEC) +
           kernel_time.raw_time.tv_nsec;
}

void time_update(void)
{
    spinlock_lock(kernel_time_lock);
    if (!kernel_time.ts.clock) {
        spinlock_unlock(kernel_time_lock);
        return;
    }
    time_t now = kernel_time.ts.read();
    time_t offset = now - kernel_time.ts.last;
    kernel_time.ts.last = now;
    time_t nsec = time_cycle_to_ns(offset, kernel_time.ts.frequency);
    kernel_time.raw_time.tv_nsec += nsec;
    // Coalesce nanoseconds into seconds
    while (kernel_time.raw_time.tv_nsec >= NSEC_PER_SEC) {
        kernel_time.raw_time.tv_nsec -= NSEC_PER_SEC;
        kernel_time.raw_time.tv_sec++;
    }
    spinlock_unlock(kernel_time_lock);
}

void time_update_clocksource(struct clocksource* cs)
{
    kernel_time.ts.clock = cs;
    kernel_time.ts.read = cs->read;
    kernel_time.ts.last = cs->read();
    kernel_time.ts.frequency = cs->frequency;
}

void time_init(void)
{
    time_update_clocksource(clocksource_get_default());
    kernel_time.raw_time.tv_sec = 0;
    kernel_time.raw_time.tv_nsec = 0;
}
