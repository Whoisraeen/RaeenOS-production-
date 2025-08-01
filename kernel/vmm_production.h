#ifndef VMM_PRODUCTION_H
#define VMM_PRODUCTION_H

/**
 * @file vmm_production.h
 * @brief Production-Grade Virtual Memory Manager for RaeenOS
 * 
 * This implements a comprehensive virtual memory manager with:
 * - 64-bit virtual address space management
 * - 4-level page table support (x86-64 standard)
 * - Copy-on-write (COW) implementation  
 * - Memory protection and access control
 * - Virtual memory areas (VMA) management
 * - Memory mapping (mmap/munmap) support
 * - Demand paging and lazy allocation
 * - Address space layout randomization (ASLR)
 * - Memory pressure handling and swapping
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "types.h"
#include "include/sync.h"
#include "pmm_production.h"

#ifdef __cplusplus
extern "C" {
#endif

// Virtual address space layout (x86-64)
#define VMM_USER_SPACE_START    0x0000000000000000ULL
#define VMM_USER_SPACE_END      0x00007FFFFFFFFFFFULL  // 128TB user space
#define VMM_KERNEL_SPACE_START  0xFFFF800000000000ULL  // -128TB
#define VMM_KERNEL_SPACE_END    0xFFFFFFFFFFFFFFFFULL

// Specific kernel areas
#define VMM_KERNEL_DIRECT_MAP   0xFFFF800000000000ULL  // Direct physical mapping
#define VMM_KERNEL_VMALLOC      0xFFFF800100000000ULL  // Kernel vmalloc area
#define VMM_KERNEL_MODULES      0xFFFF800200000000ULL  // Kernel modules
#define VMM_KERNEL_PERCPU       0xFFFF800300000000ULL  // Per-CPU data
#define VMM_KERNEL_TEXT         0xFFFFFFFF80000000ULL  // Kernel code and data

// Page table constants (x86-64 4-level paging)
#define VMM_PML4_SHIFT          39
#define VMM_PDPT_SHIFT          30  
#define VMM_PD_SHIFT            21
#define VMM_PT_SHIFT            12
#define VMM_PAGE_SIZE           4096
#define VMM_PAGE_MASK           0xFFF

#define VMM_PML4_ENTRIES        512
#define VMM_PDPT_ENTRIES        512
#define VMM_PD_ENTRIES          512
#define VMM_PT_ENTRIES          512

// Page table entry flags (x86-64)
#define VMM_PTE_PRESENT         (1ULL << 0)   // Page is present
#define VMM_PTE_WRITE           (1ULL << 1)   // Page is writable
#define VMM_PTE_USER            (1ULL << 2)   // User accessible
#define VMM_PTE_PWT             (1ULL << 3)   // Page-level write-through
#define VMM_PTE_PCD             (1ULL << 4)   // Page-level cache disable
#define VMM_PTE_ACCESSED        (1ULL << 5)   // Page accessed
#define VMM_PTE_DIRTY           (1ULL << 6)   // Page dirty
#define VMM_PTE_PAT             (1ULL << 7)   // Page Attribute Table
#define VMM_PTE_GLOBAL          (1ULL << 8)   // Global page
#define VMM_PTE_COW             (1ULL << 9)   // Copy-on-write (software)
#define VMM_PTE_SWAPPED         (1ULL << 10)  // Page swapped (software)
#define VMM_PTE_NX              (1ULL << 63)  // No execute

// Address space limits
#define VMM_MAX_VMAS            65536         // Maximum VMAs per process
#define VMM_MMAP_MIN_ADDR       0x10000       // Minimum mmap address
#define VMM_STACK_TOP           0x00007FFFFF000000ULL  // Stack top
#define VMM_MMAP_BASE           0x00007F0000000000ULL  // mmap base

// Virtual memory area types
typedef enum {
    VMA_TYPE_ANONYMOUS,     // Anonymous memory (heap, stack)
    VMA_TYPE_FILE,          // File-backed mapping
    VMA_TYPE_SHARED,        // Shared memory
    VMA_TYPE_STACK,         // Stack memory
    VMA_TYPE_HEAP,          // Heap memory
    VMA_TYPE_VDSO,          // Virtual DSO
    VMA_TYPE_VSYSCALL       // Virtual system call page
} vma_type_t;

// Virtual memory area flags
#define VMA_FLAG_NONE           0x00000000
#define VMA_FLAG_GROWSUP        0x00000001    // VMA can grow upward
#define VMA_FLAG_GROWSDOWN      0x00000002    // VMA can grow downward (stack)
#define VMA_FLAG_LOCKED         0x00000004    // VMA is locked in memory
#define VMA_FLAG_EXECUTABLE     0x00000008    // VMA contains executable code
#define VMA_FLAG_MERGEABLE      0x00000010    // VMA can be merged with adjacent VMAs
#define VMA_FLAG_RANDOMIZED     0x00000020    // VMA address is randomized

// Forward declarations
typedef struct address_space address_space_t;
typedef struct vm_area vm_area_t;
typedef struct page_table page_table_t;

// Virtual memory area structure
struct vm_area {
    uint64_t vm_start;              // Start virtual address
    uint64_t vm_end;                // End virtual address (exclusive)
    uint32_t vm_flags;              // VMA flags
    uint32_t vm_prot;               // Protection flags (PROT_READ, etc.)
    vma_type_t vm_type;             // VMA type
    
    // File backing (if applicable)
    struct {
        int fd;                     // File descriptor
        uint64_t offset;            // File offset
        bool is_shared;             // Shared mapping
    } vm_file;
    
    // Process linkage
    address_space_t* vm_mm;         // Address space this VMA belongs to
    struct vm_area* vm_next;        // Next VMA in address order
    struct vm_area* vm_prev;        // Previous VMA in address order
    
    // Red-black tree linkage for fast lookup
    struct rb_node vm_rb;
    
    // Operations for this VMA
    struct vm_operations* vm_ops;
    
    // Reference counting
    atomic_t vm_usage;              // Reference count
    
    // Statistics
    struct {
        uint64_t page_faults;       // Page faults in this VMA
        uint64_t cow_faults;        // Copy-on-write faults
        uint64_t last_access;       // Last access time
    } vm_stats;
    
    // Private data
    void* vm_private_data;
};

// VM operations structure
struct vm_operations {
    int (*open)(vm_area_t* vma);
    void (*close)(vm_area_t* vma);
    int (*fault)(vm_area_t* vma, uint64_t address, uint64_t* page);
    int (*page_mkwrite)(vm_area_t* vma, uint64_t address);
    int (*access)(vm_area_t* vma, uint64_t address, void* buf, int len, int write);
};

// Page table structure (x86-64 4-level)
struct page_table {
    uint64_t entries[VMM_PT_ENTRIES];
} __attribute__((aligned(4096)));

// Page directory structure
typedef struct page_directory {
    uint64_t entries[VMM_PD_ENTRIES];
} __attribute__((aligned(4096))) page_directory_t;

// Page directory pointer table
typedef struct pdpt {
    uint64_t entries[VMM_PDPT_ENTRIES]; 
} __attribute__((aligned(4096))) pdpt_t;

// Page map level 4 (top level)
typedef struct pml4 {
    uint64_t entries[VMM_PML4_ENTRIES];
} __attribute__((aligned(4096))) pml4_t;

// Address space structure
struct address_space {
    // Page table hierarchy
    pml4_t* pgd;                    // Page global directory (PML4)
    
    // VMA management
    vm_area_t* mmap;                // VMA list head
    struct rb_root mm_rb;           // Red-black tree of VMAs
    uint32_t map_count;             // Number of VMAs
    
    // Memory layout
    uint64_t start_code;            // Code segment start
    uint64_t end_code;              // Code segment end
    uint64_t start_data;            // Data segment start
    uint64_t end_data;              // Data segment end
    uint64_t start_brk;             // Heap start
    uint64_t brk;                   // Current heap end
    uint64_t start_stack;           // Stack start
    uint64_t mmap_base;             // mmap base address
    
    // Address space limits
    struct {
        uint64_t max_data_size;     // Maximum data size
        uint64_t max_stack_size;    // Maximum stack size
        uint64_t max_heap_size;     // Maximum heap size
        uint64_t max_mmap_size;     // Maximum mmap size
    } rlimits;
    
    // Statistics
    struct {
        uint64_t total_vm;          // Total virtual memory
        uint64_t locked_vm;         // Locked virtual memory
        uint64_t resident_pages;    // Resident pages
        uint64_t shared_pages;      // Shared pages
        uint64_t page_faults;       // Total page faults
        uint64_t major_faults;      // Major page faults
        uint64_t minor_faults;      // Minor page faults
    } vm_stats;
    
    // Memory management
    spinlock_t page_table_lock;     // Page table lock
    atomic_t mm_users;              // Number of users
    atomic_t mm_count;              // Reference count
    
    // Process association
    uint32_t owner_pid;             // Process that owns this address space
    
    // ASLR state
    struct {
        bool enabled;               // ASLR enabled
        uint64_t mmap_rnd_bits;     // mmap randomization bits
        uint64_t stack_rnd_bits;    // Stack randomization bits
    } aslr;
    
    // Security context
    void* security;                 // Security module private data
};

// VMM manager structure
typedef struct vmm_manager {
    bool initialized;
    
    // Address space management
    address_space_t* kernel_mm;     // Kernel address space
    
    // Global statistics
    struct {
        atomic64_t total_pages_mapped;
        atomic64_t total_pages_unmapped;
        atomic64_t total_page_faults;
        atomic64_t total_cow_faults;
        atomic64_t total_swap_ins;
        atomic64_t total_swap_outs;
    } stats;
    
    // Configuration
    struct {
        uint64_t vmalloc_start;     // vmalloc area start
        uint64_t vmalloc_end;       // vmalloc area end
        uint64_t high_memory;       // High memory threshold
        bool execute_disable;       // NX bit support
        bool smep_enabled;          // SMEP support
        bool smap_enabled;          // SMAP support
    } config;
    
    // Memory reclaim
    struct {
        uint64_t pages_scanned;     // Pages scanned for reclaim
        uint64_t pages_reclaimed;   // Pages reclaimed
        uint64_t swap_attempts;     // Swap attempts
    } reclaim_stats;
    
    spinlock_t global_lock;
} vmm_manager_t;

// VMM statistics structure
struct vm_stats {
    uint64_t total_vm;          // Total virtual memory
    uint64_t locked_vm;         // Locked virtual memory  
    uint64_t resident_pages;    // Resident pages
    uint64_t shared_pages;      // Shared pages
    uint64_t executable_pages;  // Executable pages
    uint64_t page_faults;       // Total page faults
    uint64_t major_faults;      // Major page faults
    uint64_t minor_faults;      // Minor page faults
    uint64_t cow_faults;        // Copy-on-write faults
    uint64_t swap_in;           // Pages swapped in
    uint64_t swap_out;          // Pages swapped out
};

// Global VMM manager
extern vmm_manager_t* vmm;

// Core VMM API functions

/**
 * Initialize the Virtual Memory Manager
 * @return 0 on success, negative error code on failure
 */
