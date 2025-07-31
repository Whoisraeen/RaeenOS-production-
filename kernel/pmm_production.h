#ifndef PMM_PRODUCTION_H
#define PMM_PRODUCTION_H

/**
 * @file pmm_production.h
 * @brief Production-Grade Physical Memory Manager for RaeenOS
 * 
 * This implements a comprehensive physical memory manager with:
 * - Buddy system allocator for efficient allocation/deallocation
 * - NUMA-aware memory management
 * - Multiple memory zones (DMA, DMA32, Normal, High, Device, Movable)
 * - Memory statistics and debugging
 * - Performance optimization with O(log n) allocation
 * - Memory leak detection and validation
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"
#include "memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory zones aligned with memory_interface.h
#define PMM_MAX_ORDER 11        // 2^11 * 4KB = 8MB max allocation
#define PMM_MAX_NUMA_NODES 64   // Maximum NUMA nodes supported
#define PMM_FRAME_SIZE 4096     // 4KB frame size
#define PMM_FRAME_SHIFT 12      // log2(4096)

// Memory zone definitions
typedef enum {
    PMM_ZONE_DMA = 0,      // < 16MB (ISA DMA)
    PMM_ZONE_DMA32,        // < 4GB (32-bit device DMA)
    PMM_ZONE_NORMAL,       // Normal memory
    PMM_ZONE_HIGH,         // High memory (x86-32 only)
    PMM_ZONE_DEVICE,       // Device memory (memory-mapped I/O)
    PMM_ZONE_MOVABLE,      // Movable/reclaimable memory
    PMM_ZONE_COUNT
} pmm_zone_t;

// Buddy allocator free list entry
typedef struct free_area {
    struct list_head free_list;     // List of free blocks
    unsigned long nr_free;          // Number of free blocks
    spinlock_t lock;               // Per-order lock for SMP
} free_area_t;

// Page frame descriptor
typedef struct page {
    // Page flags (atomic operations)
    atomic_t flags;
    
    // Reference count
    atomic_t ref_count;
    
    // Buddy allocator order
    unsigned int order;
    
    // Zone this page belongs to
    pmm_zone_t zone;
    
    // NUMA node
    unsigned int numa_node;
    
    // LRU and other lists
    struct list_head lru;
    
    // Page state tracking
    struct {
        unsigned long pfn;          // Page frame number
        void* virtual;              // Virtual address if mapped
        unsigned long flags;        // Page state flags
    } state;
    
    // Memory debugging
    struct {
        const char* alloc_func;     // Function that allocated this page
        const char* alloc_file;     // File that allocated this page
        int alloc_line;            // Line that allocated this page
        uint64_t alloc_time;       // Allocation timestamp
    } debug;
    
    // Private data for slab allocator, etc.
    void* private;
} page_t;

// NUMA node descriptor
typedef struct numa_node {
    unsigned int node_id;           // Node ID
    unsigned long start_pfn;        // Start page frame number
    unsigned long end_pfn;          // End page frame number
    unsigned long present_pages;    // Pages present in this node
    unsigned long spanned_pages;    // Total pages spanned
    
    // Per-zone page counts
    unsigned long zone_start_pfn[PMM_ZONE_COUNT];
    unsigned long zone_end_pfn[PMM_ZONE_COUNT];
    unsigned long zone_present_pages[PMM_ZONE_COUNT];
    
    // Distance table to other nodes
    unsigned char distance[PMM_MAX_NUMA_NODES];
    
    // CPU mask for this node
    uint64_t cpu_mask;
    
    // Statistics
    struct {
        unsigned long allocations;
        unsigned long frees;
        unsigned long failures;
        unsigned long migrations;
    } stats;
    
    spinlock_t lock;
} numa_node_t;

// Memory zone descriptor
typedef struct mem_zone {
    pmm_zone_t zone_type;           // Zone type
    unsigned long zone_start_pfn;   // First PFN in zone
    unsigned long zone_end_pfn;     // Last PFN in zone (exclusive)
    unsigned long present_pages;    // Pages present in zone
    unsigned long managed_pages;    // Pages managed by buddy allocator
    
    // Watermarks for memory reclaim
    unsigned long watermark[3];     // min, low, high watermarks
    
    // Per-order free areas (buddy allocator)
    free_area_t free_area[PMM_MAX_ORDER];
    
    // Zone statistics
    struct {
        unsigned long nr_free;      // Total free pages
        unsigned long nr_alloc_batch; // Batch allocation size
        unsigned long nr_alloc_success;
        unsigned long nr_alloc_fail;
        unsigned long nr_free_cma;  // Contiguous memory allocator pages
    } vm_stat;
    
    // Per-CPU page sets for fast allocation
    struct {
        struct list_head lists[2];  // Hot and cold lists
        int count[2];              // Count of pages in lists
        int high;                  // High watermark
        int batch;                 // Batch size
    } pageset[NR_CPUS];
    
    spinlock_t lock;               // Zone lock
    
    const char* name;              // Zone name for debugging
} mem_zone_t;

// Physical Memory Manager main structure
typedef struct pmm_manager {
    // Global state
    bool initialized;
    unsigned long total_pages;      // Total physical pages
    unsigned long managed_pages;    // Pages managed by allocator
    unsigned long reserved_pages;   // Reserved pages (kernel, etc.)
    
    // Memory zones
    mem_zone_t zones[PMM_ZONE_COUNT];
    unsigned int nr_zones;         // Number of active zones
    
    // NUMA nodes
    numa_node_t nodes[PMM_MAX_NUMA_NODES];
    unsigned int nr_nodes;         // Number of NUMA nodes
    
    // Page frame array
    page_t* mem_map;               // Array of page descriptors
    unsigned long mem_map_size;    // Size of mem_map array
    
    // Zone fallback order for allocation failure
    pmm_zone_t fallback_order[PMM_ZONE_COUNT][PMM_ZONE_COUNT];
    
    // Global statistics
    struct {
        atomic64_t total_allocations;
        atomic64_t total_frees;
        atomic64_t allocation_failures;
        atomic64_t oom_kills;
        atomic64_t page_faults;
        
        // Per-order allocation statistics
        atomic64_t alloc_orders[PMM_MAX_ORDER];
        atomic64_t free_orders[PMM_MAX_ORDER];
        
        // Per-zone statistics
        atomic64_t zone_allocations[PMM_ZONE_COUNT];
        atomic64_t zone_failures[PMM_ZONE_COUNT];
    } stats;
    
    // Memory debugging
    struct {
        bool leak_detection_enabled;
        bool corruption_check_enabled;
        bool debug_pagealloc;
        atomic_t debug_pages_allocated;
        struct list_head debug_list;
        spinlock_t debug_lock;
    } debug;
    
    // Configuration
    struct {
        unsigned long min_free_pages;  // Minimum free pages to maintain
        unsigned long low_free_pages;  // Low watermark
        unsigned long high_free_pages; // High watermark
        bool numa_enabled;            // NUMA support enabled
        int default_migratetype;      // Default page migration type
    } config;
    
    // Synchronization
    spinlock_t global_lock;        // Global PMM lock
    
} pmm_manager_t;

// Global PMM manager instance
extern pmm_manager_t* pmm;

// Core PMM API functions

/**
 * Initialize the Physical Memory Manager
 * @param mmap_addr Physical address of memory map
 * @param mmap_length Length of memory map
 * @return 0 on success, negative error code on failure
 */
