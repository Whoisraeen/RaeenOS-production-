#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

// Basic malloc/free (will map to kmalloc/kfree for now)
void* malloc(size_t size);
void free(void* ptr);

// Basic exit
void exit(int status);

// Basic atoi
int atoi(const char* str);

#endif // _STDLIB_H
