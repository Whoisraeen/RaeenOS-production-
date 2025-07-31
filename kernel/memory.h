// Memory management interface for RaeenOS

#ifndef MEMORY_H
#define MEMORY_H

#include "include/types.h"

// Initialize the kernel heap
void memory_init(void);

// Allocate a block of memory from the kernel heap
void* kmalloc(size_t size);

// Free a block of memory back to the kernel heap
void kfree(void* ptr);

#endif
