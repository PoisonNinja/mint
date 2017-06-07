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

extern time_t uptime;

static inline void uptime_increment(void)
{
    uptime++;
}

struct clocksource {
    char* name;
    time_t (*read)(void);
    uint32_t rating;
    uint32_t mult;
    uint32_t shift;
    struct list_head list;
};

extern void clocksource_register(struct clocksource* cs);
