#include <kernel/time/time.h>

static time_t jiffy_read(void)
{
    return jiffy;
}

static struct clocksource jiffy_clocksource = {
    .name = "jiffy",
    .rating = 1,
    .read = jiffy_read,
    .mult = 0xF424000,
    .shift = 8,
};

struct clocksource* __attribute__((weak)) clocksource_get_default(void)
{
    return &jiffy_clocksource;
}
