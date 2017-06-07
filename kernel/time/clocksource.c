#include <kernel/time/time.h>

LIST_HEAD(clocksource_list);
struct clocksource* current_clocksource = NULL;

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
