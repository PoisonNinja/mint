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
#include <lib/printf.h>
#include <string.h>

struct parameters {
    char *buffer;
    uint8_t base;
    uint8_t uppercase;
    bool is_negative;
};

struct printf_data {
    char *buffer;
    size_t max;
    size_t used;
    void (*putcf)(struct printf_data *, char);
};

static void snprintf_putcf(struct printf_data *data, char c)
{
    if (data->used < data->max)
        data->buffer[data->used++] = c;
}

static void sprintf_putcf(struct printf_data *data, char c)
{
    data->buffer[data->used++] = c;
}

static void puts(struct printf_data *data, char *message)
{
    while (*message) {
        data->putcf(data, *message++);
    }
}

static void ull2a(unsigned long long num, struct parameters *params)
{
    char *buffer = params->buffer;
    if (params->is_negative)
        *buffer++ = '-';
    int n = 0;
    unsigned long long d = 1;
    while (num / d >= params->base)
        d *= params->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= params->base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ =
                dgt + (dgt < 10 ? '0' : (params->uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

static void ll2a(int num, struct parameters *p)
{
    if (num < 0) {
        num = -num;
        p->is_negative = true;
    }
    ull2a(num, p);
}

static void ul2a(unsigned long num, struct parameters *params)
{
    char *buffer = params->buffer;
    if (params->is_negative)
        *buffer++ = '-';
    int n = 0;
    unsigned long d = 1;
    while (num / d >= params->base)
        d *= params->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= params->base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ =
                dgt + (dgt < 10 ? '0' : (params->uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

static void l2a(int num, struct parameters *p)
{
    if (num < 0) {
        num = -num;
        p->is_negative = true;
    }
    ul2a(num, p);
}

static void ui2a(unsigned int num, struct parameters *params)
{
    char *buffer = params->buffer;
    if (params->is_negative)
        *buffer++ = '-';
    int n = 0;
    unsigned int d = 1;
    while (num / d >= params->base)
        d *= params->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= params->base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ =
                dgt + (dgt < 10 ? '0' : (params->uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

static void i2a(int num, struct parameters *p)
{
    if (num < 0) {
        num = -num;
        p->is_negative = true;
    }
    ui2a(num, p);
}

static void printf_format(struct printf_data *data, const char *format,
                          va_list arg)
{
    struct parameters params;
    char buffer[65];
    params.buffer = buffer;
    const char *pos = format;
    while (*pos != '\0') {
        if (*pos == '%') {
            uint8_t lng = 0;
            pos++;
            if (*pos == 'l') {
                lng++;
                pos++;
                if (*pos == 'l') {
                    lng++;
                    pos++;
                }
            }
            switch (*pos) {
                case 'c':
                    data->putcf(data, (char)va_arg(arg, int));
                    break;
                case 'd':
                    params.base = 10;
                    i2a(va_arg(arg, int), &params);
                    puts(data, params.buffer);
                    break;
                case 's':
                    puts(data, va_arg(arg, char *));
                    break;
                case 'X':
                    params.uppercase = 1;
                    __attribute__((fallthrough));
                case 'x':
                    params.base = 16;
                    if (lng == 0)
                        ui2a(va_arg(arg, unsigned int), &params);
                    else if (lng == 1)
                        ul2a(va_arg(arg, unsigned long), &params);
                    else if (lng == 2)
                        ull2a(va_arg(arg, unsigned long long), &params);
                    puts(data, params.buffer);
                    break;
            }
            pos++;
        } else {
            data->putcf(data, *pos++);
        }
    }
}

int vsprintf(char *s, const char *format, va_list arg)
{
    struct printf_data data = {
        .buffer = s, .putcf = sprintf_putcf,
    };
    printf_format(&data, format, arg);
    return data.used;
}

int sprintf(char *s, const char *format, ...)
{
    va_list args;
    int ret;
    va_start(args, format);
    ret = vsprintf(s, format, args);
    va_end(args);

    return ret;
}
