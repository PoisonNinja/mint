#include <kernel/time/time.h>

struct clocksource* clocksource_list = NULL;
struct clocksource* current_clocksource = NULL;

void clocksource_calculate(uint32_t* mult, uint32_t* shift, uint32_t from,
                           uint32_t to, uint32_t max)
{
}

void clocksource_select()
{
    struct clocksource* cs = NULL;
    int cs_changed = 0;
    LIST_FOR_EACH(clocksource_list, cs)
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
    LIST_APPEND(clocksource_list, cs);
    printk(INFO, "clocksource: Registered clocksource %s with rating %d\n",
           cs->name, cs->rating);
    clocksource_select();
}

struct clocksource* clocksource_get(void)
{
    return current_clocksource;
}
