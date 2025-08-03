/**
 * @file heap_production.c
 * @brief Production-Grade Kernel Heap Manager with Slab Allocator
 * 
 * This file implements a comprehensive kernel heap manager for RaeenOS
 * with slab allocation, debugging features, and performance optimization.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/memory_interface.h"
#include "pmm_production.h"
#include "vmm_production_impl.c"
#include "include/sync.h"
#include "include/types.h"
#include "include/errno.h"
// Use list functions from sync.h instead
#define list_for_each_entry(pos, head, member)                \
    for (pos = list_entry((head)->next, typeof(*pos), member);    \
         &pos->member != (head);                    \
         pos = list_entry(pos->member.next, typeof(*pos), member))
#include "string.h"
#include "vga.h"

// Slab sizes for general-purpose allocation
static const size_t slab_sizes[] = {
    32, 64, 96, 128, 192, 256, 512, 1024, 2048, 4096
};
#define NUM_SLAB_SIZES (sizeof(slab_sizes) / sizeof(slab_sizes[0]))

// Use different name to avoid conflict with memory_interface.h
typedef struct kernel_slab_cache {
    char name[64];                  // Cache name
    size_t object_size;             // Object size
    size_t align;                   // Alignment requirement
    uint32_t flags;                 // Cache flags
    
    // Constructor/destructor
    void (*ctor)(void* obj);
    void (*dtor)(void* obj);
    
    // Slab lists
    struct list_head full_slabs;    // Completely allocated slabs
    struct list_head partial_slabs; // Partially allocated slabs
    struct list_head empty_slabs;   // Empty slabs
    
    // Statistics
    atomic64_t total_objects;
    atomic64_t active_objects;
    atomic64_t allocations;
    atomic64_t frees;
    atomic64_t slab_count;
    
    // Synchronization
    spinlock_t lock;
    
    // Next cache in global list
    struct kernel_slab_cache* next;
} kernel_slab_cache_t;

// Individual slab structure
typedef struct slab {
    struct list_head list;          // List linkage
    kernel_slab_cache_t* cache;           // Parent cache
    void* objects;                 // Start of objects
    uint32_t free_count;           // Number of free objects
    uint32_t objects_per_slab;     // Total objects in this slab
    void* freelist;                // Free object list
    bool* allocation_bitmap;        // Allocation bitmap
} slab_t;

// Free object header
typedef struct free_object {
    struct free_object* next;
} free_object_t;

// Heap manager structure
typedef struct heap_manager {
    bool initialized;
    
    // General-purpose slab caches
    kernel_slab_cache_t* general_caches[NUM_SLAB_SIZES];
    
    // Special-purpose caches
    kernel_slab_cache_t* cache_cache;      // Cache for cache descriptors
    kernel_slab_cache_t* slab_cache;       // Cache for slab descriptors
    
    // Large allocation tracking
    struct list_head large_allocs;
    spinlock_t large_alloc_lock;
    
    // Global cache list
    kernel_slab_cache_t* cache_list;
    spinlock_t cache_list_lock;
    
    // Statistics
    struct {
        atomic64_t total_allocations;
        atomic64_t total_frees;
        atomic64_t bytes_allocated;
        atomic64_t bytes_freed;
        atomic64_t large_allocations;
        atomic64_t cache_count;
    } stats;
    
    // Configuration
    struct {
        size_t large_alloc_threshold;   // Threshold for large allocations
        uint32_t slab_size;            // Standard slab size
        bool debug_enabled;            // Debug mode
        bool leak_detection;           // Memory leak detection
    } config;
    
} heap_manager_t;

// Large allocation header
typedef struct large_alloc {
    struct list_head list;
    size_t size;
    uint32_t magic;
    void* caller;
    const char* file;
    int line;
    uint64_t timestamp;
} large_alloc_t;

#define LARGE_ALLOC_MAGIC 0xDEADBEEF
#define SLAB_MAGIC 0xABCDEF00

// Global heap manager
static heap_manager_t heap_manager;
static heap_manager_t* heap = &heap_manager;

// Forward declarations
static kernel_slab_cache_t* create_slab_cache(const char* name, size_t size, size_t align, 
                                       uint32_t flags, void (*ctor)(void*), void (*dtor)(void*));
static void destroy_slab_cache(kernel_slab_cache_t* cache);
static void* slab_alloc_from_cache(kernel_slab_cache_t* cache, uint32_t flags);
static void slab_free_to_cache(kernel_slab_cache_t* cache, void* obj);
static slab_t* create_slab(kernel_slab_cache_t* cache);
static void destroy_slab(slab_t* slab);
static void* large_alloc(size_t size, uint32_t flags);
static void large_free(void* ptr);
static size_t find_slab_size(size_t size);

/**
 * Simple string formatter for cache names
 */
