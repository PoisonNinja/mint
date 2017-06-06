#pragma once

#include <types.h>

extern time_t uptime;

static inline void uptime_increment(void)
{
    uptime++;
}
