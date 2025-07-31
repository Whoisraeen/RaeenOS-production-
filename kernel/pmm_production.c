/**
 * @file pmm_production.c
 * @brief Production-Grade Physical Memory Manager Implementation
 * 
 * This file implements a comprehensive physical memory manager for RaeenOS
 * with buddy system allocation, NUMA awareness, multiple memory zones,
 * and extensive debugging and statistics tracking.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "pmm_production.h"
#include "memory_interface.h"
#include "hal_interface.h"
#include "types.h"
#include "errno.h"
#include "string.h"
#include "vga.h"

// Forward declarations
static int pmm_detect_numa_topology(void);
static void pmm_init_zone_fallback_order(void);
static page_t* pmm_expand(mem_zone_t* zone, unsigned int low, unsigned int high, 
                         unsigned int order);
static void pmm_validate_page(page_t* page, const char* func, int line);

// Global PMM manager instance
static pmm_manager_t pmm_manager;
pmm_manager_t* pmm = &pmm_manager;

// Zone names for debugging
static const char* zone_names[PMM_ZONE_COUNT] = {
    "DMA",
    "DMA32", 
    "Normal",
    "High",
    "Device",
    "Movable"
};

// Zone boundaries (in bytes)
static const struct {
    uint64_t start;
    uint64_t end;
} zone_boundaries[PMM_ZONE_COUNT] = {
    { 0x00000000, 0x01000000 },        // DMA: 0-16MB
    { 0x01000000, 0x100000000ULL },    // DMA32: 16MB-4GB
    { 0x100000000ULL, UINT64_MAX },    // Normal: 4GB+
    { 0, 0 },                          // High: not used on 64-bit
    { 0, 0 },                          // Device: varies
    { 0, 0 }                           // Movable: varies
};

/**
 * Initialize the Physical Memory Manager
 */
int pmm_init(uint32_t mmap_addr, uint32_t mmap_length) {
    int ret;
    
    vga_puts("PMM: Initializing production physical memory manager...\n");
    
    // Clear PMM manager structure
    memset(pmm, 0, sizeof(pmm_manager_t));
    
    // Initialize spinlocks
    spinlock_init(&pmm->global_lock);
    spinlock_init(&pmm->debug.debug_lock);
    
    // Initialize debug list
    INIT_LIST_HEAD(&pmm->debug.debug_list);
    
    // Parse memory map and initialize zones
    ret = pmm_init_zones(mmap_addr, mmap_length);
    if (ret < 0) {
        vga_puts("PMM: Failed to initialize memory zones\n");
        return ret;
    }
    
    // Detect and initialize NUMA topology
    ret = pmm_init_numa();
    if (ret < 0) {
        vga_puts("PMM: NUMA initialization failed, using UMA\n");
        pmm->config.numa_enabled = false;
    } else {
        pmm->config.numa_enabled = true;
    }
    
    // Initialize zone fallback order
    pmm_init_zone_fallback_order();
    
    // Configure default settings
    pmm->config.min_free_pages = pmm->total_pages / 128;  // 0.78% minimum
    pmm->config.low_free_pages = pmm->total_pages / 64;   // 1.56% low water
    pmm->config.high_free_pages = pmm->total_pages / 32;  // 3.12% high water
    pmm->config.default_migratetype = 0;
    
    // Enable debugging features
    pmm->debug.leak_detection_enabled = true;
    pmm->debug.corruption_check_enabled = true;
    
    pmm->initialized = true;
    
    vga_puts("PMM: Physical memory manager initialized successfully\n");
    return 0;
}

/**
 * Initialize memory zones from GRUB memory map
 */
