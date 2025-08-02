#ifndef SWAP_MANAGER_H
#define SWAP_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Initialize the swap manager
void swap_manager_init(void);

// Add a swap device (e.g., a partition or file)
int swap_manager_add_device(const char* device_path, size_t size_bytes);

// Remove a swap device
int swap_manager_remove_device(const char* device_path);

// Swap out a page from physical memory to disk
int swap_out_page(uintptr_t physical_address);

// Swap in a page from disk to physical memory
int swap_in_page(uintptr_t physical_address);

// Check if a page is currently swapped out
bool is_page_swapped(uintptr_t physical_address);

#endif // SWAP_MANAGER_H
