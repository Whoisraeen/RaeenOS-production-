#include "include/libc.h"
#include "include/syscall.h"

// Basic string functions
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    return s;
}

// Basic I/O functions (wrappers for syscalls)
int printf(const char* format, ...) {
    // Very basic printf for now
    return write(STDOUT_FILENO, format, strlen(format));
}

int puts(const char* s) {
    write(STDOUT_FILENO, s, strlen(s));
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}

int getchar(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        return (int)c;
    }
    return -1; // EOF or error
}

// Memory allocation (wrappers for syscalls)
void* malloc(size_t size) {
    // TODO: Implement userland malloc using brk/sbrk syscalls
    return NULL;
}

void free(void* ptr) {
    // TODO: Implement userland free
    (void)ptr;
}