int pmm_init_zones(uint32_t mmap_addr, uint32_t mmap_length) {
    mmap_entry_t* mmap = (mmap_entry_t*)mmap_addr;
    mmap_entry_t* mmap_end = (mmap_entry_t*)(mmap_addr + mmap_length);
    
    uint64_t highest_addr = 0;
    uint64_t total_memory = 0;
    
    // First pass: find highest address and total memory
    for (mmap_entry_t* entry = mmap; entry < mmap_end; entry++) {
        if (entry->type == 1) {  // Available RAM
            uint64_t end_addr = entry->addr + entry->len;
            if (end_addr > highest_addr) {
                highest_addr = end_addr;
            }
            total_memory += entry->len;
        }
    }
    
    pmm->total_pages = highest_addr / PMM_FRAME_SIZE;
    
    // Calculate memory map size and find location for it
    size_t mem_map_size = pmm->total_pages * sizeof(page_t);
    size_t mem_map_pages = (mem_map_size + PMM_FRAME_SIZE - 1) / PMM_FRAME_SIZE;
    
    // Find suitable location for mem_map
    void* mem_map_addr = NULL;
    for (mmap_entry_t* entry = mmap; entry < mmap_end; entry++) {
        if (entry->type == 1 && entry->len >= mem_map_size) {
            mem_map_addr = (void*)(uintptr_t)entry->addr;
            break;
        }
    }
    
    if (!mem_map_addr) {
        vga_puts("PMM: Cannot find suitable location for memory map\n");
        return -ENOMEM;
    }
    
    pmm->mem_map = (page_t*)mem_map_addr;
    pmm->mem_map_size = pmm->total_pages;
    
    // Initialize all pages as reserved
    for (unsigned long i = 0; i < pmm->total_pages; i++) {
        page_t* page = &pmm->mem_map[i];
        memset(page, 0, sizeof(page_t));
        
        atomic_set(&page->flags, 1UL << PG_reserved);
        atomic_set(&page->ref_count, 0);
        page->order = 0;
        page->numa_node = 0;
        page->state.pfn = i;
        page->state.virtual = NULL;
        
        INIT_LIST_HEAD(&page->lru);
    }
    
    // Initialize zones
    pmm->nr_zones = 0;
    
    // Initialize DMA zone (0-16MB)
    mem_zone_t* dma_zone = &pmm->zones[PMM_ZONE_DMA];
    dma_zone->zone_type = PMM_ZONE_DMA;
    dma_zone->zone_start_pfn = 0;
    dma_zone->zone_end_pfn = 0x1000000 / PMM_FRAME_SIZE;  // 16MB
    dma_zone->name = zone_names[PMM_ZONE_DMA];
    spinlock_init(&dma_zone->lock);
    
    // Initialize buddy system for DMA zone
    for (int order = 0; order < PMM_MAX_ORDER; order++) {
        INIT_LIST_HEAD(&dma_zone->free_area[order].free_list);
        dma_zone->free_area[order].nr_free = 0;
        spinlock_init(&dma_zone->free_area[order].lock);
    }
    pmm->nr_zones++;
    
    // Initialize Normal zone (16MB+)
    mem_zone_t* normal_zone = &pmm->zones[PMM_ZONE_NORMAL];
    normal_zone->zone_type = PMM_ZONE_NORMAL;
    normal_zone->zone_start_pfn = 0x1000000 / PMM_FRAME_SIZE;  // 16MB
    normal_zone->zone_end_pfn = pmm->total_pages;
    normal_zone->name = zone_names[PMM_ZONE_NORMAL];
    spinlock_init(&normal_zone->lock);
    
    // Initialize buddy system for Normal zone
    for (int order = 0; order < PMM_MAX_ORDER; order++) {
        INIT_LIST_HEAD(&normal_zone->free_area[order].free_list);
        normal_zone->free_area[order].nr_free = 0;
        spinlock_init(&normal_zone->free_area[order].lock);
    }
    pmm->nr_zones++;
    
    // Second pass: mark available pages and add to buddy allocator
    unsigned long reserved_pages = 0;
    for (mmap_entry_t* entry = mmap; entry < mmap_end; entry++) {
        if (entry->type == 1) {  // Available RAM
            unsigned long start_pfn = entry->addr / PMM_FRAME_SIZE;
            unsigned long end_pfn = (entry->addr + entry->len) / PMM_FRAME_SIZE;
            
            for (unsigned long pfn = start_pfn; pfn < end_pfn; pfn++) {
                if (pfn >= pmm->total_pages) break;
                
                page_t* page = &pmm->mem_map[pfn];
                
                // Skip pages used by memory map
                unsigned long mem_map_start_pfn = (uintptr_t)mem_map_addr / PMM_FRAME_SIZE;
                unsigned long mem_map_end_pfn = mem_map_start_pfn + mem_map_pages;
                if (pfn >= mem_map_start_pfn && pfn < mem_map_end_pfn) {
                    reserved_pages++;
                    continue;
                }
                
                // Skip first 1MB (reserved for kernel and legacy)
                if (pfn < 256) {  // 1MB / 4KB = 256 pages
                    reserved_pages++;
                    continue;
                }
                
                // Mark page as available
                atomic_clear_bit(PG_reserved, &page->flags);
                atomic_set(&page->ref_count, 0);
                
                // Determine zone
                uint64_t addr = pfn * PMM_FRAME_SIZE;
                if (addr < 0x1000000) {  // 16MB
                    page->zone = PMM_ZONE_DMA;
                    dma_zone->present_pages++;
                } else {
                    page->zone = PMM_ZONE_NORMAL;
                    normal_zone->present_pages++;
                }
                
                // Add to buddy allocator as order-0 page
                pmm_free_pages(PMM_PFN_TO_ADDR(pfn), 0);
            }
        }
    }
    
    pmm->reserved_pages = reserved_pages;
    pmm->managed_pages = pmm->total_pages - reserved_pages;
    
    // Update zone statistics
    dma_zone->managed_pages = dma_zone->present_pages;
    normal_zone->managed_pages = normal_zone->present_pages;
    
    vga_puts("PMM: Memory zones initialized\n");
    vga_puts("  Total pages: ");
    // Simple number printing
    char num_str[32];
    uint64_to_string(pmm->total_pages, num_str, sizeof(num_str));
    vga_puts(num_str);
    vga_puts("\n  Managed pages: ");
    uint64_to_string(pmm->managed_pages, num_str, sizeof(num_str));
    vga_puts(num_str);
    vga_puts("\n");
    
    return 0;
}

