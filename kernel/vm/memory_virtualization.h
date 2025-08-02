#ifndef MEMORY_VIRTUALIZATION_H
#define MEMORY_VIRTUALIZATION_H

#include <stdint.h>
#include <stdbool.h>

// Initialize memory virtualization (e.g., enable EPT/NPT)
void mem_virt_init(void);

// Map guest physical address to host physical address (placeholder)
int mem_virt_map_guest_memory(uint64_t guest_paddr, uint64_t host_paddr, size_t size);

// Unmap guest physical address (placeholder)
int mem_virt_unmap_guest_memory(uint64_t guest_paddr, size_t size);

#endif // MEMORY_VIRTUALIZATION_H
