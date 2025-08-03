#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

/**
 * @file slab_allocator.h
 * @brief Production-Grade SLAB/SLUB Allocator for RaeenOS Kernel
 * 
 * This implements a high-performance slab allocator with:
 * - SLUB (Simple Low-fragmentation Unified Buffer) algorithm
 * - Per-CPU caching for lock-free fast paths
 * - Memory leak detection and debugging
 * - Object constructor/destructor support
 * - Memory usage tracking and statistics
 * - Cache coloring to reduce cache conflicts
 * - Emergency allocation handling
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/types.h"
#include "include/memory_interface.h"
#include "pmm_production.h"

#ifdef __cplusplus
extern "C" {
#endif

// Slab allocator constants
#define SLAB_MAX_SIZE           8192        // Maximum object size
#define SLAB_MIN_ALIGN          8           // Minimum alignment
#define SLAB_MAX_ALIGN          4096        // Maximum alignment
#define SLAB_NAME_LEN           32          // Cache name length
#define SLAB_MAX_CACHES         256         // Maximum number of caches
#define SLAB_OBJECTS_PER_SLAB   64          // Target objects per slab

// Cache flags
#define SLAB_HWCACHE_ALIGN      0x00000001  // Align objects to L1 cache line
#define SLAB_POISON             0x00000002  // Poison objects
#define SLAB_RED_ZONE           0x00000004  // Add red zones around objects
#define SLAB_TRACK_CALLER       0x00000008  // Track allocation caller
#define SLAB_STORE_USER         0x00000010  // Store user information
#define SLAB_PANIC              0x00000020  // Panic on allocation failure
#define SLAB_DESTROY_BY_RCU     0x00000040  // RCU-safe destruction
#define SLAB_MEM_SPREAD         0x00000080  // NUMA memory spreading
#define SLAB_TRACE              0x00000100  // Enable tracing
#define SLAB_DEBUG_OBJECTS      0x00000200  // Debug object tracking
#define SLAB_NOLEAKTRACE        0x00000400  // Don't trace for leaks
#define SLAB_FAILSLAB           0x00000800  // Fault injection testing

// Object states for debugging
typedef enum {
    SLAB_OBJECT_FREE,           // Object is free
    SLAB_OBJECT_ALLOCATED,      // Object is allocated
    SLAB_OBJECT_ACTIVE,         // Object is active/in use
    SLAB_OBJECT_POISONED        // Object is poisoned
} slab_object_state_t;

// Per-CPU cache structure
typedef struct kmem_cache_cpu {
    void** freelist;            // Free object list
    struct page* page;          // Current slab page
    struct page* partial;       // Partial slab page
    unsigned long tid;          // Transaction ID for lockless operations
    
    // Statistics
    unsigned long alloc_hit;    // Allocations from CPU cache
    unsigned long alloc_miss;   // Allocations from partial/empty slabs
    unsigned long free_hit;     // Frees to CPU cache
    unsigned long free_miss;    // Frees to partial/full slabs
} kmem_cache_cpu_t;

// Slab page structure
typedef struct slab_page {
    struct page* page;          // Associated page
    void* freelist;             // Free object list
    unsigned int objects;       // Total objects in slab
    unsigned int inuse;         // Objects in use
    unsigned int frozen;        // Frozen flag for CPU cache
    
    // Linkage
    struct slab_page* next;     // Next slab in list
    struct kmem_cache* cache;   // Parent cache
    
    // Debug information
    struct {
        const char* alloc_func; // Allocation function
        const char* alloc_file; // Allocation file
        int alloc_line;         // Allocation line
        uint64_t alloc_time;    // Allocation timestamp
    } debug;
} slab_page_t;

// Cache node structure (per-NUMA node)
typedef struct kmem_cache_node {
    spinlock_t list_lock;       // Lock for slab lists
    
    struct list_head partial;   // Partial slabs
    unsigned long nr_partial;   // Number of partial slabs
    
    struct list_head full;      // Full slabs
    unsigned long nr_full;      // Number of full slabs
    
    // Allocation tracking
    atomic_long_t total_objects;    // Total objects allocated
    atomic_long_t nr_slabs;         // Number of slabs
    
    // NUMA node ID
    int node_id;
} kmem_cache_node_t;

// Object debug structure
typedef struct slab_debug_obj {
    struct list_head list;      // Global debug list
    void* object;               // Object pointer
    size_t size;                // Object size
    const char* alloc_func;     // Allocation function
    const char* alloc_file;     // Allocation file
    int alloc_line;             // Allocation line
    uint64_t alloc_time;        // Allocation timestamp
    slab_object_state_t state;  // Object state
    uint32_t magic;             // Magic number for validation
} slab_debug_obj_t;

// Main cache structure
typedef struct kmem_cache {
    char name[SLAB_NAME_LEN];   // Cache name
    
    // Object properties
    size_t size;                // Object size
    size_t align;               // Object alignment
    size_t object_size;         // Real object size (with metadata)
    size_t slab_size;           // Slab size
    unsigned int objects_per_slab; // Objects per slab
    
    // Cache flags
    unsigned long flags;        // Cache flags
    
    // Constructor/destructor
    void (*ctor)(void*);        // Object constructor
    void (*dtor)(void*);        // Object destructor
    
    // Per-CPU data
    kmem_cache_cpu_t __percpu *cpu_slab;
    
    // Per-NUMA node data
    kmem_cache_node_t* nodes[MAX_NUMA_NODES];
    
    // Cache coloring
    unsigned int colour_off;    // Color offset
    unsigned int colour;        // Current color
    unsigned int colour_next;   // Next color
    
    // Statistics
    struct {
        atomic64_t alloc_hit;       // Fast path allocations
        atomic64_t alloc_miss;      // Slow path allocations
        atomic64_t alloc_node_mismatch; // NUMA node mismatches
        atomic64_t free_hit;        // Fast path frees
        atomic64_t free_miss;       // Slow path frees
        atomic64_t free_frozen;     // Frees to frozen slabs
        atomic64_t alloc_slowpath;  // Slow path entries
        atomic64_t free_slowpath;   // Slow path entries
        atomic64_t alloc_refill;    // CPU cache refills
        atomic64_t alloc_empty;     // Empty CPU cache allocations
        atomic64_t free_remove_partial; // Partial slab removals
        atomic64_t alloc_from_partial;  // Allocations from partial
        atomic64_t free_add_partial;    // Additions to partial
        atomic64_t remove_full;     // Full slab removals
        
        // Memory usage
        atomic64_t total_objects;   // Total allocated objects
        atomic64_t active_objects;  // Currently active objects
        atomic64_t total_slabs;     // Total slabs
        atomic64_t active_slabs;    // Active slabs
        atomic64_t bytes_allocated; // Total bytes allocated
        atomic64_t bytes_freed;     // Total bytes freed
        
        // Debug statistics
        atomic64_t debug_allocs;    // Debug allocations
        atomic64_t debug_frees;     // Debug frees
        atomic64_t poison_checks;   // Poison checks performed
        atomic64_t redzone_checks;  // Red zone checks performed
    } stats;
    
    // Cache linkage
    struct list_head list;      // Global cache list
    struct kmem_cache* parent;  // Parent cache (for aliases)
    int refcount;               // Reference count
    
    // Memory reclaim
    struct {
        struct list_head lru;   // LRU list
        uint64_t last_access;   // Last access time
        bool reclaimable;       // Can be reclaimed
    } reclaim;
    
    // Debugging
    struct {
        bool track_caller;      // Track allocation caller
        bool store_user;        // Store user information
        bool red_zone;          // Use red zones
        bool poison;            // Poison objects
        struct list_head debug_list; // Debug object list
        spinlock_t debug_lock;  // Debug list lock
        atomic_t debug_count;   // Debug object count
    } debug;
    
    // Cache-specific data
    void* private;
} kmem_cache_t;

// General purpose cache sizes
extern kmem_cache_t* kmalloc_caches[KMALLOC_SHIFT_HIGH + 1];

// SLAB allocator manager
typedef struct slab_allocator {
    bool initialized;
    
    // Cache management
    struct list_head cache_list;    // List of all caches
    kmem_cache_t* cache_cache;      // Cache for cache objects
    spinlock_t cache_lock;          // Cache list lock
    unsigned int cache_count;       // Number of caches
    
    // Default caches for kmalloc
    kmem_cache_t* malloc_caches[32]; // Power-of-2 sized caches
    
    // Global statistics
    struct {
        atomic64_t total_caches;        // Total caches created
        atomic64_t total_allocations;   // Total allocations
        atomic64_t total_frees;         // Total frees
        atomic64_t allocation_failures; // Allocation failures
        atomic64_t cache_shrinks;       // Cache shrinks
        atomic64_t slab_reclaims;       // Slab reclaims
        
        // Memory usage
        atomic64_t active_caches;       // Active caches
        atomic64_t total_slabs;         // Total slabs
        atomic64_t total_objects;       // Total objects
        atomic64_t active_objects;      // Active objects
        atomic64_t wasted_bytes;        // Wasted bytes (fragmentation)
    } global_stats;
    
    // Configuration
    struct {
        bool debug_enabled;         // Global debug enable
        bool poison_enabled;        // Global poison enable
        bool redzone_enabled;       // Global red zone enable
        bool track_caller;          // Global caller tracking
        size_t max_cache_size;      // Maximum cache size
        unsigned int shrink_interval; // Shrink interval (seconds)
        unsigned int color_distance;  // Cache coloring distance
    } config;
    
    // Emergency allocation
    struct {
        void* emergency_pool;       // Emergency memory pool
        size_t pool_size;           // Pool size
        size_t pool_used;           // Pool bytes used
        spinlock_t pool_lock;       // Pool lock
        bool pool_active;           // Pool is active
    } emergency;
    
    // Debugging and leak detection
    struct {
        struct list_head leak_list; // List of potential leaks
        spinlock_t leak_lock;       // Leak list lock
        atomic_t tracked_objects;   // Number of tracked objects
        bool leak_detection;        // Leak detection enabled
        uint64_t last_leak_check;  // Last leak check time
    } leak_detector;
    
} slab_allocator_t;

// Global slab allocator instance
extern slab_allocator_t* slab_allocator;

// Core SLAB allocator API

/**
 * Initialize the SLAB allocator
 * @return 0 on success, negative error code on failure
 */