int vmm_init(void);

/**
 * Late initialization after other subsystems
 * @return 0 on success, negative error code on failure
 */
int vmm_late_init(void);

/**
 * Cleanup VMM resources
 */
void vmm_cleanup(void);

// Address space management

/**
 * Create a new address space
 * @return New address space or NULL on failure
 */
address_space_t* vmm_create_address_space(void);

/**
 * Destroy an address space
 * @param mm Address space to destroy
 */
void vmm_destroy_address_space(address_space_t* mm);

/**
 * Clone an address space (for fork)
 * @param mm Address space to clone
 * @param flags Clone flags
 * @return New address space or NULL on failure
 */
address_space_t* vmm_clone_address_space(address_space_t* mm, uint32_t flags);

/**
 * Switch to different address space
 * @param mm Address space to switch to
 */
void vmm_switch_address_space(address_space_t* mm);

/**
 * Get current address space
 * @return Current address space
 */
address_space_t* vmm_get_current_address_space(void);

// Virtual memory mapping

/**
 * Map virtual address to physical address
 * @param mm Address space
 * @param vaddr Virtual address
 * @param paddr Physical address
 * @param size Size to map
 * @param prot Protection flags
 * @return 0 on success, negative error code on failure
 */
int vmm_map_page(address_space_t* mm, uint64_t vaddr, uint64_t paddr, 
                 size_t size, uint32_t prot);

