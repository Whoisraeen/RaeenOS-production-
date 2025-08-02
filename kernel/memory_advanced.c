/**
 * Advanced Memory Management System for RaeenOS
 * Implements complete virtual memory, swapping, protection, and optimization
 */

#include "memory.h"
#include "memory_advanced.h"
#include "paging.h"
#include "swap.h"
#include "security.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================
// ADVANCED MEMORY PROTECTION
// ============================================================================

typedef enum {
    MEMORY_PROT_NONE = 0,
    MEMORY_PROT_READ = 1,
    MEMORY_PROT_WRITE = 2,
    MEMORY_PROT_EXEC = 4,
    MEMORY_PROT_USER = 8,
    MEMORY_PROT_KERNEL = 16,
    MEMORY_PROT_SHARED = 32,
    MEMORY_PROT_COPY_ON_WRITE = 64
} memory_protection_t;

typedef struct memory_region {
    uintptr_t start_addr;
    size_t size;
    memory_protection_t protection;
    uint32_t flags;
    struct memory_region* next;
    struct memory_region* prev;

    // Reference counting for shared memory
    atomic_int ref_count;

    // Backing store information
    struct {
        bool is_file_backed;
        uint64_t file_offset;
        void* file_handle;
        bool is_swappable;
        uint64_t swap_offset;
    } backing;

    // Performance tracking
    struct {
        uint64_t access_count;
        uint64_t last_access_time;
        uint32_t page_faults;
        bool is_hot;
    } stats;
} memory_region_t;

typedef struct address_space {
    uint64_t cr3_value;  // Page directory base
    memory_region_t* regions;
    spinlock_t lock;

    // Memory usage statistics
    size_t total_virtual;
    size_t total_physical;
    size_t total_swap;
    size_t peak_usage;

    // Security context
    uint32_t security_level;
    bool nx_enabled;
    bool smep_enabled;
    bool smap_enabled;

    // Process association
    uint32_t process_id;
    struct address_space* parent;
} address_space_t;

// ============================================================================
// SWAP SUBSYSTEM
// ============================================================================

typedef struct swap_entry {
    uint64_t offset;
    uint32_t size;
    bool in_use;
    uint32_t priority;
    struct swap_entry* next;
} swap_entry_t;

typedef struct swap_device {
    char name[32];
    uint64_t total_size;
    uint64_t used_size;
    uint32_t priority;
    bool is_active;

    // Device operations
    int (*read_page)(struct swap_device* dev, uint64_t offset, void* buffer);
    int (*write_page)(struct swap_device* dev, uint64_t offset, const void* buffer);

    swap_entry_t* free_entries;
    spinlock_t lock;
} swap_device_t;

// Global swap management
static swap_device_t swap_devices[MAX_SWAP_DEVICES];
static int active_swap_devices = 0;
static uint64_t total_swap_space = 0;
static uint64_t used_swap_space = 0;

// ============================================================================
// MEMORY ALLOCATOR IMPROVEMENTS
// ============================================================================

typedef struct slab_cache {
    char name[32];
    size_t object_size;
    size_t alignment;
    uint32_t objects_per_slab;

    // Constructor/destructor for objects
    void (*constructor)(void* obj);
    void (*destructor)(void* obj);

    // Slab lists
    struct slab* full_slabs;
    struct slab* partial_slabs;
    struct slab* empty_slabs;

    // Statistics
    uint64_t total_objects;
    uint64_t active_objects;
    uint64_t allocations;
    uint64_t frees;

    spinlock_t lock;
} slab_cache_t;

typedef struct buddy_allocator {
    void* memory_base;
    size_t total_size;
    uint32_t max_order;

    // Free lists for each order
    struct free_block** free_lists;
    uint32_t* free_counts;

    // Bitmap for tracking allocations
    uint8_t* allocation_bitmap;

    // Statistics
    uint64_t total_allocations;
    uint64_t total_frees;
    size_t peak_usage;

    spinlock_t lock;
} buddy_allocator_t;

// ============================================================================
// IMPLEMENTATION
// ============================================================================

static address_space_t kernel_address_space;
static slab_cache_t* memory_region_cache;
static buddy_allocator_t physical_allocator;

