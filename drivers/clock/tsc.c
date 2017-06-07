#include <kernel/init.h>
#include <kernel/time/time.h>

static time_t tsc_read(void)
{
    uint32_t high, low;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((time_t)low) | (((time_t)high) << 32);
}

static struct clocksource tsc_clocksource = {
    .name = "tsc", .rating = 350, .read = tsc_read,
};

static int tsc_init(void)
{
    clocksource_register(&tsc_clocksource);
    return 0;
}
CORE_INITCALL(tsc_init);