int slab_init(void);

/**
 * Late initialization after system is fully up
 * @return 0 on success, negative error code on failure
 */
int slab_late_init(void);

/**
 * Cleanup SLAB allocator resources
 */
void slab_cleanup(void);

// Cache management

/**
 * Create a new slab cache
 * @param name Cache name
 * @param size Object size
 * @param align Object alignment (0 for default)
 * @param flags Cache flags
 * @param ctor Object constructor (can be NULL)
 * @param dtor Object destructor (can be NULL)
 * @return New cache or NULL on failure
 */
kmem_cache_t* slab_cache_create(const char* name, size_t size, size_t align,
                               unsigned long flags, void (*ctor)(void*), void (*dtor)(void*));

/**
 * Destroy a slab cache
 * @param cache Cache to destroy
 */
void slab_cache_destroy(kmem_cache_t* cache);

/**
 * Allocate object from cache  
 * @param cache Cache to allocate from
 * @param flags Allocation flags
 * @return Allocated object or NULL on failure
 */
void* slab_cache_alloc(kmem_cache_t* cache, unsigned int flags);

/**
 * Allocate object from cache on specific NUMA node
 * @param cache Cache to allocate from
 * @param flags Allocation flags
 * @param node NUMA node ID
 * @return Allocated object or NULL on failure
 */
