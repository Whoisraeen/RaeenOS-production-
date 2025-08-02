#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

// Basic printf
int printf(const char* format, ...);

// Basic puts
int puts(const char* str);

// Basic putchar
int putchar(int c);

// Basic sprintf
int sprintf(char* str, const char* format, ...);

#endif // _STDIO_H
