#include <kernel.h>
#include <lib/printf.h>

int _printk(int level, const char* format, ...)
{
    char buffer[1024];
    va_list args;
    int r;
    va_start(args, format);
    r = vsprintf(buffer, format, args);
    va_end(args);
    return r;
}