void* slab_cache_alloc_node(kmem_cache_t* cache, unsigned int flags, int node);

/**
 * Free object back to cache
 * @param cache Cache to free to
 * @param object Object to free
 */
void slab_cache_free(kmem_cache_t* cache, void* object);

/**
 * Shrink cache by freeing unused slabs
 * @param cache Cache to shrink
 * @return Number of slabs freed
 */
int slab_cache_shrink(kmem_cache_t* cache);

// General-purpose allocation (kmalloc interface)

/**
 * Allocate memory of specified size
 * @param size Size to allocate
 * @param flags Allocation flags
 * @return Allocated memory or NULL on failure
 */
void* kmalloc(size_t size, unsigned int flags);

/**
 * Allocate zeroed memory
 * @param size Size to allocate
 * @param flags Allocation flags
 * @return Allocated zeroed memory or NULL on failure
 */
void* kzalloc(size_t size, unsigned int flags);

/**
 * Allocate aligned memory
 * @param size Size to allocate
 * @param align Alignment requirement
 * @param flags Allocation flags
 * @return Allocated aligned memory or NULL on failure
 */
void* kmalloc_aligned(size_t size, size_t align, unsigned int flags);

/**
 * Reallocate memory
 * @param ptr Original pointer
 * @param new_size New size
 * @param flags Allocation flags
 * @return Reallocated memory or NULL on failure
 */
