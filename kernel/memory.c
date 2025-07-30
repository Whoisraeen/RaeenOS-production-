// RaeenOS Kernel Heap

#include "memory.h"
#include "pmm.h"
#include "paging.h"
#include <stdbool.h>

// A block of memory in the heap
typedef struct heap_block {
    struct heap_block* next;
    size_t size;
    bool free;
} heap_block_t;

static heap_block_t* heap_start = NULL;

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
            return (void*)((uint8_t*)current + sizeof(heap_block_t));
        }

        current = current->next;
    }

    // Expand the heap
    heap_block_t* new_page = (heap_block_t*)pmm_alloc_frame();
    if (!new_page) {
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
    return kmalloc(size);
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

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
}