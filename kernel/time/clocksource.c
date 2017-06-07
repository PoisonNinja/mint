#include <kernel/time/time.h>

LIST_HEAD(clocksource_list);
struct clocksource* current_clocksource = NULL;

void clocksource_select()
{
    struct clocksource* cs = NULL;
    list_for_each_entry(cs, &clocksource_list, list)
    {
        if (current_clocksource) {
            if (cs->rating > current_clocksource->rating)
                current_clocksource = cs;
        } else
            current_clocksource = cs;
    }
    printk(INFO, "Setting %s as current clocksource\n",
           current_clocksource->name);
}

void clocksource_register(struct clocksource* cs)
{
    list_add_tail(&cs->list, &clocksource_list);
    printk(INFO, "Registered clocksource %s with rating %d\n", cs->name,
           cs->rating);
    clocksource_select();
}
