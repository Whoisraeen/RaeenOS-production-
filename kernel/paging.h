// RaeenOS Paging (Virtual Memory Manager)

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include "include/types.h"

#define PAGE_SIZE 4096

// Page Table Entry (PTE) flags
#define PTE_PRESENT     0x01
#define PTE_READ_WRITE  0x02
#define PTE_USER        0x04
#define PTE_WRITETHROUGH 0x08
#define PTE_CACHE_DISABLE 0x10
#define PTE_ACCESSED    0x20
#define PTE_DIRTY       0x40 // Only for page table entries
#define PTE_PAT         0x80 // Only for page table entries
#define PTE_GLOBAL      0x100 // Only for page table entries
// #define PTE_NX          0x80000000 // No Execute (for PAE/x64 systems)

// Page Directory Entry (PDE) flags
#define PDE_PRESENT     0x01
#define PDE_READ_WRITE  0x02
#define PDE_USER        0x04
#define PDE_WRITETHROUGH 0x08
#define PDE_CACHE_DISABLE 0x10
#define PDE_ACCESSED    0x20
#define PDE_PAGE_SIZE   0x80 // 0 for 4KB, 1 for 4MB
#define PDE_PAT         0x1000 // Only for page directory entries

// A single 4-byte entry in a page table or page directory
typedef uint32_t pte_t;
typedef uint32_t pde_t;

// A 4KB page table containing 1024 page table entries (PTEs)
// This structure must be 4KB aligned.
typedef struct {
    pte_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

// A 4KB page directory containing 1024 page directory entries (PDEs)
// This structure must be 4KB aligned.
typedef struct {
    pde_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

// Initialize the paging system
void paging_init(void);

// Returns a pointer to the kernel's page directory.
page_directory_t* paging_get_kernel_directory(void);

// Creates a new address space (page directory), cloning the kernel space mappings.
page_directory_t* paging_create_address_space(void);

// Maps a physical address to a virtual address in the given page directory.
void paging_map_page(page_directory_t* dir, void* virt, void* phys, bool is_user, bool is_writable);

// Maps a physical address to a virtual address in the given page directory for kernel use.
// These pages are always supervisor-only and can be read/write.
void paging_map_kernel_page(page_directory_t* dir, void* virt, void* phys, bool is_writable);

// Switches to the given page directory.
void paging_switch_directory(page_directory_t* dir);

// Returns the currently active page directory.
page_directory_t* paging_get_current_directory(void);

/**
 * @brief Clones a page directory, creating a deep copy of the address space.
 * 
 * @param src The source page directory to clone.
 * @return page_directory_t* A pointer to the new page directory, or NULL on failure.
 */
page_directory_t* paging_clone_directory(page_directory_t* src);

/**
 * @brief Frees all memory associated with a page directory.
 * 
 * This includes all user-space page tables and the physical frames they map to.
 * It does not unmap kernel-space pages.
 * 
 * @param dir The page directory to free.
 */
void paging_free_directory(page_directory_t* dir);

// Checks if a given virtual address is a valid userspace pointer.
bool is_valid_userspace_ptr(void* addr, size_t size);

#endif // PAGING_H
