// RaeenOS Physical Memory Manager (PMM)

#include "pmm.h"
#include "vga.h" // For debugging output

static uint32_t* pmm_bitmap = NULL;
static size_t pmm_total_frames = 0;
static size_t pmm_bitmap_size_in_dwords = 0;
uint32_t highest_address = 0;

// Helper to set a bit in the bitmap (mark as used)
static void pmm_set_bit(size_t bit) {
    if (pmm_bitmap) {
        pmm_bitmap[bit / 32] |= (1 << (bit % 32));
    }
}

// Helper to clear a bit in the bitmap (mark as free)
static void pmm_clear_bit(size_t bit) {
    if (pmm_bitmap) {
        pmm_bitmap[bit / 32] &= ~(1 << (bit % 32));
    }
}

// Helper to test if a bit is set
static int pmm_test_bit(size_t bit) {
    if (pmm_bitmap) {
        return pmm_bitmap[bit / 32] & (1 << (bit % 32));
    }
    return 1; // Assume used if bitmap not initialized
}

// Find the first free frame in the bitmap
static int pmm_find_first_free() {
    if (!pmm_bitmap) return -1;

    for (size_t i = 0; i < pmm_bitmap_size_in_dwords; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) { // If not all frames in this dword are used
            for (int j = 0; j < 32; j++) {
                if (!(pmm_bitmap[i] & (1 << j))) {
                    size_t frame_index = i * 32 + j;
                    if (frame_index < pmm_total_frames) {
                        return frame_index;
                    }
                }
            }
        }
    }
    return -1; // No free frames found
}

void pmm_init_from_mmap(uint32_t mmap_addr, uint32_t mmap_length) {
    mmap_entry_t* mmap = (mmap_entry_t*)mmap_addr;
    mmap_entry_t* mmap_end = (mmap_entry_t*)(mmap_addr + mmap_length);

    // First, find the highest address to determine the total memory size
    highest_address = 0;
    for (mmap_entry_t* entry = mmap; entry < mmap_end; entry++) {
        if (entry->type == 1) { // Available RAM
            uint32_t top = entry->addr + entry->len;
            if (top > highest_address) {
                highest_address = top;
            }
        }
    }

    pmm_total_frames = highest_address / PMM_FRAME_SIZE;
    size_t pmm_bitmap_size_in_bytes = pmm_total_frames / 8;
    pmm_bitmap_size_in_dwords = pmm_bitmap_size_in_bytes / 4;
    if (pmm_bitmap_size_in_bytes % 4) pmm_bitmap_size_in_dwords++;


    // Now, find a large enough free memory region to store the bitmap
    for (mmap_entry_t* entry = mmap; entry < mmap_end; entry++) {
        if (entry->type == 1 && entry->len >= pmm_bitmap_size_in_bytes) {
            // We found a place for the bitmap!
            pmm_bitmap = (uint32_t*)entry->addr;
            break;
        }
    }

    if (!pmm_bitmap) {
        // This is a fatal error, we can't initialize the PMM
        // In a real OS, we would panic here. For now, we'll just hang.
        vga_clear();
        vga_print_string("PMM: No suitable memory region found for bitmap!");
        for(;;);
    }

    // Initially, mark all memory as used
    for (size_t i = 0; i < pmm_bitmap_size_in_dwords; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    // Now, iterate through the memory map again and mark available regions as free
    for (mmap_entry_t* entry = mmap; entry < mmap_end; entry++) {
        if (entry->type == 1) { // Available RAM
            uint32_t start_frame = entry->addr / PMM_FRAME_SIZE;
            uint32_t num_frames = entry->len / PMM_FRAME_SIZE;

            for (uint32_t i = 0; i < num_frames; i++) {
                pmm_clear_bit(start_frame + i);
            }
        }
    }

    // Finally, mark the bitmap itself as used
    size_t bitmap_frames = pmm_bitmap_size_in_bytes / PMM_FRAME_SIZE;
    if (pmm_bitmap_size_in_bytes % PMM_FRAME_SIZE) bitmap_frames++;

    size_t bitmap_start_frame = (uint32_t)pmm_bitmap / PMM_FRAME_SIZE;
    for (size_t i = 0; i < bitmap_frames; i++) {
        pmm_set_bit(bitmap_start_frame + i);
    }

    // Also reserve the first 1MB for the kernel and other legacy stuff
    for (size_t i = 0; i < 256; i++) { // 1MB / 4KB = 256 frames
        pmm_set_bit(i);
    }
}


void* pmm_alloc_frame() {
    int frame_index = pmm_find_first_free();
    if (frame_index == -1) {
        return NULL; // Out of memory
    }

    pmm_set_bit(frame_index);
    return (void*)((uintptr_t)frame_index * PMM_FRAME_SIZE);
}

void pmm_free_frame(void* frame_addr) {
    size_t frame_index = (uintptr_t)frame_addr / PMM_FRAME_SIZE;
    pmm_clear_bit(frame_index);
}