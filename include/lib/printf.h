#pragma once

#include <stdarg.h>
#include <types.h>

extern int vsprintf(char* s, const char* format, va_list arg);
extern int sprintf(char* s, const char* format, ...);