int pmm_init(uint32_t mmap_addr, uint32_t mmap_length);

/**
 * Late initialization after other subsystems are ready
 * @return 0 on success, negative error code on failure
 */
int pmm_late_init(void);

/**
 * Cleanup PMM resources
 */
void pmm_cleanup(void);

/**
 * Allocate pages using buddy allocator
 * @param order Order of allocation (2^order pages)
 * @param flags Allocation flags (GFP_KERNEL, GFP_ATOMIC, etc.)
 * @param node Preferred NUMA node (-1 for any)
 * @return Physical address of allocated memory, or NULL on failure
 */
void* pmm_alloc_pages(unsigned int order, unsigned int flags, int node);

/**
 * Free pages allocated by pmm_alloc_pages
 * @param addr Physical address to free
 * @param order Order of allocation
 */
void pmm_free_pages(void* addr, unsigned int order);

/**
 * Allocate a single page
 * @param flags Allocation flags
 * @param node Preferred NUMA node (-1 for any)
 * @return Physical address of allocated page, or NULL on failure
 */
static inline void* pmm_alloc_page(unsigned int flags, int node) {
    return pmm_alloc_pages(0, flags, node);
}

/**
 * Free a single page
 * @param addr Physical address to free
 */
static inline void pmm_free_page(void* addr) {
    pmm_free_pages(addr, 0);
}

/**
 * Get page descriptor from physical address
 * @param addr Physical address
 * @return Pointer to page descriptor
 */
page_t* pmm_addr_to_page(void* addr);

/**
 * Get physical address from page descriptor
 * @param page Page descriptor
 * @return Physical address
 */
void* pmm_page_to_addr(page_t* page);

