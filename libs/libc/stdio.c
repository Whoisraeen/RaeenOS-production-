#include "include/stdio.h"
#include "../kernel/vga.h"
#include "../kernel/string.h"

// Basic printf implementation
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[256]; // Small buffer for now
    int len = vsprintf(buffer, format, args);
    vga_puts(buffer);

    va_end(args);
    return len;
}

// Basic puts implementation
int puts(const char* str) {
    vga_puts(str);
    vga_puts("\n");
    return 0;
}

// Basic putchar implementation
int putchar(int c) {
    vga_put_char((char)c);
    return c;
}

// Basic sprintf implementation
int sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsprintf(str, format, args);
    va_end(args);
    return len;
}

