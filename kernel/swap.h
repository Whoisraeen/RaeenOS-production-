#ifndef SWAP_H
#define SWAP_H

#include "include/types.h"
// Using types.h for kernel build

// Initialize the swapping mechanism
void swap_init(void);

// Swap out a page from physical memory to disk
bool swap_out_page(uint32_t physical_address);

// Swap in a page from disk to physical memory
bool swap_in_page(uint32_t physical_address);

#endif // SWAP_H
