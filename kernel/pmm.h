// RaeenOS Physical Memory Manager (PMM)

#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PMM_FRAME_SIZE 4096

// GRUB memory map entry structure
typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) mmap_entry_t;

// Initialize the physical memory manager from a GRUB memory map
// mmap_addr: The address of the memory map
// mmap_length: The length of the memory map
void pmm_init_from_mmap(uint32_t mmap_addr, uint32_t mmap_length);

// Allocate a single 4KB frame of physical memory
// Returns the physical address of the allocated frame, or 0 if no frames are free
void* pmm_alloc_frame(void);

// Free a previously allocated physical memory frame
// frame_addr: The physical address of the frame to free
void pmm_free_frame(void* frame_addr);

extern uint32_t highest_address;

#endif // PMM_H
