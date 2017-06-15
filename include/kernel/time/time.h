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

#include <lib/list.h>
#include <types.h>

#define HZ 1000

extern time_t jiffy;

#define NSEC_PER_SEC 1000000000L
#define NSEC_PER_MSEC 1000000L

static inline void jiffy_increase(void)
{
    jiffy++;
}

struct clocksource {
    char* name;
    time_t (*read)(void);
    void (*enable)(void);
    uint32_t rating;
    uint32_t mult;
    uint32_t shift;
    struct clocksource *next, *prev;
};

extern void clocksource_calculate(uint32_t* mult, uint32_t* shift,
                                  uint32_t from, uint32_t to, uint32_t max);
extern void clocksource_register(struct clocksource* cs);
extern struct clocksource* clocksource_get(void);
extern struct clocksource* clocksource_get_default(void);

struct timesource {
    struct clocksource* clock;
    time_t (*read)(void);
    uint32_t mult;
    uint32_t shift;
    time_t last;
};

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct time {
    struct timesource ts;
    struct timespec raw_time;
};

extern void time_update(void);
extern void time_update_clocksource(struct clocksource* cs);
extern void time_init(void);

extern void tick_handler(void);

extern time_t ktime_get(void);
