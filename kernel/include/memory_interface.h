#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

/**
 * @file memory_interface.h
 * @brief Comprehensive Memory Management Interface for RaeenOS
 * 
 * This interface defines the unified memory management API including
 * virtual memory, heap allocation, physical memory management, and
 * advanced features like NUMA, memory protection, and swapping.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory management API version
#define MEMORY_API_VERSION 1

// Memory allocation flags
#define MM_FLAG_KERNEL      (1 << 0)   // Kernel memory
#define MM_FLAG_USER        (1 << 1)   // User memory
#define MM_FLAG_DMA         (1 << 2)   // DMA-capable memory
#define MM_FLAG_ZERO        (1 << 3)   // Zero-initialized memory
#define MM_FLAG_ATOMIC      (1 << 4)   // Atomic allocation (no sleep)
#define MM_FLAG_CONTIGUOUS  (1 << 5)   // Physically contiguous
#define MM_FLAG_HIGH        (1 << 6)   // High memory
#define MM_FLAG_LOW         (1 << 7)   // Low memory
#define MM_FLAG_NODMA       (1 << 8)   // Non-DMA memory
#define MM_FLAG_NOWAIT      (1 << 9)   // No waiting
#define MM_FLAG_RETRY       (1 << 10)  // Retry on failure
#define MM_FLAG_LOCKED      (1 << 11)  // Lock pages in memory

// Memory protection flags
#define MM_PROT_NONE        0x00        // No access
#define MM_PROT_READ        (1 << 0)    // Read access
#define MM_PROT_WRITE       (1 << 1)    // Write access
#define MM_PROT_EXEC        (1 << 2)    // Execute access
#define MM_PROT_USER        (1 << 3)    // User mode access
#define MM_PROT_KERNEL      (1 << 4)    // Kernel mode access
#define MM_PROT_NOCACHE     (1 << 5)    // Non-cacheable
#define MM_PROT_WRITETHRU   (1 << 6)    // Write-through cache
#define MM_PROT_WRITEBACK   (1 << 7)    // Write-back cache

// Memory mapping flags
#define MM_MAP_PRIVATE      (1 << 0)    // Private mapping
#define MM_MAP_SHARED       (1 << 1)    // Shared mapping
#define MM_MAP_FIXED        (1 << 2)    // Fixed address
#define MM_MAP_ANONYMOUS    (1 << 3)    // Anonymous mapping
#define MM_MAP_LOCKED       (1 << 4)    // Lock mapping
#define MM_MAP_POPULATE     (1 << 5)    // Pre-populate pages
#define MM_MAP_NONBLOCK     (1 << 6)    // Non-blocking
#define MM_MAP_GROWSDOWN    (1 << 7)    // Stack-like mapping
#define MM_MAP_HUGE         (1 << 8)    // Use huge pages
#define MM_MAP_NORESERVE    (1 << 9)    // Don't reserve swap

// Memory zone types
typedef enum {
    MEMORY_ZONE_DMA,        // DMA-capable memory (< 16MB on x86)
    MEMORY_ZONE_DMA32,      // 32-bit DMA memory (< 4GB)
    MEMORY_ZONE_NORMAL,     // Normal memory
    MEMORY_ZONE_HIGH,       // High memory (> 896MB on 32-bit x86)
    MEMORY_ZONE_DEVICE,     // Device memory
    MEMORY_ZONE_MOVABLE,    // Movable memory for defragmentation
    MEMORY_ZONE_COUNT
} memory_zone_t;

// Memory allocation algorithms
typedef enum {
    MM_ALLOC_FIRST_FIT,
    MM_ALLOC_BEST_FIT,
    MM_ALLOC_WORST_FIT,
    MM_ALLOC_BUDDY,
    MM_ALLOC_SLAB,
    MM_ALLOC_SLUB
} mm_alloc_algorithm_t;

// NUMA node information
typedef struct numa_node {
    uint32_t node_id;
    uint64_t memory_size;
    uint64_t free_memory;
    uint32_t cpu_mask;          // CPUs in this node
    uint32_t distance_map[16];  // Distance to other nodes
} numa_node_t;

// Memory statistics
typedef struct memory_stats {
    // Physical memory
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t used_pages;
    uint64_t cached_pages;
    uint64_t buffer_pages;
    uint64_t shared_pages;
    uint64_t reserved_pages;
    
    // Virtual memory
    uint64_t vmalloc_total;
    uint64_t vmalloc_used;
    uint64_t vmalloc_free;
    
    // Slab allocator
    uint64_t slab_total;
    uint64_t slab_used;
    uint64_t slab_free;
    
    // Swap
    uint64_t swap_total;
    uint64_t swap_used;
    uint64_t swap_free;
    uint64_t swap_in_pages;
    uint64_t swap_out_pages;
    
    // Memory pressure
    uint32_t memory_pressure;   // 0-100 percentage
    uint32_t oom_kills;         // Out of memory kills
    
    // Per-zone statistics
    struct {
        uint64_t total_pages;
        uint64_t free_pages;
        uint64_t min_pages;
        uint64_t low_pages;
        uint64_t high_pages;
    } zones[MEMORY_ZONE_COUNT];
} memory_stats_t;

// Virtual memory area (VMA)
typedef struct vma {
    void* start;                // Start address
    void* end;                  // End address
    uint32_t flags;             // VMA flags
    uint32_t prot;              // Protection flags
    
    // File backing (if applicable)
    struct {
        int fd;                 // File descriptor
        off_t offset;           // File offset
        size_t length;          // Mapping length
    } file;
    
    // NUMA information
    uint32_t numa_node;         // Preferred NUMA node
    uint32_t numa_policy;       // NUMA allocation policy
    
    // Statistics
    uint64_t fault_count;       // Page fault count
    uint64_t access_time;       // Last access time
    
    // Private data
    void* private_data;
} vma_t;

// Page frame structure
typedef struct page_frame {
    phys_addr_t phys_addr;      // Physical address
    void* virt_addr;            // Virtual address (if mapped)
    uint32_t flags;             // Page flags
    uint32_t ref_count;         // Reference count
    uint32_t order;             // Buddy system order
    memory_zone_t zone;         // Memory zone
    uint32_t numa_node;         // NUMA node
    
    // Page state
    enum {
        PAGE_FREE,
        PAGE_ALLOCATED,
        PAGE_RESERVED,
        PAGE_SWAPPED,
        PAGE_DIRTY,
        PAGE_LOCKED
    } state;
    
    // Swap information
    struct {
        uint32_t swap_device;
        uint64_t swap_offset;
    } swap;
    
    // LRU information
    struct page_frame* lru_next;
    struct page_frame* lru_prev;
} page_frame_t;

// Memory mapping descriptor
typedef struct memory_mapping {
    uint32_t mapping_id;        // Unique mapping ID
    void* virt_addr;            // Virtual address
    phys_addr_t phys_addr;      // Physical address (if direct mapping)
    size_t size;                // Mapping size
    uint32_t flags;             // Mapping flags
    uint32_t prot;              // Protection flags
    uint32_t ref_count;         // Reference count
    
    // Process information
    uint32_t pid;               // Process ID (0 for kernel)
    
    // File backing
    struct {
        int fd;
        off_t offset;
        bool is_file_backed;
    } file;
    
    void* private_data;
} memory_mapping_t;

// Slab cache structure
typedef struct slab_cache {
    char name[64];              // Cache name
    size_t object_size;         // Object size
    size_t align;               // Alignment requirement
    uint32_t flags;             // Cache flags
    
    // Constructor/destructor
    void (*ctor)(void* obj);
    void (*dtor)(void* obj);
    
    // Statistics
    uint64_t total_objects;
    uint64_t active_objects;
    uint64_t allocations;
    uint64_t frees;
    
    void* private_data;
} slab_cache_t;

// Memory allocator interface
typedef struct memory_allocator {
    const char* name;
    mm_alloc_algorithm_t algorithm;
    
    // Core allocation functions
    void* (*alloc)(size_t size, uint32_t flags);
    void* (*alloc_aligned)(size_t size, size_t alignment, uint32_t flags);
    void* (*realloc)(void* ptr, size_t new_size, uint32_t flags);
    void (*free)(void* ptr);
    
    // Page allocation
    void* (*alloc_pages)(size_t pages, uint32_t flags);
    void (*free_pages)(void* ptr, size_t pages);
    
    // NUMA-aware allocation
    void* (*alloc_on_node)(size_t size, uint32_t node, uint32_t flags);
    
    // Statistics
    int (*get_stats)(memory_stats_t* stats);
    
    void* private_data;
} memory_allocator_t;

// Memory management operations
typedef struct memory_operations {
    // Initialization
    int (*init)(void);
    void (*cleanup)(void);
    
    // Physical memory management
    int (*pmm_init)(void);
    page_frame_t* (*pmm_alloc_page)(uint32_t flags);
    int (*pmm_free_page)(page_frame_t* page);
    page_frame_t* (*pmm_alloc_pages)(size_t pages, uint32_t flags);
    int (*pmm_free_pages)(page_frame_t* pages, size_t count);
    
    // Virtual memory management
    int (*vmm_init)(void);
    void* (*vmm_alloc)(size_t size, uint32_t flags);
    void (*vmm_free)(void* ptr);
    int (*vmm_map)(void* virt, phys_addr_t phys, size_t size, uint32_t prot);
    int (*vmm_unmap)(void* virt, size_t size);
    int (*vmm_protect)(void* virt, size_t size, uint32_t prot);
    phys_addr_t (*vmm_virt_to_phys)(void* virt);
    void* (*vmm_phys_to_virt)(phys_addr_t phys);
    
    // Heap management
    void* (*heap_alloc)(size_t size, uint32_t flags);
    void* (*heap_alloc_aligned)(size_t size, size_t alignment, uint32_t flags);
    void* (*heap_realloc)(void* ptr, size_t new_size, uint32_t flags);
    void (*heap_free)(void* ptr);
    size_t (*heap_size)(void* ptr);
    
    // Memory mapping
    memory_mapping_t* (*mmap)(void* addr, size_t length, uint32_t prot, 
                              uint32_t flags, int fd, off_t offset);
    int (*munmap)(memory_mapping_t* mapping);
    int (*mprotect)(memory_mapping_t* mapping, uint32_t prot);
    int (*msync)(memory_mapping_t* mapping, uint32_t flags);
    
    // Slab allocator
    slab_cache_t* (*slab_create)(const char* name, size_t size, size_t align,
                                 uint32_t flags, void (*ctor)(void*), void (*dtor)(void*));
    void (*slab_destroy)(slab_cache_t* cache);
    void* (*slab_alloc)(slab_cache_t* cache, uint32_t flags);
    void (*slab_free)(slab_cache_t* cache, void* obj);
    
    // Memory locking
    int (*mlock)(void* addr, size_t len);
    int (*munlock)(void* addr, size_t len);
    int (*mlockall)(uint32_t flags);
    int (*munlockall)(void);
    
    // Memory advice
    int (*madvise)(void* addr, size_t len, int advice);
    
    // NUMA support
    int (*numa_init)(void);
    uint32_t (*numa_get_node_count)(void);
    numa_node_t* (*numa_get_node)(uint32_t node_id);
    void* (*numa_alloc_on_node)(size_t size, uint32_t node, uint32_t flags);
    int (*numa_set_policy)(uint32_t policy, uint32_t* nodes);
    
    // Swap management
    int (*swap_init)(void);
    int (*swap_add_device)(const char* device, uint32_t priority);
    int (*swap_remove_device)(const char* device);
    int (*swap_out_page)(page_frame_t* page);
    int (*swap_in_page)(page_frame_t* page);
    
    // Memory pressure and OOM
    int (*get_memory_pressure)(void);
    int (*oom_kill_process)(uint32_t pid);
    int (*register_oom_notifier)(void (*callback)(void));
    
    // Memory information
    int (*get_memory_info)(memory_stats_t* stats);
    int (*get_zone_info)(memory_zone_t zone, memory_stats_t* stats);
    int (*get_numa_info)(numa_node_t* nodes, size_t* count);
    
    // Memory debugging
    int (*check_memory_corruption)(void);
    int (*dump_memory_map)(void);
    int (*validate_pointer)(void* ptr);
    
    // Copy operations with error checking  
    int (*copy_to_user)(void* user_dest, const void* kernel_src, size_t n);
    int (*copy_from_user)(void* kernel_dest, const void* user_src, size_t n);
    int (*copy_in_user)(void* user_dest, const void* user_src, size_t n);
    
    // String operations with bounds checking
    ssize_t (*strncpy_from_user)(char* dest, const char* user_src, size_t count);
    ssize_t (*strlen_user)(const char* user_str);
    
    // Clear user memory
    int (*clear_user)(void* user_mem, size_t n);
    
    // Memory barriers and cache operations
    void (*memory_barrier)(void);
    void (*read_barrier)(void);
    void (*write_barrier)(void);
    void (*cache_flush)(void* addr, size_t size);
    void (*cache_invalidate)(void* addr, size_t size);
    void (*cache_clean)(void* addr, size_t size);
} memory_ops_t;

// Memory manager structure
typedef struct memory_manager {
    memory_ops_t* ops;
    memory_allocator_t* allocators[8];  // Different allocators
    uint32_t allocator_count;
    
    // Global statistics
    memory_stats_t stats;
    
    // Configuration
    struct {
        size_t page_size;
        size_t huge_page_size;
        uint32_t numa_nodes;
        bool swap_enabled;
        uint32_t oom_score_adj;
        uint32_t swappiness;        // 0-100, swap aggressiveness
        uint32_t dirty_ratio;       // % of memory that can be dirty
        uint32_t vfs_cache_pressure; // VFS cache reclaim pressure
    } config;
    
    // Synchronization
    void* lock;
    
    void* private_data;
} memory_manager_t;

// Global memory manager instance
extern memory_manager_t* mm;

// Memory management API functions

// Initialization
int memory_init(void);
void memory_cleanup(void);
int memory_late_init(void);

// Allocator registration
int memory_register_allocator(memory_allocator_t* allocator);
int memory_unregister_allocator(memory_allocator_t* allocator);
memory_allocator_t* memory_get_allocator(const char* name);

// Core allocation functions
void* kmalloc(size_t size, uint32_t flags);
void* kzalloc(size_t size, uint32_t flags);
void* kmalloc_aligned(size_t size, size_t alignment, uint32_t flags);
void* krealloc(void* ptr, size_t new_size, uint32_t flags);
void kfree(void* ptr);

// Page allocation
void* alloc_pages(size_t pages, uint32_t flags);
void free_pages(void* ptr, size_t pages);
void* alloc_page(uint32_t flags);
void free_page(void* ptr);

// Virtual memory operations
int vmap(void* virt, phys_addr_t phys, size_t size, uint32_t prot);
int vunmap(void* virt, size_t size);
int vprotect(void* virt, size_t size, uint32_t prot);

// Memory mapping operations
memory_mapping_t* do_mmap(void* addr, size_t length, uint32_t prot, 
                          uint32_t flags, int fd, off_t offset);
int do_munmap(memory_mapping_t* mapping);

// Slab allocator
slab_cache_t* kmem_cache_create(const char* name, size_t size, size_t align,
                                uint32_t flags, void (*ctor)(void*), void (*dtor)(void*));
void kmem_cache_destroy(slab_cache_t* cache);
void* kmem_cache_alloc(slab_cache_t* cache, uint32_t flags);
void kmem_cache_free(slab_cache_t* cache, void* obj);

// NUMA functions
void* kmalloc_node(size_t size, uint32_t flags, uint32_t node);
uint32_t numa_node_id(void);
uint32_t numa_mem_id(void);

// User space memory functions
int copy_to_user_safe(void* user_dest, const void* kernel_src, size_t n);
int copy_from_user_safe(void* kernel_dest, const void* user_src, size_t n);
bool access_ok(const void* user_ptr, size_t size);

// Memory debugging
void* kmalloc_debug(size_t size, uint32_t flags, const char* file, int line);
void kfree_debug(void* ptr, const char* file, int line);

#ifdef DEBUG_MEMORY
#define kmalloc(size, flags) kmalloc_debug(size, flags, __FILE__, __LINE__)
#define kfree(ptr) kfree_debug(ptr, __FILE__, __LINE__)
#endif

// Utility macros
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGNED(addr) (((addr) & (PAGE_SIZE - 1)) == 0)
#define PAGES_FOR_SIZE(size) (((size) + PAGE_SIZE - 1) / PAGE_SIZE)

#define NUMA_NO_NODE ((uint32_t)-1)

// Common allocator flags combinations
#define GFP_KERNEL    (MM_FLAG_KERNEL)
#define GFP_USER      (MM_FLAG_USER)
#define GFP_ATOMIC    (MM_FLAG_ATOMIC | MM_FLAG_KERNEL)
#define GFP_DMA       (MM_FLAG_DMA | MM_FLAG_KERNEL)
#define GFP_ZERO      (MM_FLAG_ZERO | MM_FLAG_KERNEL)

#ifdef __cplusplus
}
#endif

#endif // MEMORY_INTERFACE_H