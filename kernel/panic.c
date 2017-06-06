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

#include <cpu/interrupt.h>
#include <kernel/console.h>
#include <kernel/stacktrace.h>
#include <lib/printf.h>
#include <string.h>

#define PANIC_BUFFER_SIZE 1024

void __attribute__((noreturn))
_panic(const char *file, int line, const char *format, ...)
{
    interrupt_disable();
    char buffer[PANIC_BUFFER_SIZE];
    int r = sprintf(buffer, "Panic in file %s on line %d: ", file, line);
    console_write(buffer, r);
    memset(buffer, 0, r);
    va_list args;
    va_start(args, format);
    r = vsnprintf(buffer, PANIC_BUFFER_SIZE, format, args);
    va_end(args);
    console_write(buffer, r);
    stacktrace();
    for (;;)
        ;
}
