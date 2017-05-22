#include <stdlib.h>

static void reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

char *itoa(int value, char *buf, int base)
{
    char *chars = "0123456789abcdef";
    int i = 0;
    while (value > 0) {
        buf[i++] = chars[value % base];
        value /= base;
    }
    buf[i] = '\0';
    reverse(buf);
}
