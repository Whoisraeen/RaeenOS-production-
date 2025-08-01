#ifndef PMM_H
#define PMM_H

#include "include/types.h"

#define PMM_FRAME_SIZE 4096

void pmm_init_from_mmap(uint64_t mmap_addr, uint32_t mmap_length);
void* pmm_alloc_frame(void);
void pmm_free_frame(void* frame_addr);

#endif // PMM_H