// Physical Memory Manager Stubs for RaeenOS
// These are minimal implementations to get the kernel linking

#include <stdint.h>
#include <stddef.h>

// Memory tracking (very simple for now)
static uint64_t next_free_page = 0x200000; // Start at 2MB
static uint64_t memory_end = 0x40000000;   // Assume 1GB for now

// Allocate physical pages
uint64_t pmm_alloc_pages(size_t count) {
    if (count == 0) return 0;
    
    uint64_t page_size = 4096; // 4KB pages
    uint64_t allocation = next_free_page;
    
    // Simple bump allocator
    next_free_page += count * page_size;
    
    // Check if we've run out of memory
    if (next_free_page >= memory_end) {
        // Reset for simplicity (this is just a stub)
        next_free_page = 0x200000;
        return 0;
    }
    
    return allocation;
}

// Free physical pages (stub - doesn't actually free for now)
void pmm_free_pages(uint64_t address, size_t count) {
    // TODO: Implement proper free list
    // For now, just ignore the free request
    (void)address;
    (void)count;
}

// Allocate a single frame (for compatibility)
uint64_t pmm_alloc_frame(void) {
    return pmm_alloc_pages(1);
}

// Free a single frame (for compatibility)
void pmm_free_frame(uint64_t address) {
    pmm_free_pages(address, 1);
}

// Initialize PMM (stub)
void pmm_init(void) {
    // Initialize the physical memory manager
    // For now, just ensure our static variables are set
    next_free_page = 0x200000; // Start at 2MB
    memory_end = 0x40000000;   // 1GB
}

// Get available memory (stub)
uint64_t pmm_get_available_memory(void) {
    return memory_end - next_free_page;
}

// Get used memory (stub)
uint64_t pmm_get_used_memory(void) {
    return next_free_page - 0x200000;
}