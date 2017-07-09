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

#pragma once

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
    uint32_t frequency;
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
    uint32_t frequency;
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
