#include <cpu/interrupt.h>
#include <kernel/console.h>
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
    for (;;)
        ;
}
