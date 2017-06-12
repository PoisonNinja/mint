#include <arch/cpu/cpu.h>
#include <arch/drivers/io.h>
#include <kernel/init.h>
#include <kernel/time/time.h>

#define PIT_FREQ 1193182ul

#define TSC_CALIBRATE_MS 10
#define TSC_CALIBRATE_LATCH (PIT_FREQ / (1000 / TSC_CALIBRATE_MS))

/*
 * Results are in EAX and EDX for both 64-bit and 32-bit, but on 64-bit,
 * =A unfortunately means RAX, which means that it will wrap around
 * on 32-bit boundaries. Thus, we manually shift them together.
 */
#ifdef X86_64
static time_t tsc_read()
{
    unsigned int low, high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}
#else
/*
 * On 32-bit platforms, =A combines the two 32-bit values in EAX and EDX
 * into one 64-bit :)
 */
static time_t tsc_read(void)
{
    uint64_t ret;
    asm volatile("rdtsc" : "=A"(ret));
    return ret;
}
#endif

static struct clocksource tsc_clocksource = {
    .name = "tsc", .rating = 350, .read = tsc_read,
};

static time_t tsc_calibrate(uint64_t ms)
{
    uint64_t t1, t2;
    outb(0x43, 0xB0);  // 1011 0000 - Channel 2, low and high byte, mode 0
    outb(0x42, TSC_CALIBRATE_LATCH & 0xFF);
    outb(0x42, TSC_CALIBRATE_LATCH >> 8);
    t1 = tsc_read();
    while ((inb(0x61) & 0x20) == 0)
        ;
    t2 = tsc_read();
    uint64_t diff = t2 - t1;
    return diff / ms;
}

static int tsc_init(void)
{
    time_t khz;
    time_t lowest = UINT64_MAX;
    for (int i = 0; i < 5; i++)
        if ((khz = tsc_calibrate(TSC_CALIBRATE_MS)) < lowest)
            lowest = khz;
    printk(INFO, "tsc: Calibrated to %luMHz\n", lowest / 1000);
    clocksource_calculate(&tsc_clocksource.mult, &tsc_clocksource.shift, lowest,
                          NSEC_PER_MSEC, 0);
    printk(INFO, "tsc: Setting mult to %u and shift to %u\n",
           tsc_clocksource.mult, tsc_clocksource.shift);
    clocksource_register(&tsc_clocksource);
    return 0;
}
EARLY_INITCALL(tsc_init);