// Initialize advanced memory management
int memory_advanced_init(void) {
    // Initialize kernel address space
    memset(&kernel_address_space, 0, sizeof(address_space_t));
    kernel_address_space.cr3_value = get_current_cr3();
    kernel_address_space.security_level = SECURITY_LEVEL_KERNEL;
    kernel_address_space.nx_enabled = cpu_has_nx();
    kernel_address_space.smep_enabled = cpu_has_smep();
    kernel_address_space.smap_enabled = cpu_has_smap();
    spinlock_init(&kernel_address_space.lock);

    // Initialize slab cache for memory regions
    memory_region_cache = slab_cache_create("memory_regions",
        sizeof(memory_region_t), 8, NULL, NULL);
    if (!memory_region_cache) {
        return -ENOMEM;
    }

    // Initialize buddy allocator for physical memory
    if (buddy_allocator_init(&physical_allocator,
        get_physical_memory_base(), get_physical_memory_size()) < 0) {
        return -ENOMEM;
    }

    // Initialize swap subsystem
    swap_init();

    return 0;
}

// Create new address space for process
address_space_t* address_space_create(uint32_t process_id) {
    address_space_t* as = kmalloc(sizeof(address_space_t));
    if (!as) return NULL;

    memset(as, 0, sizeof(address_space_t));

    // Create new page directory
    as->cr3_value = create_page_directory();
    if (!as->cr3_value) {
        kfree(as);
        return NULL;
    }

    as->process_id = process_id;
    as->security_level = SECURITY_LEVEL_USER;
    as->nx_enabled = true;
    spinlock_init(&as->lock);

    return as;
}

// Map memory region with protection
int memory_map_region(address_space_t* as, uintptr_t vaddr, size_t size,
                     memory_protection_t protection, uint32_t flags) {
    if (!as || !size || (vaddr & PAGE_MASK)) {
        return -EINVAL;
    }

    spinlock_acquire(&as->lock);

    // Check for overlapping regions
    memory_region_t* existing = find_memory_region(as, vaddr, size);
    if (existing) {
        spinlock_release(&as->lock);
        return -EEXIST;
    }

    // Allocate new region
    memory_region_t* region = slab_cache_alloc(memory_region_cache);
    if (!region) {
        spinlock_release(&as->lock);
        return -ENOMEM;
    }

    // Initialize region
    region->start_addr = vaddr;
    region->size = ALIGN_UP(size, PAGE_SIZE);
    region->protection = protection;
    region->flags = flags;
    atomic_set(&region->ref_count, 1);

    // Allocate physical pages if needed
    if (!(flags & MAP_LAZY)) {
        if (allocate_physical_pages(region) < 0) {
            slab_cache_free(memory_region_cache, region);
            spinlock_release(&as->lock);
            return -ENOMEM;
        }
    }

    // Insert into region list
    insert_memory_region(as, region);

    // Update page tables
    if (map_pages_to_region(as, region) < 0) {
        remove_memory_region(as, region);
        slab_cache_free(memory_region_cache, region);
        spinlock_release(&as->lock);
        return -ENOMEM;
    }

    as->total_virtual += region->size;
    spinlock_release(&as->lock);

    return 0;
}

// Handle page fault with advanced features
int handle_page_fault(uintptr_t fault_addr, uint32_t error_code,
                     address_space_t* as) {
    spinlock_acquire(&as->lock);

    memory_region_t* region = find_memory_region(as, fault_addr, 1);
    if (!region) {
        spinlock_release(&as->lock);
        return -EFAULT; // Segmentation fault
    }

    region->stats.page_faults++;
    region->stats.last_access_time = get_system_time();

    // Handle different fault types
    if (error_code & PAGE_FAULT_PROTECTION) {
        // Protection violation
        if (region->protection & MEMORY_PROT_COPY_ON_WRITE) {
            // Handle copy-on-write
            if (handle_cow_fault(region, fault_addr) < 0) {
                spinlock_release(&as->lock);
                return -ENOMEM;
            }
        } else {
            spinlock_release(&as->lock);
            return -EACCES; // Access violation
        }
    } else if (error_code & PAGE_FAULT_NOT_PRESENT) {
        // Page not present
        if (region->backing.is_swappable &&
            is_page_swapped(region, fault_addr)) {
            // Swap in page
            if (swap_in_page(region, fault_addr) < 0) {
                spinlock_release(&as->lock);
                return -EIO;
            }
        } else {
            // Allocate new page (lazy allocation)
            if (allocate_page_for_region(region, fault_addr) < 0) {
                spinlock_release(&as->lock);
                return -ENOMEM;
            }
        }
    }

    spinlock_release(&as->lock);
    return 0;
}