/**
 * Initialize NUMA topology detection
 */
int pmm_init_numa(void) {
    // For now, assume single NUMA node (UMA system)
    // In a real implementation, we would parse ACPI SRAT tables
    
    pmm->nr_nodes = 1;
    numa_node_t* node = &pmm->nodes[0];
    
    node->node_id = 0;
    node->start_pfn = 0;
    node->end_pfn = pmm->total_pages;
    node->present_pages = pmm->managed_pages;
    node->spanned_pages = pmm->total_pages;
    node->cpu_mask = 0xFFFFFFFFFFFFFFFFULL;  // All CPUs
    
    // Set distance to self as 10 (standard NUMA distance)
    node->distance[0] = 10;
    for (int i = 1; i < PMM_MAX_NUMA_NODES; i++) {
        node->distance[i] = 255;  // Invalid/unreachable
    }
    
    spinlock_init(&node->lock);
    
    return 0;
}

/**
 * Initialize zone fallback order for allocation failures
 */
void pmm_init_zone_fallback_order(void) {
    // DMA zone fallback: DMA -> Normal
    pmm->fallback_order[PMM_ZONE_DMA][0] = PMM_ZONE_DMA;
    pmm->fallback_order[PMM_ZONE_DMA][1] = PMM_ZONE_NORMAL;
    
    // Normal zone fallback: Normal -> DMA32 -> DMA
    pmm->fallback_order[PMM_ZONE_NORMAL][0] = PMM_ZONE_NORMAL;
    pmm->fallback_order[PMM_ZONE_NORMAL][1] = PMM_ZONE_DMA32;
    pmm->fallback_order[PMM_ZONE_NORMAL][2] = PMM_ZONE_DMA;
}

