#include <kernel/time/time.h>

static time_t jiffy_read(void)
{
    return jiffy;
}

static struct clocksource jiffy_clocksource = {
    .name = "jiffy", .rating = 1, .read = jiffy_read, .frequency = 1000,
};

struct clocksource* __attribute__((weak)) clocksource_get_default(void)
{
    // Needed when no clocksource is registered
    return &jiffy_clocksource;
}