// Swap out pages when memory is low
int swap_out_pages(size_t target_pages) {
    size_t pages_swapped = 0;

    // Use LRU algorithm to select pages for swapping
    for (int i = 0; i < MAX_PROCESSES && pages_swapped < target_pages; i++) {
        address_space_t* as = get_process_address_space(i);
        if (!as) continue;

        spinlock_acquire(&as->lock);

        memory_region_t* region = as->regions;
        while (region && pages_swapped < target_pages) {
            if (region->backing.is_swappable &&
                !region->stats.is_hot &&
                (get_system_time() - region->stats.last_access_time) > SWAP_THRESHOLD) {

                size_t region_pages = region->size / PAGE_SIZE;
                for (size_t j = 0; j < region_pages && pages_swapped < target_pages; j++) {
                    uintptr_t page_addr = region->start_addr + (j * PAGE_SIZE);

                    if (is_page_present(page_addr) &&
                        !is_page_locked(page_addr)) {

                        if (swap_out_page(region, page_addr) == 0) {
                            pages_swapped++;
                        }
                    }
                }
            }
            region = region->next;
        }

        spinlock_release(&as->lock);
    }

    return pages_swapped;
}

// Memory compaction to reduce fragmentation
int memory_compact(void) {
    // Implement memory compaction algorithm
    // Move allocated pages to reduce fragmentation

    spinlock_acquire(&physical_allocator.lock);

    // Scan for movable pages
    uint64_t moved_pages = 0;
    for (uint32_t order = 0; order < physical_allocator.max_order; order++) {
        struct free_block* block = physical_allocator.free_lists[order];

        while (block) {
            // Try to merge with adjacent blocks
            if (try_merge_blocks(block, order)) {
                moved_pages++;
            }
            block = block->next;
        }
    }

    spinlock_release(&physical_allocator.lock);

    return moved_pages;
}

// Advanced memory statistics
void get_memory_stats(memory_stats_t* stats) {
    memset(stats, 0, sizeof(memory_stats_t));

    // Physical memory stats
    stats->total_physical = physical_allocator.total_size;
    stats->used_physical = physical_allocator.total_size -
        calculate_free_physical();
    stats->peak_physical = physical_allocator.peak_usage;

    // Virtual memory stats
    stats->total_virtual = VIRTUAL_MEMORY_SIZE;
    stats->used_virtual = calculate_used_virtual();

    // Swap stats
    stats->total_swap = total_swap_space;
    stats->used_swap = used_swap_space;

    // Cache stats
    stats->cache_size = calculate_cache_usage();
    stats->buffer_size = calculate_buffer_usage();

    // Performance stats
    stats->page_faults = get_total_page_faults();
    stats->swap_ins = get_total_swap_ins();
    stats->swap_outs = get_total_swap_outs();
}

// Memory pressure handling
void handle_memory_pressure(memory_pressure_level_t level) {
    switch (level) {
        case MEMORY_PRESSURE_LOW:
            // Start background page reclaim
            schedule_page_reclaim();
            break;

        case MEMORY_PRESSURE_MEDIUM:
            // More aggressive reclaim
            reclaim_clean_pages();
            compact_memory();
            break;

        case MEMORY_PRESSURE_HIGH:
            // Emergency measures
            swap_out_pages(EMERGENCY_SWAP_PAGES);
            kill_memory_hogs();
            break;

        case MEMORY_PRESSURE_CRITICAL:
            // Last resort
            trigger_oom_killer();
            break;
    }
}

// NUMA-aware memory allocation
void* numa_alloc(size_t size, int node) {
    if (node < 0 || node >= get_numa_node_count()) {
        node = get_current_numa_node();
    }

    // Try to allocate from preferred node
    void* ptr = buddy_alloc_from_node(&physical_allocator, size, node);
    if (ptr) {
        return ptr;
    }

    // Fall back to other nodes
    for (int i = 0; i < get_numa_node_count(); i++) {
        if (i != node) {
            ptr = buddy_alloc_from_node(&physical_allocator, size, i);
            if (ptr) {
                return ptr;
            }
        }
    }

    return NULL;
}