/**
 * Unmap virtual address range
 * @param mm Address space
 * @param vaddr Virtual address
 * @param size Size to unmap
 * @return 0 on success, negative error code on failure
 */
int vmm_unmap_pages(address_space_t* mm, uint64_t vaddr, size_t size);

/**
 * Change protection of virtual address range
 * @param mm Address space
 * @param vaddr Virtual address
 * @param size Size of range
 * @param prot New protection flags
 * @return 0 on success, negative error code on failure
 */
int vmm_protect_pages(address_space_t* mm, uint64_t vaddr, size_t size, uint32_t prot);

// VMA management

/**
 * Find VMA containing address
 * @param mm Address space
 * @param addr Virtual address
 * @return VMA or NULL if not found
 */
vm_area_t* vmm_find_vma(address_space_t* mm, uint64_t addr);

/**
 * Create a new VMA
 * @param mm Address space  
 * @param start Start address
 * @param len Length
 * @param prot Protection flags
 * @param flags VMA flags
 * @param type VMA type
 * @return New VMA or NULL on failure
 */
vm_area_t* vmm_create_vma(address_space_t* mm, uint64_t start, size_t len,
                          uint32_t prot, uint32_t flags, vma_type_t type);

/**
 * Remove VMA from address space
 * @param mm Address space
 * @param vma VMA to remove
 */
void vmm_remove_vma(address_space_t* mm, vm_area_t* vma);

/**
 * Split VMA at given address
 * @param vma VMA to split
 * @param addr Address to split at
 * @return New VMA or NULL on failure
 */
