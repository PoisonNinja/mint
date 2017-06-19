/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
    printk(INFO, "tsc: Calibrated to %lluKHz\n", lowest);
    tsc_clocksource.frequency = khz * 1000;
    clocksource_register(&tsc_clocksource);
    return 0;
}
EARLY_INITCALL(tsc_init);
