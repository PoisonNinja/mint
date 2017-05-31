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
#include <lib/printf.h>
#include <string.h>

int _printk(int level, const char* format, ...)
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    va_list args;
    int r;
    va_start(args, format);
    r = vsprintf(buffer, format, args);
    va_end(args);
    console_write(buffer, strlen(buffer));
    return r;
}
