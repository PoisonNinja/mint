#include <cpu/interrupt.h>
#include <kernel/console.h>
#include <lib/printf.h>

#define PANIC_BUFFER_SIZE 1024

void __attribute__((noreturn)) panic(const char *format, ...)
{
    interrupt_disable();
    char buffer[PANIC_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, PANIC_BUFFER_SIZE, format, args);
    va_end(args);
    console_write(buffer, 1024);
    for (;;)
        ;
}