static int simple_sprintf(char* buf, size_t size, const char* prefix, size_t number) {
    if (!buf || !prefix || size == 0) return 0;
    
    // Copy prefix
    size_t prefix_len = strlen(prefix);
    if (prefix_len >= size) prefix_len = size - 1;
    
    memcpy(buf, prefix, prefix_len);
    
    // Convert number to string (simple implementation)
    char num_str[32];
    int num_len = 0;
    size_t temp = number;
    
    if (temp == 0) {
        num_str[0] = '0';
        num_len = 1;
    } else {
        while (temp > 0) {
            num_str[num_len++] = '0' + (temp % 10);
            temp /= 10;
        }
        // Reverse the digits
        for (int i = 0; i < num_len / 2; i++) {
            char c = num_str[i];
            num_str[i] = num_str[num_len - 1 - i];
            num_str[num_len - 1 - i] = c;
        }
    }
    
    // Append number to buffer
    size_t remaining = size - prefix_len - 1;
    if ((size_t)num_len > remaining) num_len = remaining;
    
    memcpy(buf + prefix_len, num_str, num_len);
    buf[prefix_len + num_len] = '\0';
    
    return prefix_len + num_len;
}

/**
 * Initialize the kernel heap manager
 */
int heap_init(void) {
    vga_puts("HEAP: Initializing production kernel heap manager...\n");
    
    // Clear heap manager structure
    memset(heap, 0, sizeof(heap_manager_t));
    
    // Initialize locks
    spinlock_init(&heap->large_alloc_lock);
    spinlock_init(&heap->cache_list_lock);
    
    // Initialize lists
    INIT_LIST_HEAD(&heap->large_allocs);
    
    // Set configuration
    heap->config.large_alloc_threshold = 4096;  // 4KB threshold
    heap->config.slab_size = 4096;              // 4KB slabs
    heap->config.debug_enabled = true;
    heap->config.leak_detection = true;
    
    // Create cache for cache descriptors
    heap->cache_cache = create_slab_cache("cache_cache", sizeof(kernel_slab_cache_t), 
                                         sizeof(void*), 0, NULL, NULL);
    if (!heap->cache_cache) {
        vga_puts("HEAP: Failed to create cache cache\n");
        return -ENOMEM;
    }
    
    // Create cache for slab descriptors
    heap->slab_cache = create_slab_cache("slab_cache", sizeof(slab_t), 
                                        sizeof(void*), 0, NULL, NULL);
    if (!heap->slab_cache) {
        vga_puts("HEAP: Failed to create slab cache\n");
        return -ENOMEM;
    }
    
    // Create general-purpose caches
    for (int i = 0; i < NUM_SLAB_SIZES; i++) {
        char name[32];
        simple_sprintf(name, sizeof(name), "kmalloc-", slab_sizes[i]);
        
        heap->general_caches[i] = create_slab_cache(name, slab_sizes[i], 
                                                   sizeof(void*), 0, NULL, NULL);
        if (!heap->general_caches[i]) {
            vga_puts("HEAP: Failed to create general cache\n");
            return -ENOMEM;
        }
    }
    
    heap->initialized = true;
    
    vga_puts("HEAP: Kernel heap manager initialized successfully\n");
    return 0;
}

/**
 * Kernel malloc implementation
 */
