#include <lib/printf.h>
#include <string.h>

static char *merge_strings(char *target, char *src)
{
    while (*src) {
        *target++ = *src++;
    }
    return target;
}

static void ui2a(unsigned int num, char *buffer, unsigned int base,
                 int uppercase)
{
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ = dgt + (dgt < 10 ? '0' : (uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
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
                    ui2a(va_arg(arg, int), int_conv, 10, 0);
                    s = merge_strings(s, int_conv);
                    break;
                case 's':
                    s = merge_strings(s, va_arg(arg, char *));
                    break;
                case 'x':
                    ui2a(va_arg(arg, int), int_conv, 16, 0);
                    s = merge_strings(s, int_conv);
                    break;
                case 'X':
                    ui2a(va_arg(arg, int), int_conv, 16, 1);
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