void* krealloc(void* ptr, size_t new_size, unsigned int flags);

/**
 * Free memory allocated by kmalloc
 * @param ptr Pointer to free
 */
void kfree(void* ptr);

/**
 * Get size of allocated object
 * @param ptr Pointer to object
 * @return Size of object or 0 if invalid
 */
size_t ksize(void* ptr);

// NUMA-aware allocation

/**
 * Allocate memory on specific NUMA node
 * @param size Size to allocate
 * @param flags Allocation flags
 * @param node NUMA node ID
 * @return Allocated memory or NULL on failure
 */
void* kmalloc_node(size_t size, unsigned int flags, int node);

/**
 * Allocate zeroed memory on specific NUMA node
 * @param size Size to allocate
 * @param flags Allocation flags
 * @param node NUMA node ID
 * @return Allocated zeroed memory or NULL on failure
 */
void* kzalloc_node(size_t size, unsigned int flags, int node);

// Debug and statistics

/**
 * Get cache statistics
 * @param cache Cache to get statistics for
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int slab_get_cache_stats(kmem_cache_t* cache, struct slab_cache_stats* stats);

/**
 * Get global slab statistics
 * @param stats Output statistics structure
 * @return 0 on success, negative error code on failure
 */
int slab_get_global_stats(struct slab_global_stats* stats);

/**
 * Check for memory leaks
 * @return Number of potential leaks found
 */
int slab_check_leaks(void);

/**
 * Validate object integrity
 * @param object Object to validate
 * @return True if object is valid
 */
bool slab_validate_object(void* object);

/**
 * Dump cache information
 * @param cache Cache to dump (NULL for all caches)
 */
void slab_dump_caches(kmem_cache_t* cache);

/**
 * Enable/disable leak detection
 * @param enable True to enable, false to disable
 */
void slab_set_leak_detection(bool enable);

// Internal functions (should not be called directly)

/**
 * Initialize default kmalloc caches
 */
int slab_init_kmalloc_caches(void);

/**
 * Get cache for kmalloc size
 * @param size Allocation size
 * @return Appropriate cache or NULL
 */
kmem_cache_t* slab_get_kmalloc_cache(size_t size);

/**
 * Emergency allocation from reserved pool
 * @param size Size to allocate
 * @return Allocated memory or NULL
 */
void* slab_emergency_alloc(size_t size);

/**
 * Free emergency allocation
 * @param ptr Pointer to free
 * @param size Size of allocation
 */
void slab_emergency_free(void* ptr, size_t size);

// Utility macros

#define SLAB_CACHE_ALIGN(size) \
    (((size) + SLAB_MIN_ALIGN - 1) & ~(SLAB_MIN_ALIGN - 1))

#define SLAB_CACHE_SIZE(size) \
    ((size) <= 8192 ? (size) : 0)

#define SLAB_IS_ALIGNED(ptr, align) \
    (((uintptr_t)(ptr) & ((align) - 1)) == 0)

// Cache creation shortcuts
#define SLAB_CREATE_CACHE(name, type, flags) \
    slab_cache_create(name, sizeof(type), __alignof__(type), flags, NULL, NULL)

#define SLAB_CREATE_CACHE_CTOR(name, type, flags, ctor) \
    slab_cache_create(name, sizeof(type), __alignof__(type), flags, ctor, NULL)

#define SLAB_CREATE_CACHE_DTOR(name, type, flags, ctor, dtor) \
    slab_cache_create(name, sizeof(type), __alignof__(type), flags, ctor, dtor)

// Debug allocation macros
#ifdef DEBUG_SLAB
#define kmalloc_debug(size, flags) \
    kmalloc_trace(size, flags, __FUNCTION__, __FILE__, __LINE__)
#define kfree_debug(ptr) \
    kfree_trace(ptr, __FUNCTION__, __FILE__, __LINE__)

void* kmalloc_trace(size_t size, unsigned int flags, const char* func, 
                   const char* file, int line);
void kfree_trace(void* ptr, const char* func, const char* file, int line);
#else
#define kmalloc_debug(size, flags) kmalloc(size, flags)
#define kfree_debug(ptr) kfree(ptr)
#endif

#ifdef __cplusplus
}
#endif

#endif // SLAB_ALLOCATOR_H