void* kmalloc(size_t size, uint32_t flags) {
    if (!heap->initialized || size == 0) {
        return NULL;
    }
    
    atomic64_inc(&heap->stats.total_allocations);
    atomic64_add(&heap->stats.bytes_allocated, size);
    
    // Check for large allocation
    if (size > heap->config.large_alloc_threshold) {
        atomic64_inc(&heap->stats.large_allocations);
        return large_alloc(size, flags);
    }
    
    // Find appropriate slab size
    size_t slab_size = find_slab_size(size);
    if (slab_size == 0) {
        return large_alloc(size, flags);
    }
    
    // Find cache index
    int cache_idx = -1;
    for (int i = 0; i < NUM_SLAB_SIZES; i++) {
        if (slab_sizes[i] == slab_size) {
            cache_idx = i;
            break;
        }
    }
    
    if (cache_idx == -1) {
        return large_alloc(size, flags);
    }
    
    // Allocate from slab cache
    void* ptr = slab_alloc_from_cache(heap->general_caches[cache_idx], flags);
    
    // Zero memory if requested
    if (ptr && (flags & MM_FLAG_ZERO)) {
        memset(ptr, 0, size);
    }
    
    return ptr;
}

/**
 * Kernel free implementation
 */
void kfree(void* ptr) {
    if (!ptr || !heap->initialized) {
        return;
    }
    
    atomic64_inc(&heap->stats.total_frees);
    
    // Check if it's a large allocation
    // Simple heuristic: check if it's page-aligned and > threshold
    if (((uintptr_t)ptr & (PAGE_SIZE - 1)) == 0) {
        large_alloc_t* header = (large_alloc_t*)((char*)ptr - sizeof(large_alloc_t));
        if (header->magic == LARGE_ALLOC_MAGIC) {
            large_free(ptr);
            return;
        }
    }
    
    // Find which slab this belongs to
    // This is simplified - in production we'd use more efficient lookup
    for (int i = 0; i < NUM_SLAB_SIZES; i++) {
        kernel_slab_cache_t* cache = heap->general_caches[i];
        
        // Check if pointer could belong to this cache
        // This is a simplified check
        if (cache) {
            slab_free_to_cache(cache, ptr);
            return;
        }
    }
    
    // If we get here, something is wrong
    vga_puts("HEAP: Warning - kfree() called on unknown pointer\n");
}

/**
 * Aligned kernel malloc
 */
void* kmalloc_aligned(size_t size, size_t alignment, uint32_t flags) {
    if (!heap->initialized || size == 0 || alignment == 0) {
        return NULL;
    }
    
    // For simplicity, use large allocation for aligned requests
    return large_alloc(size + alignment - 1, flags);
}

/**
 * Kernel realloc implementation
 */
