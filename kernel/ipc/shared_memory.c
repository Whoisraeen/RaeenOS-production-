#include "shared_memory.h"
#include "../memory.h"
#include "../string.h"
#include "../vga.h"
#include "../pmm.h"
#include "../paging.h"

static shared_memory_segment_t* shm_segments[32]; // Max 32 shared memory segments
static uint32_t next_shm_id = 0;

shared_memory_segment_t* shm_create(const char* name, size_t size) {
    if (next_shm_id >= 32) {
        debug_print("Shared Memory: Max segments reached.\n");
        return NULL;
    }

    shared_memory_segment_t* segment = (shared_memory_segment_t*)kmalloc(sizeof(shared_memory_segment_t));
    if (!segment) return NULL;

    memset(segment, 0, sizeof(shared_memory_segment_t));
    segment->id = next_shm_id++;
    strncpy(segment->name, name, sizeof(segment->name) - 1);
    segment->size = size;
    segment->ref_count = 0;

    // Allocate physical memory for the shared segment
    // For simplicity, allocate contiguous frames for now
    size_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    segment->physical_address = (uintptr_t)pmm_alloc_frames(num_pages);
    if (segment->physical_address == 0) {
        kfree(segment);
        debug_print("Shared Memory: Failed to allocate physical memory.\n");
        return NULL;
    }

    shm_segments[segment->id] = segment;

    debug_print("Shared Memory: Created segment ");
    debug_print(name);
    debug_print(" (ID: ");
    vga_put_dec(segment->id);
    debug_print(", Size: ");
    vga_put_dec(size);
    debug_print(" bytes)\n");

    return segment;
}

shared_memory_segment_t* shm_get(const char* name, uint32_t id) {
    for (uint32_t i = 0; i < next_shm_id; i++) {
        if (shm_segments[i] && 
            ((name && strcmp(shm_segments[i]->name, name) == 0) || 
             (id != 0 && shm_segments[i]->id == id))) {
            return shm_segments[i];
        }
    }
    return NULL;
}

void* shm_attach(shared_memory_segment_t* segment) {
    if (!segment) return NULL;

    // Map the physical memory of the shared segment into the current process's virtual address space
    // For simplicity, map it at a fixed high address for now
    uintptr_t vaddr = 0x80000000; // Example high virtual address
    size_t num_pages = (segment->size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t i = 0; i < num_pages; i++) {
        vmm_map_page(vmm_get_current_address_space(), 
                     vaddr + i * PAGE_SIZE, 
                     segment->physical_address + i * PAGE_SIZE, 
                     PTE_PRESENT | PTE_WRITE | PTE_USER);
    }

    segment->ref_count++;
    debug_print("Shared Memory: Attached segment ");
    debug_print(segment->name);
    debug_print(" to VAddr ");
    vga_put_hex(vaddr);
    debug_print("\n");

    return (void*)vaddr;
}

int shm_detach(shared_memory_segment_t* segment) {
    if (!segment) return -1;

    // Unmap the shared memory from the current process's address space
    uintptr_t vaddr = 0x80000000; // Must match attach address
    size_t num_pages = (segment->size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t i = 0; i < num_pages; i++) {
        vmm_unmap_page(vmm_get_current_address_space(), vaddr + i * PAGE_SIZE);
    }

    segment->ref_count--;
    debug_print("Shared Memory: Detached segment ");
    debug_print(segment->name);
    debug_print("\n");

    return 0;
}

int shm_destroy(shared_memory_segment_t* segment) {
    if (!segment) return -1;

    if (segment->ref_count > 0) {
        debug_print("Shared Memory: Cannot destroy segment with active attachments.\n");
        return -1; // Still in use
    }

    // Free physical memory
    size_t num_pages = (segment->size + PAGE_SIZE - 1) / PAGE_SIZE;
    pmm_free_frames((void*)segment->physical_address, num_pages);

    shm_segments[segment->id] = NULL;
    kfree(segment);

    debug_print("Shared Memory: Destroyed segment ");
    vga_put_dec(segment->id);
    debug_print("\n");

    return 0;
}
