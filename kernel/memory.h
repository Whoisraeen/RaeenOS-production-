#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h> // For size_t
#include <stdint.h>
#include <stdbool.h>

// Initialize the kernel heap
void memory_init(void);

// Kernel memory allocation
void* kmalloc(size_t size);
void kfree(void* ptr);

// Aliases for application compatibility
void* memory_alloc(size_t size);
void memory_free(void* ptr);

// Placeholder for memory protection functions
bool memory_protect_range(uintptr_t addr, size_t size, uint32_t flags);
bool memory_unprotect_range(uintptr_t addr, size_t size);

// ASLR (Address Space Layout Randomization)
void memory_enable_aslr(void);
uintptr_t memory_get_random_offset(uintptr_t range);

#endif
