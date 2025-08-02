// RaeenOS Kernel Heap

#include "memory.h"
#include "pmm.h"
#include "paging.h"
#include "sync.h" // For spinlock
#include "../vga.h"

// A block of memory in the heap
typedef struct heap_block {
    struct heap_block* next;
    size_t size;
    bool free;
} heap_block_t;

static heap_block_t* heap_start = NULL;
static spinlock_t heap_lock = SPINLOCK_INIT;

void memory_init(void) {
    // Allocate a single page for the initial heap
    heap_start = (heap_block_t*)pmm_alloc_frame();
    if (!heap_start) {
        // PANIC! Cannot allocate frame for heap.
        return;
    }

    heap_start->next = NULL;
    heap_start->size = PMM_FRAME_SIZE - sizeof(heap_block_t);
    heap_start->free = true;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    spinlock_acquire(&heap_lock);

    // Align the size to the size of a heap_block_t
    if (size % sizeof(heap_block_t) != 0) {
        size += sizeof(heap_block_t) - (size % sizeof(heap_block_t));
    }

    heap_block_t* current = heap_start;
    while (current) {
        if (current->free && current->size >= size) {
            // Found a free block that is large enough
            if (current->size > size + sizeof(heap_block_t)) {
                // Split the block
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)current + sizeof(heap_block_t) + size);
                new_block->next = current->next;
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->free = true;

                current->next = new_block;
                current->size = size;
            }

            current->free = false;
            spinlock_release(&heap_lock);
            return (void*)((uint8_t*)current + sizeof(heap_block_t));
        }

        current = current->next;
    }

    // Expand the heap
    heap_block_t* new_page = (heap_block_t*)pmm_alloc_frame();
    if (!new_page) {
        spinlock_release(&heap_lock);
        return NULL; // Out of memory
    }

    new_page->next = NULL;
    new_page->size = PMM_FRAME_SIZE - sizeof(heap_block_t);
    new_page->free = true;

    // Add the new page to the end of the heap
    current = heap_start;
    while (current->next) {
        current = current->next;
    }
    current->next = new_page;

    // Try to allocate again
    void* allocated_ptr = kmalloc(size); // Recursive call, but will eventually find space or fail
    spinlock_release(&heap_lock);
    return allocated_ptr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    spinlock_acquire(&heap_lock);

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->free = true;

    // Coalesce free blocks
    heap_block_t* current = heap_start;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(heap_block_t);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }

    spinlock_release(&heap_lock);
}

// Placeholder for memory protection functions
bool memory_protect_range(uintptr_t addr, size_t size, uint32_t flags) {
    debug_print("Memory: Protecting range (placeholder).");
    // In a real implementation, this would modify page table entries.
    (void)addr;
    (void)size;
    (void)flags;
    return false; // Not implemented
}

bool memory_unprotect_range(uintptr_t addr, size_t size) {
    debug_print("Memory: Unprotecting range (placeholder).");
    // In a real implementation, this would modify page table entries.
    (void)addr;
    (void)size;
    return false; // Not implemented
}

// Placeholder for ASLR (Address Space Layout Randomization)
void memory_enable_aslr(void) {    debug_print("Memory: Enabling ASLR (basic implementation).");    // In a real implementation, this would involve randomizing base addresses    // for kernel and userland components during boot/load time.    // For now, this function serves as an entry point.}uintptr_t memory_get_random_offset(uintptr_t range) {    // Very basic pseudo-random number generator for demonstration.    // In a real system, this would use a high-quality entropy source.    static uint32_t seed = 123456789; // Initial seed    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF; // LCG    return (uintptr_t)(seed % range);}