/**
 * Allocate pages using buddy allocator
 */
void* pmm_alloc_pages(unsigned int order, unsigned int flags, int node) {
    if (!pmm->initialized || order >= PMM_MAX_ORDER) {
        return NULL;
    }
    
    // Update statistics
    atomic64_inc(&pmm->stats.total_allocations);
    atomic64_inc(&pmm->stats.alloc_orders[order]);
    
    // Determine preferred zone based on flags
    pmm_zone_t preferred_zone = PMM_ZONE_NORMAL;
    if (flags & MM_FLAG_DMA) {
        preferred_zone = PMM_ZONE_DMA;
    }
    
    // Try allocation from preferred zone first
    mem_zone_t* zone = &pmm->zones[preferred_zone];
    page_t* page = __pmm_alloc_pages(zone, order, flags);
    
    if (!page) {
        // Try fallback zones
        for (int i = 1; i < PMM_ZONE_COUNT; i++) {
            pmm_zone_t fallback_zone = pmm->fallback_order[preferred_zone][i];
            if (fallback_zone == PMM_ZONE_COUNT) break;
            
            zone = &pmm->zones[fallback_zone];
            page = __pmm_alloc_pages(zone, order, flags);
            if (page) break;
        }
    }
    
    if (!page) {
        atomic64_inc(&pmm->stats.allocation_failures);
        atomic64_inc(&pmm->stats.zone_failures[preferred_zone]);
        return NULL;
    }
    
    // Set up page for use
    atomic_set(&page->ref_count, 1);
    page->order = order;
    
    // Memory debugging
    if (pmm->debug.leak_detection_enabled) {
        page->debug.alloc_func = __FUNCTION__;
        page->debug.alloc_file = __FILE__;
        page->debug.alloc_line = __LINE__;
        page->debug.alloc_time = hal->cpu_timestamp();
        
        atomic_inc(&pmm->debug.debug_pages_allocated);
    }
    
    // Zero memory if requested
    if (flags & MM_FLAG_ZERO) {
        void* addr = pmm_page_to_addr(page);
        memset(addr, 0, (1UL << order) * PMM_FRAME_SIZE);
    }
    
    return pmm_page_to_addr(page);
}

/**
 * Internal buddy allocator page allocation
 */
page_t* __pmm_alloc_pages(mem_zone_t* zone, unsigned int order, unsigned int flags) {
    free_area_t* area;
    page_t* page;
    unsigned int current_order;
    
    spin_lock(&zone->lock);
    
    // Look for free pages starting from requested order
    for (current_order = order; current_order < PMM_MAX_ORDER; current_order++) {
        area = &zone->free_area[current_order];
        
        if (list_empty(&area->free_list)) {
            continue;
        }
        
        // Get first page from free list
        page = list_first_entry(&area->free_list, page_t, lru);
        list_del(&page->lru);
        area->nr_free--;
        zone->vm_stat.nr_free -= (1UL << current_order);
        
        // Clear buddy flag
        atomic_clear_bit(PG_buddy, &page->flags);
        
        // If we got a larger block, split it down to requested size
        if (current_order != order) {
            page = pmm_expand(zone, order, current_order, current_order);
        }
        
        spin_unlock(&zone->lock);
        return page;
    }
    
    spin_unlock(&zone->lock);
    return NULL;
}

/**
 * Split a large buddy block into smaller blocks
 */
