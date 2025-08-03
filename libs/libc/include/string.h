// RaeenOS Kernel-Level String Utilities
// -------------------------------------
// This file provides basic string manipulation functions required by the kernel.
// These are implemented from scratch to avoid dependencies on a standard C library.

#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#include "../../../kernel/include/types.h"

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

// Additional RaeenOS string functions
void* memory_set(void* s, int c, size_t n);
void string_copy(char* dest, const char* src, size_t dest_size);
int string_compare(const char* s1, const char* s2);
char* string_find_last(char* str, char c);
bool string_ends_with(const char* str, const char* suffix);
char* string_duplicate(const char* str);
void string_format(char* dest, size_t dest_size, const char* format, ...);
void uint64_to_string(uint64_t value, char* buffer, size_t buffer_size);

#endif // KERNEL_STRING_H
