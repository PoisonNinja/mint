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

#include <arch/drivers/io.h>
#include <kernel/console.h>
#include <lib/printf.h>
#include <string.h>

#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static int x = 0;
static int y = 0;
static volatile uint16_t *video = (volatile uint16_t *)VGA_BUFFER;
static int color = 15;

static void x86_64_console_vga_update_cursor(int col, int row)
{
    unsigned short position = (row * VGA_WIDTH) + col;

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

static void x86_64_console_vga_write_string(const char *string, size_t length)
{
    while (length) {
        if (*string == '\n') {
            x = 0;
            y++;
            string++;
            length--;
        } else if (*string == '\e') {
            string += 3;
            length -= 3;
            switch (*string) {
                case '1':
                    color = 12;  // Light Red
                    break;
                case '2':
                    color = 10;  // Light Green
                    break;
                case '3':
                    color = 14;  // Yellow
                    break;
                case '6':
                    color = 11;  // Light Blue
                    break;
                case '9':
                    color = 15;  // Reset to white
                    break;
                default:
                    color = 15;
                    break;
            }
            string += 2;
            length -= 2;
        } else {
            size_t index = y * VGA_WIDTH + x++;
            video[index] = *string++ | (color << 8);
            length--;
        }
        if (x == VGA_WIDTH) {
            x = 0;
            y++;
        }
        if (y == VGA_HEIGHT) {
            for (int ny = 1; ny < VGA_HEIGHT; ny++) {
                memcpy((void *)&video[(ny - 1) * VGA_WIDTH],
                       (void *)&video[ny * VGA_WIDTH], 2 * VGA_WIDTH);
            }
            memset((void *)&video[(VGA_HEIGHT - 1) * VGA_WIDTH], 0,
                   2 * VGA_WIDTH);
            y = VGA_HEIGHT - 1;
        }
    }
    x86_64_console_vga_update_cursor(x, y);
}

static int x86_64_console_vga_write(const char *message, size_t size)
{
    x86_64_console_vga_write_string(message, size);
    return size;
}

static struct console x86_64_console_vga = {
    .name = "x86_64 vga", .write = x86_64_console_vga_write,
};

// Serial port
#define SERIAL_PORT 0x3F8

static int x86_64_console_serial_write(const char *message, size_t size)
{
    int i = 0;
    while (size--) {
        outb(SERIAL_PORT, message[i++]);
    }
    return i;
}

static struct console x86_64_console_serial = {
    .name = "x86_64 serial", .write = x86_64_console_serial_write,
};

int x86_64_init_console(void)
{
    console_register(&x86_64_console_serial);
    console_register(&x86_64_console_vga);
    return 0;
}