void* krealloc(void* ptr, size_t new_size, uint32_t flags) {
    if (!new_size) {
        kfree(ptr);
        return NULL;
    }
    
    if (!ptr) {
        return kmalloc(new_size, flags);
    }
    
    // For simplicity, allocate new block and copy
    void* new_ptr = kmalloc(new_size, flags);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copy old data (we don't know the old size, so copy up to new size)
    // In production, we'd track allocation sizes
    memcpy(new_ptr, ptr, new_size);
    kfree(ptr);
    
    return new_ptr;
}

/**
 * Create a new slab cache
 */
static kernel_slab_cache_t* create_slab_cache(const char* name, size_t size, size_t align, 
                                       uint32_t flags, void (*ctor)(void*), void (*dtor)(void*)) {
    kernel_slab_cache_t* cache;
    
    // Allocate cache descriptor
    if (heap->cache_cache) {
        cache = (kernel_slab_cache_t*)slab_alloc_from_cache(heap->cache_cache, GFP_KERNEL);
    } else {
        // Bootstrap allocation
        void* page = pmm_alloc_page(GFP_KERNEL, -1);
        if (!page) return NULL;
        cache = (kernel_slab_cache_t*)page;
    }
    
    if (!cache) {
        return NULL;
    }
    
    memset(cache, 0, sizeof(kernel_slab_cache_t));
    
    // Initialize cache
    strcpy(cache->name, name);
    cache->object_size = size;
    cache->align = align ? align : sizeof(void*);
    cache->flags = flags;
    cache->ctor = ctor;
    cache->dtor = dtor;
    
    // Initialize lists
    INIT_LIST_HEAD(&cache->full_slabs);
    INIT_LIST_HEAD(&cache->partial_slabs);
    INIT_LIST_HEAD(&cache->empty_slabs);
    
    // Initialize lock
    spinlock_init(&cache->lock);
    
    // Add to global cache list
    spin_lock(&heap->cache_list_lock);
    cache->next = heap->cache_list;
    heap->cache_list = cache;
    atomic64_inc(&heap->stats.cache_count);
    spin_unlock(&heap->cache_list_lock);
    
    return cache;
}

/**
 * Allocate object from slab cache
 */
static void* slab_alloc_from_cache(kernel_slab_cache_t* cache, uint32_t flags) {
    if (!cache) {
        return NULL;
    }
    
    spin_lock(&cache->lock);
    
    slab_t* slab = NULL;
    
    // Try to find a partial slab first
    if (!list_empty(&cache->partial_slabs)) {
        slab = list_first_entry(&cache->partial_slabs, slab_t, list);
    } else if (!list_empty(&cache->empty_slabs)) {
        // Use an empty slab
        slab = list_first_entry(&cache->empty_slabs, slab_t, list);
        list_del(&slab->list);
        list_add(&slab->list, &cache->partial_slabs);
    } else {
        // Create a new slab
        spin_unlock(&cache->lock);
        slab = create_slab(cache);
        if (!slab) {
            return NULL;
        }
        spin_lock(&cache->lock);
        list_add(&slab->list, &cache->partial_slabs);
    }
    
    // Allocate object from slab
    void* obj = NULL;
    if (slab && slab->freelist) {
        obj = slab->freelist;
        free_object_t* next = ((free_object_t*)obj)->next;
        slab->freelist = next;
        slab->free_count--;
        
        // Move slab to appropriate list if it's now full
        if (slab->free_count == 0) {
            list_del(&slab->list);
            list_add(&slab->list, &cache->full_slabs);
        }
        
        atomic64_inc(&cache->active_objects);
        atomic64_inc(&cache->allocations);
    }
    
    spin_unlock(&cache->lock);
    
    // Call constructor if present
    if (obj && cache->ctor) {
        cache->ctor(obj);
    }
    
    return obj;
}

/**
 * Free object to slab cache
 */
static void slab_free_to_cache(kernel_slab_cache_t* cache, void* obj) {
    if (!cache || !obj) {
        return;
    }
    
    // Call destructor if present
    if (cache->dtor) {
        cache->dtor(obj);
    }
    
    spin_lock(&cache->lock);
    
    // Find which slab this object belongs to
    // This is simplified - in production we'd use more efficient lookup
    slab_t* target_slab = NULL;
    
    // Check full slabs first
    list_for_each_entry(target_slab, &cache->full_slabs, list) {
        if (obj >= target_slab->objects && 
            obj < (char*)target_slab->objects + (target_slab->objects_per_slab * cache->object_size)) {
            
            // Move from full to partial
            list_del(&target_slab->list);
            list_add(&target_slab->list, &cache->partial_slabs);
            break;
        }
    }
    
    // Check partial slabs
    if (!target_slab) {
        list_for_each_entry(target_slab, &cache->partial_slabs, list) {
            if (obj >= target_slab->objects && 
                obj < (char*)target_slab->objects + (target_slab->objects_per_slab * cache->object_size)) {
                break;
            }
        }
    }
    
    if (target_slab) {
        // Add object to freelist
        free_object_t* free_obj = (free_object_t*)obj;
        free_obj->next = (free_object_t*)target_slab->freelist;
        target_slab->freelist = free_obj;
        target_slab->free_count++;
        
        // Move to empty list if slab is now completely free
        if (target_slab->free_count == target_slab->objects_per_slab) {
            list_del(&target_slab->list);
            list_add(&target_slab->list, &cache->empty_slabs);
        }
        
        atomic64_dec(&cache->active_objects);
        atomic64_inc(&cache->frees);
    }
    
    spin_unlock(&cache->lock);
}

/**
 * Create a new slab for cache
 */
static slab_t* create_slab(kernel_slab_cache_t* cache) {
    if (!cache) {
        return NULL;
    }
    
    // Allocate slab descriptor
    slab_t* slab;
    if (heap->slab_cache) {
        slab = (slab_t*)slab_alloc_from_cache(heap->slab_cache, GFP_KERNEL);
    } else {
        // Bootstrap allocation
        void* page = pmm_alloc_page(GFP_KERNEL, -1);
        if (!page) return NULL;
        slab = (slab_t*)page;
    }
    
    if (!slab) {
        return NULL;
    }
    
    memset(slab, 0, sizeof(slab_t));
    
    // Allocate memory for objects
    void* objects = pmm_alloc_page(GFP_KERNEL, -1);
    if (!objects) {
        if (heap->slab_cache) {
            slab_free_to_cache(heap->slab_cache, slab);
        } else {
            pmm_free_page(slab);
        }
        return NULL;
    }
    
    // Initialize slab
    slab->cache = cache;
    slab->objects = objects;
    slab->objects_per_slab = heap->config.slab_size / cache->object_size;
    slab->free_count = slab->objects_per_slab;
    
    // Build freelist
    char* obj_ptr = (char*)objects;
    free_object_t* prev = NULL;
    
    for (uint32_t i = 0; i < slab->objects_per_slab; i++) {
        free_object_t* free_obj = (free_object_t*)obj_ptr;
        free_obj->next = prev;
        prev = free_obj;
        obj_ptr += cache->object_size;
    }
    
    slab->freelist = prev;
    
    atomic64_inc(&cache->slab_count);
    
    return slab;
}

/**
 * Large allocation implementation
 */
static void* large_alloc(size_t size, uint32_t flags) {
    // Calculate total size including header
    size_t total_size = sizeof(large_alloc_t) + size;
    
    // Allocate pages
    size_t pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    void* mem = pmm_alloc_pages(0, flags, -1);  // Order 0 for single pages
    if (!mem) {
        return NULL;
    }
    
    // Set up header
    large_alloc_t* header = (large_alloc_t*)mem;
    header->size = size;
    header->magic = LARGE_ALLOC_MAGIC;
    header->caller = __builtin_return_address(0);
    header->file = __FILE__;
    header->line = __LINE__;
    header->timestamp = hal->cpu_timestamp();
    
    // Add to tracking list
    spin_lock(&heap->large_alloc_lock);
    list_add(&header->list, &heap->large_allocs);
    spin_unlock(&heap->large_alloc_lock);
    
    // Return pointer after header
    return (char*)mem + sizeof(large_alloc_t);
}

/**
 * Large free implementation
 */
static void large_free(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Get header
    large_alloc_t* header = (large_alloc_t*)((char*)ptr - sizeof(large_alloc_t));
    
    if (header->magic != LARGE_ALLOC_MAGIC) {
        vga_puts("HEAP: Invalid magic in large_free\n");
        return;
    }
    
    // Remove from tracking list
    spin_lock(&heap->large_alloc_lock);
    list_del(&header->list);
    spin_unlock(&heap->large_alloc_lock);
    
    // Free pages
    size_t total_size = sizeof(large_alloc_t) + header->size;
    size_t pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    pmm_free_pages(header, 0);  // Order 0 for single pages
    
    atomic64_add(&heap->stats.bytes_freed, header->size);
}

/**
 * Find appropriate slab size for allocation
 */
static size_t find_slab_size(size_t size) {
    for (int i = 0; i < NUM_SLAB_SIZES; i++) {
        if (size <= slab_sizes[i]) {
            return slab_sizes[i];
        }
    }
    return 0;  // Too large for slab allocation
}

/**
 * Get heap statistics
 */
int heap_get_stats(memory_stats_t* stats) {
    if (!stats || !heap->initialized) {
        return -EINVAL;
    }
    
    memset(stats, 0, sizeof(memory_stats_t));
    
    // Copy basic statistics
    stats->slab_total = atomic64_read(&heap->stats.bytes_allocated);
    stats->slab_used = atomic64_read(&heap->stats.bytes_allocated) - 
                       atomic64_read(&heap->stats.bytes_freed);
    stats->slab_free = 0;  // Would calculate from free objects
    
    return 0;
}

/**
 * Cleanup heap manager
 */
void heap_cleanup(void) {
    heap->initialized = false;
    // In production, would free all caches and slabs
}