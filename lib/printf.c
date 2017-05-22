#include <lib/printf.h>
#include <stdlib.h>
#include <string.h>

static char *merge_strings(char *target, char *src)
{
    while (*src) {
        *target++ = *src++;
    }
    return target;
}

int vsprintf(char *s, const char *format, va_list arg)
{
    const char *pos = format;
    while (*pos != '\0') {
        char int_conv[100];
        if (*pos == '%') {
            pos++;
            switch (*pos) {
                case 'd':
                    itoa(va_arg(arg, int), int_conv, 10);
                    s = merge_strings(s, int_conv);
                    break;
                case 's':
                    s = merge_strings(s, va_arg(arg, char *));
                    break;
                case 'x':
                    itoa(va_arg(arg, int), int_conv, 16);
                    s = merge_strings(s, int_conv);
                    break;
            }
        } else {
            *s++ = *pos;
        }
        pos++;
    }
    return 0;
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
