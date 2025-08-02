#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h> // For size_t

// Initialize the kernel heap
void memory_init(void);

// Kernel memory allocation
void* kalloc(size_t size);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