vm_area_t* vmm_split_vma(vm_area_t* vma, uint64_t addr);

/**
 * Try to merge adjacent VMAs
 * @param mm Address space
 * @param vma VMA to try merging
 * @return Merged VMA or original VMA
 */
vm_area_t* vmm_merge_vma(address_space_t* mm, vm_area_t* vma);

// Memory mapping (mmap/munmap)

/**
 * Map memory (mmap implementation)
 * @param mm Address space
 * @param addr Preferred address (0 for any)
 * @param len Length to map
 * @param prot Protection flags
 * @param flags Mapping flags
 * @param fd File descriptor (-1 for anonymous)
 * @param offset File offset
 * @return Mapped address or NULL on failure
 */
uint64_t vmm_mmap(address_space_t* mm, uint64_t addr, size_t len, uint32_t prot,
                  uint32_t flags, int fd, uint64_t offset);

/**
 * Unmap memory (munmap implementation)
 * @param mm Address space
 * @param addr Address to unmap
 * @param len Length to unmap
 * @return 0 on success, negative error code on failure
 */
int vmm_munmap(address_space_t* mm, uint64_t addr, size_t len);

// Page fault handling

/**
 * Handle page fault
 * @param mm Address space
 * @param addr Faulting address
 * @param error_code Error code from CPU
 * @return 0 on success, negative error code on failure
 */
int vmm_handle_page_fault(address_space_t* mm, uint64_t addr, uint64_t error_code);

/**
 * Handle copy-on-write fault
 * @param mm Address space
 * @param vma VMA containing the fault
 * @param addr Faulting address
 * @return 0 on success, negative error code on failure
 */
int vmm_handle_cow_fault(address_space_t* mm, vm_area_t* vma, uint64_t addr);

// Address translation

/**
 * Translate virtual address to physical
 * @param mm Address space
 * @param vaddr Virtual address
 * @return Physical address or 0 if not mapped
 */
uint64_t vmm_virt_to_phys(address_space_t* mm, uint64_t vaddr);

/**
 * Walk page table for address
 * @param mm Address space
 * @param vaddr Virtual address
 * @param create_missing Create missing page table levels
 * @return Pointer to PTE or NULL
 */
uint64_t* vmm_walk_page_table(address_space_t* mm, uint64_t vaddr, bool create_missing);

// Memory information

/**
 * Get address space statistics
 * @param mm Address space
 * @param stats Output statistics
 * @return 0 on success, negative error code on failure
 */
int vmm_get_address_space_stats(address_space_t* mm, struct vm_stats* stats);

/**
 * Check if address is valid user space address
 * @param addr Virtual address
 * @return True if valid user address
 */
bool vmm_is_user_address(uint64_t addr);

/**
 * Check if address is valid kernel address
 * @param addr Virtual address
 * @return True if valid kernel address
 */
bool vmm_is_kernel_address(uint64_t addr);

// Utility functions

/**
 * Flush TLB for address range
 * @param mm Address space
 * @param start Start address
 * @param end End address
 */
void vmm_flush_tlb_range(address_space_t* mm, uint64_t start, uint64_t end);

/**
 * Invalidate page in TLB
 * @param addr Virtual address
 */
void vmm_invalidate_page(uint64_t addr);

// Debug and validation

/**
 * Validate address space consistency
 * @param mm Address space to validate
 * @return Number of errors found
 */
int vmm_validate_address_space(address_space_t* mm);

/**
 * Dump address space layout
 * @param mm Address space to dump
 */
void vmm_dump_address_space(address_space_t* mm);

/**
 * Dump page table contents
 * @param mm Address space
 * @param vaddr Virtual address to dump around
 */
void vmm_dump_page_table(address_space_t* mm, uint64_t vaddr);

// Inline utility functions

static inline uint64_t vmm_pml4_index(uint64_t addr) {
    return (addr >> VMM_PML4_SHIFT) & 0x1FF;
}

static inline uint64_t vmm_pdpt_index(uint64_t addr) {
    return (addr >> VMM_PDPT_SHIFT) & 0x1FF;
}

static inline uint64_t vmm_pd_index(uint64_t addr) {
    return (addr >> VMM_PD_SHIFT) & 0x1FF;
}

static inline uint64_t vmm_pt_index(uint64_t addr) {
    return (addr >> VMM_PT_SHIFT) & 0x1FF;
}

static inline uint64_t vmm_page_align(uint64_t addr) {
    return addr & ~VMM_PAGE_MASK;
}

static inline uint64_t vmm_page_offset(uint64_t addr) {
    return addr & VMM_PAGE_MASK;
}

#ifdef __cplusplus
}
#endif

#endif // VMM_PRODUCTION_H