/**
 * Reserve pages (mark as unavailable for allocation)
 * @param start_pfn Starting page frame number
 * @param count Number of pages to reserve
 * @return 0 on success, negative error code on failure
 */
int pmm_reserve_pages(unsigned long start_pfn, unsigned long count);

/**
 * Unreserve previously reserved pages
 * @param start_pfn Starting page frame number
 * @param count Number of pages to unreserve
 * @return 0 on success, negative error code on failure
 */
int pmm_unreserve_pages(unsigned long start_pfn, unsigned long count);

// NUMA-aware functions

/**
 * Get the current NUMA node
 * @return Current NUMA node ID
 */
int pmm_numa_node_id(void);

/**
 * Allocate pages on specific NUMA node
 * @param order Order of allocation
 * @param flags Allocation flags
 * @param node NUMA node ID
 * @return Physical address or NULL on failure
 */
void* pmm_alloc_pages_node(unsigned int order, unsigned int flags, int node);

/**
 * Get NUMA node for physical address
 * @param addr Physical address
 * @return NUMA node ID
 */
int pmm_addr_to_nid(void* addr);

// Zone management functions

/**
 * Get zone for physical address
 * @param addr Physical address
 * @return Zone type
 */
pmm_zone_t pmm_addr_to_zone(void* addr);

/**
 * Get zone statistics
 * @param zone Zone type
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int pmm_get_zone_stats(pmm_zone_t zone, struct zone_stats* stats);

// Memory information and statistics

/**
 * Get global memory statistics
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int pmm_get_memory_stats(memory_stats_t* stats);

/**
 * Get total system memory
 * @return Total memory in bytes
 */
uint64_t pmm_get_total_memory(void);

/**
 * Get free memory
 * @return Free memory in bytes
 */
uint64_t pmm_get_free_memory(void);

/**
 * Check if system is under memory pressure
 * @return True if under memory pressure
 */
bool pmm_under_memory_pressure(void);

// Memory debugging and validation

/**
 * Enable memory leak detection
 * @param enable True to enable, false to disable
 */
void pmm_enable_leak_detection(bool enable);

/**
 * Check for memory corruption
 * @return Number of corrupted pages found
 */
int pmm_check_corruption(void);

/**
 * Validate physical address
 * @param addr Physical address to validate
 * @return True if address is valid
 */
bool pmm_validate_addr(void* addr);

/**
 * Dump memory map for debugging
 */
void pmm_dump_memory_map(void);

/**
 * Dump zone information
 * @param zone Zone to dump (-1 for all zones)
 */
void pmm_dump_zones(int zone);

// Internal helper functions (should not be called directly)

/**
 * Initialize memory zones from memory map
 */
int pmm_init_zones(uint32_t mmap_addr, uint32_t mmap_length);

/**
 * Initialize NUMA topology
 */
int pmm_init_numa(void);

/**
 * Initialize buddy allocator for a zone
 */
int pmm_init_buddy_system(mem_zone_t* zone);

/**
 * Internal buddy allocator functions
 */
page_t* __pmm_alloc_pages(mem_zone_t* zone, unsigned int order, unsigned int flags);
void __pmm_free_pages(mem_zone_t* zone, page_t* page, unsigned int order);

/**
 * Memory zone fallback allocation
 */
page_t* pmm_rmqueue_fallback(unsigned int order, unsigned int flags);

/**
 * Update zone watermarks
 */
void pmm_update_zone_watermarks(mem_zone_t* zone);

// Utility macros
#define PMM_PFN_TO_ADDR(pfn) ((void*)((pfn) << PMM_FRAME_SHIFT))
#define PMM_ADDR_TO_PFN(addr) (((unsigned long)(addr)) >> PMM_FRAME_SHIFT)
#define PMM_PFN_VALID(pfn) ((pfn) < pmm->total_pages)
#define PMM_ADDR_VALID(addr) PMM_PFN_VALID(PMM_ADDR_TO_PFN(addr))

// Page flags
#define PG_locked        0  // Page is locked in memory
#define PG_error         1  // Page has an error
#define PG_referenced    2  // Page has been referenced
#define PG_uptodate      3  // Page contents are valid
#define PG_dirty         4  // Page has been modified
#define PG_lru           5  // Page is on LRU list
#define PG_active        6  // Page is on active list
#define PG_reserved      7  // Page is reserved
#define PG_private       8  // Page has private data
#define PG_slab          9  // Page is used by slab allocator
#define PG_compound      10 // Part of compound page
#define PG_reclaim       11 // Page should be reclaimed
#define PG_buddy         12 // Page is in buddy allocator

#ifdef __cplusplus
}
#endif

#endif // PMM_PRODUCTION_H