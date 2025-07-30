#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>
#include <stdint.h>

// Basic string functions
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);

// Basic I/O functions (wrappers for syscalls)
int printf(const char* format, ...);
int puts(const char* s);
int getchar(void);

// Memory allocation (wrappers for syscalls)
void* malloc(size_t size);
void free(void* ptr);

#endif // LIBC_H