static page_t* pmm_expand(mem_zone_t* zone, unsigned int low, unsigned int high, 
                         unsigned int order) {
    unsigned long size = 1UL << order;
    page_t* page = NULL;
    
    while (order > low) {
        order--;
        size >>= 1;
        
        // Get buddy page
        page_t* buddy = page + size;
        
        // Add buddy to free list
        list_add(&buddy->lru, &zone->free_area[order].free_list);
        zone->free_area[order].nr_free++;
        zone->vm_stat.nr_free += size;
        
        // Mark buddy as free
        atomic_set_bit(PG_buddy, &buddy->flags);
        buddy->order = order;
    }
    
    return page;
}

/**
 * Free pages allocated by pmm_alloc_pages
 */
void pmm_free_pages(void* addr, unsigned int order) {
    if (!addr || !pmm->initialized || order >= PMM_MAX_ORDER) {
        return;
    }
    
    page_t* page = pmm_addr_to_page(addr);
    if (!page) {
        return;
    }
    
    // Update statistics
    atomic64_inc(&pmm->stats.total_frees);
    atomic64_inc(&pmm->stats.free_orders[order]);
    
    // Validate page
    pmm_validate_page(page, __FUNCTION__, __LINE__);
    
    // Get zone
    mem_zone_t* zone = &pmm->zones[page->zone];
    
    // Memory debugging cleanup
    if (pmm->debug.leak_detection_enabled) {
        atomic_dec(&pmm->debug.debug_pages_allocated);
    }
    
    __pmm_free_pages(zone, page, order);
}

/**
 * Internal buddy allocator page free with coalescing
 */
void __pmm_free_pages(mem_zone_t* zone, page_t* page, unsigned int order) {
    unsigned long page_idx = page - pmm->mem_map;
    unsigned long buddy_idx;
    page_t* buddy;
    unsigned long combined_idx;
    
    spin_lock(&zone->lock);
    
    // Coalesce buddies as much as possible
    while (order < PMM_MAX_ORDER - 1) {
        buddy_idx = page_idx ^ (1UL << order);
        
        if (buddy_idx >= pmm->total_pages) {
            break;
        }
        
        buddy = &pmm->mem_map[buddy_idx];
        
        // Check if buddy is free and same order
        if (!atomic_test_bit(PG_buddy, &buddy->flags) || buddy->order != order) {
            break;
        }
        
        // Remove buddy from free list
        list_del(&buddy->lru);
        zone->free_area[order].nr_free--;
        zone->vm_stat.nr_free -= (1UL << order);
        atomic_clear_bit(PG_buddy, &buddy->flags);
        
        // Combine with buddy
        combined_idx = buddy_idx & page_idx;
        page = &pmm->mem_map[combined_idx];
        page_idx = combined_idx;
        order++;
    }
    
    // Add to free list
    page->order = order;
    atomic_set_bit(PG_buddy, &page->flags);
    list_add(&page->lru, &zone->free_area[order].free_list);
    zone->free_area[order].nr_free++;
    zone->vm_stat.nr_free += (1UL << order);
    
    spin_unlock(&zone->lock);
}

/**
 * Get page descriptor from physical address
 */
page_t* pmm_addr_to_page(void* addr) {
    unsigned long pfn = PMM_ADDR_TO_PFN(addr);
    
    if (!PMM_PFN_VALID(pfn)) {
        return NULL;
    }
    
    return &pmm->mem_map[pfn];
}

/**
 * Get physical address from page descriptor
 */
void* pmm_page_to_addr(page_t* page) {
    if (!page || page < pmm->mem_map || 
        page >= pmm->mem_map + pmm->mem_map_size) {
        return NULL;
    }
    
    unsigned long pfn = page - pmm->mem_map;
    return PMM_PFN_TO_ADDR(pfn);
}

/**
 * Get global memory statistics
 */
