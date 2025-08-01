#ifndef PAGING_H
#define PAGING_H

#include "include/types.h"

#define PAGE_SIZE 4096

// Page table entry flags for x86-64
#define PTE_PRESENT     (1ULL << 0)
#define PTE_WRITE       (1ULL << 1)
#define PTE_USER        (1ULL << 2)
#define PTE_WRITETHROUGH (1ULL << 3)
#define PTE_CACHE_DISABLE (1ULL << 4)
#define PTE_ACCESSED    (1ULL << 5)
#define PTE_DIRTY       (1ULL << 6)
#define PTE_PAT         (1ULL << 7)
#define PTE_GLOBAL      (1ULL << 8)
#define PTE_NO_EXECUTE  (1ULL << 63)

// Page table levels
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

// Page table structures
typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(PAGE_SIZE))) pml4_t;

typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(PAGE_SIZE))) pdpt_t;

typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(PAGE_SIZE))) pd_t;

typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(PAGE_SIZE))) pt_t;

// Virtual Memory Manager functions
void vmm_init(void);
void vmm_map_page(pml4_t* pml4, uintptr_t vaddr, uintptr_t paddr, uint64_t flags);
void vmm_unmap_page(pml4_t* pml4, uintptr_t vaddr);
uintptr_t vmm_get_physical_address(pml4_t* pml4, uintptr_t vaddr);
pml4_t* vmm_create_address_space(void);
void vmm_switch_address_space(pml4_t* pml4);
pml4_t* vmm_get_current_address_space(void);

#endif // PAGING_H