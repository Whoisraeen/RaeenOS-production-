// RaeenOS Kernel-Level String Utilities
// -------------------------------------
// This file provides basic string manipulation functions required by the kernel.
// These are implemented from scratch to avoid dependencies on a standard C library.

#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#include "include/types.h"

size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strtok_r(char* str, const char* delim, char** saveptr);
size_t strspn(const char* s, const char* accept);
size_t strcspn(const char* s, const char* reject);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);

#endif // KERNEL_STRING_H
