#include <kernel/time/time.h>

LIST_HEAD(clocksource_list);
struct clocksource* current_clocksource = NULL;

void clocksource_calculate(uint32_t* mult, uint32_t* shift, uint32_t to,
                           uint32_t from, uint32_t max)
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
