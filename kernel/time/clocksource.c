#include <kernel/time/time.h>

LIST_HEAD(clocksource_list);
struct clocksource* current_clocksource = NULL;

// Copied verbatim from the Linux kernel - To comply with licensing:
/*
 * Copyright (C) 2004, 2005 IBM, John Stultz (johnstul@us.ibm.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
void clocksource_calculate(uint32_t* mult, uint32_t* shift, uint32_t from,
                           uint32_t to, uint32_t max)
{
    uint64_t tmp;
    uint32_t sft, sftacc = 32;

    /*
     * Calculate the shift factor which is limiting the conversion
     * range:
     */
    tmp = ((uint64_t)max * from) >> 32;
    while (tmp) {
        tmp >>= 1;
        sftacc--;
    }

    /*
     * Find the conversion shift/mult pair which has the best
     * accuracy and fits the maxsec conversion range:
     */
    for (sft = 32; sft > 0; sft--) {
        tmp = (uint64_t)to << sft;
        tmp += from / 2;
        tmp /= from;
        if ((tmp >> sftacc) == 0)
            break;
    }
    *mult = tmp;
    *shift = sft;
}

void clocksource_select()
{
    struct clocksource* cs = NULL;
    int cs_changed = 0;
    list_for_each_entry(cs, &clocksource_list, list)
    {
        if (current_clocksource) {
            if (cs->rating > current_clocksource->rating)
                current_clocksource = cs;
            cs_changed = 1;
        } else {
            current_clocksource = cs;
            cs_changed = 1;
        }
    }
    if (cs_changed) {
        printk(INFO, "clocksource: Setting %s as current clocksource\n",
               current_clocksource->name);
        time_update_clocksource(current_clocksource);
    }
}

void clocksource_register(struct clocksource* cs)
{
    list_add_tail(&cs->list, &clocksource_list);
    printk(INFO, "clocksource: Registered clocksource %s with rating %d\n",
           cs->name, cs->rating);
    clocksource_select();
}

struct clocksource* clocksource_get(void)
{
    return current_clocksource;
}
