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

#include <kernel/lock/spinlock.h>
#include <kernel/time/time.h>

time_t jiffy = 0;

static struct time kernel_time;
static spinlock_t kernel_time_lock = {0};

static time_t time_cycle_to_ns(uint64_t cycle, uint32_t mult, uint32_t shift)
{
    return ((cycle * mult) >> shift);
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
    time_t nsec =
        time_cycle_to_ns(offset, kernel_time.ts.mult, kernel_time.ts.shift);
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
    kernel_time.ts.mult = cs->mult;
    kernel_time.ts.shift = cs->shift;
}

void time_init(void)
{
    time_update_clocksource(clocksource_get_default());
    kernel_time.raw_time.tv_sec = 0;
    kernel_time.raw_time.tv_nsec = 0;
}