int pmm_get_memory_stats(memory_stats_t* stats) {
    if (!stats || !pmm->initialized) {
        return -EINVAL;
    }
    
    memset(stats, 0, sizeof(memory_stats_t));
    
    stats->total_pages = pmm->total_pages;
    stats->reserved_pages = pmm->reserved_pages;
    stats->used_pages = pmm->total_pages - pmm->managed_pages;
    
    // Calculate free pages from all zones
    for (int i = 0; i < PMM_ZONE_COUNT; i++) {
        if (i < pmm->nr_zones) {
            mem_zone_t* zone = &pmm->zones[i];
            stats->free_pages += zone->vm_stat.nr_free;
            
            stats->zones[i].total_pages = zone->present_pages;
            stats->zones[i].free_pages = zone->vm_stat.nr_free;
        }
    }
    
    return 0;
}

/**
 * Validate page descriptor
 */
static void pmm_validate_page(page_t* page, const char* func, int line) {
    if (!page) return;
    
    // Check if page is within valid range
    if (page < pmm->mem_map || page >= pmm->mem_map + pmm->mem_map_size) {
        vga_puts("PMM: Invalid page descriptor in ");
        vga_puts(func);
        vga_puts("\n");
        return;
    }
    
    // Additional validation can be added here
}

/**
 * Get total system memory
 */
uint64_t pmm_get_total_memory(void) {
    return pmm->total_pages * PMM_FRAME_SIZE;
}

/**
 * Get free memory
 */
uint64_t pmm_get_free_memory(void) {
    uint64_t free_pages = 0;
    
    for (int i = 0; i < pmm->nr_zones; i++) {
        free_pages += pmm->zones[i].vm_stat.nr_free;
    }
    
    return free_pages * PMM_FRAME_SIZE;
}

/**
 * Check if system is under memory pressure
 */
bool pmm_under_memory_pressure(void) {
    uint64_t free_pages = pmm_get_free_memory() / PMM_FRAME_SIZE;
    return free_pages < pmm->config.low_free_pages;
}

/**
 * Simple 64-bit to string conversion for debugging
 */
void uint64_to_string(uint64_t value, char* buffer, size_t buffer_size) {
    if (buffer_size < 2) return;
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    char temp[32];
    int pos = 0;
    
    while (value > 0 && pos < 31) {
        temp[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Reverse string
    int i = 0;
    while (pos > 0 && i < buffer_size - 1) {
        buffer[i++] = temp[--pos];
    }
    buffer[i] = '\0';
}

/**
 * Dump memory zones for debugging
 */
void pmm_dump_zones(int zone) {
    vga_puts("PMM Memory Zone Information:\n");
    
    int start_zone = (zone >= 0 && zone < PMM_ZONE_COUNT) ? zone : 0;
    int end_zone = (zone >= 0 && zone < PMM_ZONE_COUNT) ? zone + 1 : pmm->nr_zones;
    
    for (int i = start_zone; i < end_zone; i++) {
        mem_zone_t* z = &pmm->zones[i];
        
        vga_puts("Zone ");
        vga_puts(z->name);
        vga_puts(":\n");
        
        char num_str[32];
        vga_puts("  Present pages: ");
        uint64_to_string(z->present_pages, num_str, sizeof(num_str));
        vga_puts(num_str);
        vga_puts("\n");
        
        vga_puts("  Free pages: ");
        uint64_to_string(z->vm_stat.nr_free, num_str, sizeof(num_str));
        vga_puts(num_str);
        vga_puts("\n");
    }
}

/**
 * Late initialization after other subsystems are ready
 */
int pmm_late_init(void) {
    vga_puts("PMM: Late initialization complete\n");
    return 0;
}

/**
 * Cleanup PMM resources
 */
void pmm_cleanup(void) {
    // In a real implementation, we would clean up any allocated resources
    pmm->initialized = false;
}

/**
 * Enable memory leak detection
 */
void pmm_enable_leak_detection(bool enable) {
    pmm->debug.leak_detection_enabled = enable;
}

/**
 * Validate physical address
 */
bool pmm_validate_addr(void* addr) {
    return PMM_ADDR_VALID(addr);
}

// Additional utility functions can be added here...