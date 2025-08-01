#pragma once

#include <stddef.h> // For size_t

// Kernel memory allocation
void* kalloc(size_t size);

// Initialize the kernel heap
void memory_init(void);

// Allocate a block of memory from the kernel heap
void* kmalloc(size_t size);

{{ ... }}
void kfree(void* ptr);

#endif
