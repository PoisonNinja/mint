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
    while (length--) {
        if (*string == '\n') {
            x = 0;
            y++;
            string++;
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
            }
            string += 2;
            length -= 2;
        } else {
            size_t index = y * VGA_WIDTH + x++;
            video[index] = *string++ | (color << 8);
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
