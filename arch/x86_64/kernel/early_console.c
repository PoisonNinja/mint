#include <arch/drivers/io.h>
#include <kernel/console.h>
#include <lib/printf.h>

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
    return 0;
}
