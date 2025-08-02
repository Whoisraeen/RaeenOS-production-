#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Shared memory segment structure
typedef struct shared_memory_segment {
    uint32_t id;
    char name[64];
    size_t size;
    uintptr_t physical_address;
    uint32_t ref_count;
    // Add access control/permissions
} shared_memory_segment_t;

// Create a new shared memory segment
shared_memory_segment_t* shm_create(const char* name, size_t size);

// Get an existing shared memory segment by name or ID
shared_memory_segment_t* shm_get(const char* name, uint32_t id);

// Attach a shared memory segment to a process's address space
void* shm_attach(shared_memory_segment_t* segment);

// Detach a shared memory segment from a process's address space
int shm_detach(shared_memory_segment_t* segment);

// Destroy a shared memory segment
int shm_destroy(shared_memory_segment_t* segment);

#endif // SHARED_MEMORY_H
