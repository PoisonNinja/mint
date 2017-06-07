/*
 * Copyright (C) 2017 Jason Lu (PoisonNinja)
 *
 * This file is part of Mint. Mint is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <kernel.h>
#include <kernel/console.h>
#include <kernel/time/time.h>
#include <lib/printf.h>
#include <string.h>

static char* colors[] = {
    "\e[36m",  // Blue for debug
    "\e[32m",  // Green for info
    "\e[33m",  // Yellow for warning
    "\e[31m",  // Red for error
};

int _printk(int level, const char* format, ...)
{
    int r;
    char buffer[1024];
    memset(buffer, 0, 1024);
    time_t t = ktime_get();
    time_t sec = t / NSECS_PER_SECS;
    time_t nsec = t % NSECS_PER_SECS;
    r = sprintf(buffer, "%s[%05lu.%09lu] ", colors[level], sec, nsec);
    console_write(buffer, r);
    console_write("\e[39m", 6);
    memset(buffer, 0, 1024);
    va_list args;
    va_start(args, format);
    r = vsprintf(buffer, format, args);
    va_end(args);
    console_write(buffer, r);
    return r;
}
