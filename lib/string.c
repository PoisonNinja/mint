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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

void *memchr(const void *str, int c, size_t n)
{
    const uint8_t *d = str;
    while (n--) {
        if (*d == (uint8_t)c)
            return (void *)d;
        d++;
    }
    return NULL;
}

int memcmp(const void *str1, const void *str2, size_t n)
{
    const uint8_t *a = str1, *b = str2;
    while (n--) {
        if (*a != *b)
            return *a - *b;
        a++, b++;
    }
    return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *a = dest;
    const uint8_t *b = src;
    while (n--) {
        *a++ = *b++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    if (dest < src)
        return memcpy(dest, src, n);
    else {
        uint8_t *a = dest + n - 1;
        const uint8_t *b = src + n - 1;
        while (n--) {
            *a-- = *b--;
        }
        return dest;
    }
}

void *__attribute__((weak)) memset(void *str, int c, size_t n)
{
    uint8_t *a = str;
    while (n--) {
        *a++ = (uint8_t)c;
    }
    return str;
}

char *strcat(char *dest, const char *src)
{
    char *end = dest;
    while (*end != '\0')
        end++;
    while (*src != '\0')
        *end++ = *src++;
    return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *end = dest;
    while (*end != '\0')
        end++;
    while (n--) {
        if (!(*end++ = *src++))
            return dest;
    }
    *end = '\0';
    return dest;
}

char *strchr(const char *str, int c)
{
    // The C standard is weird
    char *ret = (char *)str;
    while (*ret != (char)c) {
        if (*ret++ == '\0')
            return NULL;
    }
    return ret;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
        str1++, str2++;
    return *str1 - *str2;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    while (n-- && *str1) {
        if (*str1 != *str2)
            return *str1 - *str2;
        str1++, str2++;
    }
    return 0;
}

char *strcpy(char *dest, const char *src)
{
    for (; *src != '\0'; dest++, src++)
        *dest = *src;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    while (n-- && *src) {
        *dest++ = *src++;
    }
    while (n--) {
        *dest++ = 0;
    }
    return dest;
}

size_t strlen(const char *str)
{
    size_t n = 0;
    while (*str++ != '\0')
        n++;
    return n;
}

char *strrchr(const char *str, int c)
{
    char *last = NULL;
    while (*str++ != '\0') {
        if (*str == c)
            last = (char *)str;
    }
    return last;
}

char *strstr(const char *haystack, const char *needle)
{
    size_t n = strlen(needle);
    while (*haystack)
        if (!memcmp(haystack++, needle, n))
            return (char *)haystack - 1;
    return 0;
}
