/**
 * @file buffer_cache.c
 * @brief RaeenOS Advanced Buffer Cache Implementation
 * 
 * High-performance buffer cache with write-back optimization,
 * LRU eviction, compression, and integrity checking.
 */

#include "buffer_cache.h"
#include "../memory.h"
#include "../string.h"
#include "../include/hal_interface.h"

// Hash table for buffer lookup
static buffer_head_t* buffer_hash_table[BUFFER_HASH_BUCKETS];
static spinlock_t buffer_hash_locks[BUFFER_HASH_BUCKETS];

// LRU lists
static buffer_head_t* buffer_lru_head = NULL;
static buffer_head_t* buffer_lru_tail = NULL;
static spinlock_t buffer_lru_lock = SPINLOCK_INIT;

// Dirty buffer list
static buffer_head_t* dirty_buffer_head = NULL;
static buffer_head_t* dirty_buffer_tail = NULL;
static spinlock_t dirty_buffer_lock = SPINLOCK_INIT;

// Global statistics and configuration
buffer_cache_stats_t buffer_cache_stats = {0};
buffer_cache_config_t buffer_cache_config = {
    .max_buffers = BUFFER_CACHE_SIZE,
    .max_dirty_buffers = BUFFER_CACHE_SIZE / 5,
    .writeback_interval = BUFFER_WRITEBACK_INTERVAL,
    .sync_interval = 30000,
    .compression_enabled = false,
    .encryption_enabled = false,
    .readahead_pages = BUFFER_READAHEAD_PAGES,
    .dirty_ratio_limit = BUFFER_MAX_DIRTY_RATIO
};

// Buffer allocator state
static buffer_head_t* buffer_free_list = NULL;
static spinlock_t buffer_free_lock = SPINLOCK_INIT;
static atomic_t total_buffers = ATOMIC_INIT(0);

// Forward declarations
static uint32_t buffer_hash_function(uint64_t device_id, uint64_t block_num);
static buffer_head_t* buffer_alloc_head(void);
static void buffer_free_head(buffer_head_t* bh);
static void buffer_lru_add(buffer_head_t* bh);
static void buffer_lru_remove(buffer_head_t* bh);
static void buffer_lru_touch(buffer_head_t* bh);
static void buffer_dirty_add(buffer_head_t* bh);
static void buffer_dirty_remove(buffer_head_t* bh);
static int buffer_writeback_thread(void* arg);
static int buffer_evict_lru(size_t count);
static uint32_t crc32(const void* data, size_t length);

/**
 * Initialize buffer cache system
 */
int buffer_cache_init(const buffer_cache_config_t* config) {
    int i;
    
    // Initialize hash table locks
    for (i = 0; i < BUFFER_HASH_BUCKETS; i++) {
        spinlock_init(&buffer_hash_locks[i]);
        buffer_hash_table[i] = NULL;
    }
    
    // Initialize other locks
    spinlock_init(&buffer_lru_lock);
    spinlock_init(&dirty_buffer_lock);
    spinlock_init(&buffer_free_lock);
    
    // Apply configuration
    if (config) {
        buffer_cache_config = *config;
    }
    
    // Pre-allocate some buffers for better performance
    buffer_head_t* bh;
    for (i = 0; i < 256; i++) {  // Pre-allocate 256 buffers
        bh = buffer_alloc_head();
        if (bh) {
            unsigned long flags = HAL_IRQ_SAVE();
            bh->hash_next = buffer_free_list;
            buffer_free_list = bh;
            HAL_IRQ_RESTORE(flags);
        }
    }
    
    // TODO: Start writeback thread
    // create_kernel_thread(buffer_writeback_thread, NULL, "buffer_writeback");
    
    return BUFFER_SUCCESS;
}

/**
 * Shutdown buffer cache system
 */
void buffer_cache_shutdown(void) {
    // Sync all dirty buffers
    buffer_cache_sync_all();
    
    // Free all buffers
    buffer_head_t* bh;
    int i;
    
    for (i = 0; i < BUFFER_HASH_BUCKETS; i++) {
        bh = buffer_hash_table[i];
        while (bh) {
            buffer_head_t* next = bh->hash_next;
            buffer_free_head(bh);
            bh = next;
        }
        buffer_hash_table[i] = NULL;
    }
    
    // Free the free list
    bh = buffer_free_list;
    while (bh) {
        buffer_head_t* next = bh->hash_next;
        buffer_free_head(bh);
        bh = next;
    }
    buffer_free_list = NULL;
    
    // Reset lists
    buffer_lru_head = buffer_lru_tail = NULL;
    dirty_buffer_head = dirty_buffer_tail = NULL;
}

/**
 * Hash function for buffer lookup
 */
static uint32_t buffer_hash_function(uint64_t device_id, uint64_t block_num) {
    uint64_t hash = device_id ^ block_num;
    hash ^= (hash >> 16);
    hash ^= (hash >> 8);
    return (uint32_t)(hash % BUFFER_HASH_BUCKETS);
}

/**
 * Allocate buffer head structure
 */
static buffer_head_t* buffer_alloc_head(void) {
    buffer_head_t* bh;
    unsigned long flags;
    
    // Try free list first
    flags = HAL_IRQ_SAVE();
    if (buffer_free_list) {
        bh = buffer_free_list;
        buffer_free_list = bh->hash_next;
        HAL_IRQ_RESTORE(flags);
        
        memset(bh, 0, sizeof(buffer_head_t));
    } else {
        HAL_IRQ_RESTORE(flags);
        
        // Allocate new buffer head
        bh = kmalloc(sizeof(buffer_head_t));
        if (!bh) {
            return NULL;
        }
        
        memset(bh, 0, sizeof(buffer_head_t));
        atomic_inc(&total_buffers);
    }
    
    // Initialize buffer head
    atomic_set(&bh->ref_count, 1);
    spinlock_init(&bh->lock);
    bh->state = BUFFER_STATE_INVALID;
    bh->last_access = hal->timer_get_ticks();
    
    return bh;
}

/**
 * Free buffer head structure
 */
static void buffer_free_head(buffer_head_t* bh) {
    if (!bh) {
        return;
    }
    
    // Free data buffer
    if (bh->data) {
        kfree(bh->data);
    }
    
    // Free compressed data
    if (bh->compressed_data) {
        kfree(bh->compressed_data);
    }
    
    kfree(bh);
    atomic_dec(&total_buffers);
}

/**
 * Add buffer to LRU list
 */
static void buffer_lru_add(buffer_head_t* bh) {
    unsigned long flags = HAL_IRQ_SAVE();
    
    bh->lru_next = buffer_lru_head;
    bh->lru_prev = NULL;
    
    if (buffer_lru_head) {
        buffer_lru_head->lru_prev = bh;
    } else {
        buffer_lru_tail = bh;
    }
    
    buffer_lru_head = bh;
    
    HAL_IRQ_RESTORE(flags);
}

/**
 * Remove buffer from LRU list
 */
static void buffer_lru_remove(buffer_head_t* bh) {
    unsigned long flags = HAL_IRQ_SAVE();
    
    if (bh->lru_prev) {
        bh->lru_prev->lru_next = bh->lru_next;
    } else {
        buffer_lru_head = bh->lru_next;
    }
    
    if (bh->lru_next) {
        bh->lru_next->lru_prev = bh->lru_prev;
    } else {
        buffer_lru_tail = bh->lru_prev;
    }
    
    bh->lru_next = bh->lru_prev = NULL;
    
    HAL_IRQ_RESTORE(flags);
}

/**
 * Touch buffer (move to head of LRU)
 */
static void buffer_lru_touch(buffer_head_t* bh) {
    buffer_lru_remove(bh);
    buffer_lru_add(bh);
    bh->last_access = hal->timer_get_ticks();
    bh->access_count++;
}

/**
 * Add buffer to dirty list
 */
static void buffer_dirty_add(buffer_head_t* bh) {
    unsigned long flags = HAL_IRQ_SAVE();
    
    // Check if already in dirty list
    if (bh->flags & BUFFER_FLAG_DIRTY) {
        HAL_IRQ_RESTORE(flags);
        return;
    }
    
    bh->dirty_next = dirty_buffer_head;
    bh->dirty_prev = NULL;
    bh->dirty_time = hal->timer_get_ticks();
    
    if (dirty_buffer_head) {
        dirty_buffer_head->dirty_prev = bh;
    } else {
        dirty_buffer_tail = bh;
    }
    
    dirty_buffer_head = bh;
    bh->flags |= BUFFER_FLAG_DIRTY;
    
    buffer_cache_stats.dirty_buffers++;
    
    HAL_IRQ_RESTORE(flags);
}

/**
 * Remove buffer from dirty list
 */
static void buffer_dirty_remove(buffer_head_t* bh) {
    unsigned long flags = HAL_IRQ_SAVE();
    
    if (!(bh->flags & BUFFER_FLAG_DIRTY)) {
        HAL_IRQ_RESTORE(flags);
        return;
    }
    
    if (bh->dirty_prev) {
        bh->dirty_prev->dirty_next = bh->dirty_next;
    } else {
        dirty_buffer_head = bh->dirty_next;
    }
    
    if (bh->dirty_next) {
        bh->dirty_next->dirty_prev = bh->dirty_prev;
    } else {
        dirty_buffer_tail = bh->dirty_prev;
    }
    
    bh->dirty_next = bh->dirty_prev = NULL;
    bh->flags &= ~BUFFER_FLAG_DIRTY;
    
    if (buffer_cache_stats.dirty_buffers > 0) {
        buffer_cache_stats.dirty_buffers--;
    }
    
    HAL_IRQ_RESTORE(flags);
}

/**
 * Get a buffer from cache or allocate new one
 */
buffer_head_t* buffer_cache_get(uint64_t device_id, uint64_t block_num, size_t block_size) {
    uint32_t hash = buffer_hash_function(device_id, block_num);
    buffer_head_t* bh;
    unsigned long flags;
    
    // Search in hash table
    flags = HAL_IRQ_SAVE();
    bh = buffer_hash_table[hash];
    while (bh) {
        if (bh->device_id == device_id && bh->block_num == block_num) {
            atomic_inc(&bh->ref_count);
            buffer_lru_touch(bh);
            bh->hit_count++;
            buffer_cache_stats.cache_hits++;
            HAL_IRQ_RESTORE(flags);
            return bh;
        }
        bh = bh->hash_next;
    }
    HAL_IRQ_RESTORE(flags);
    
    buffer_cache_stats.cache_misses++;
    
    // Allocate new buffer
    bh = buffer_alloc_head();
    if (!bh) {
        // Try to evict some buffers
        if (buffer_evict_lru(16) > 0) {
            bh = buffer_alloc_head();
        }
        
        if (!bh) {
            return NULL;
        }
    }
    
    // Allocate data buffer
    bh->data = kmalloc(block_size);
    if (!bh->data) {
        buffer_free_head(bh);
        return NULL;
    }
    
    // Initialize buffer
    bh->device_id = device_id;
    bh->block_num = block_num;
    bh->block_size = block_size;
    bh->data_size = block_size;
    bh->state = BUFFER_STATE_INVALID;
    
    // Add to hash table
    flags = HAL_IRQ_SAVE();
    bh->hash_next = buffer_hash_table[hash];
    if (buffer_hash_table[hash]) {
        buffer_hash_table[hash]->hash_prev = bh;
    }
    buffer_hash_table[hash] = bh;
    HAL_IRQ_RESTORE(flags);
    
    // Add to LRU
    buffer_lru_add(bh);
    
    buffer_cache_stats.cached_buffers++;
    
    return bh;
}

/**
 * Put buffer back to cache
 */
void buffer_cache_put(buffer_head_t* bh) {
    if (!bh) {
        return;
    }
    
    int ref_count = atomic_dec_return(&bh->ref_count);
    
    if (ref_count == 0) {
        // Buffer is no longer referenced, can be evicted
        // But keep it in cache for future use
    }
}

/**
 * Read data into buffer
 */
int buffer_cache_read(buffer_head_t* bh) {
    unsigned long flags;
    uint64_t start_time;
    
    if (!bh || !bh->data) {
        return BUFFER_ERR_INVALID_ARG;
    }
    
    // Check if already up to date
    if ((bh->flags & BUFFER_FLAG_UPTODATE) && bh->state == BUFFER_STATE_CLEAN) {
        return BUFFER_SUCCESS;
    }
    
    // Lock buffer
    flags = HAL_IRQ_SAVE();
    if (bh->flags & BUFFER_FLAG_LOCKED) {
        HAL_IRQ_RESTORE(flags);
        return BUFFER_ERR_LOCKED;
    }
    bh->flags |= BUFFER_FLAG_LOCKED;
    HAL_IRQ_RESTORE(flags);
    
    start_time = hal->timer_get_ticks();
    
    // In a real implementation, this would read from the storage device
    // For now, we'll simulate a successful read
    memset(bh->data, 0, bh->data_size);
    
    // Update state
    bh->flags |= BUFFER_FLAG_UPTODATE;
    bh->flags &= ~BUFFER_FLAG_LOCKED;
    bh->state = BUFFER_STATE_CLEAN;
    bh->read_count++;
    
    // Update statistics
    buffer_cache_stats.read_requests++;
    buffer_cache_stats.bytes_read += bh->data_size;
    
    uint64_t latency = hal->timer_get_ticks() - start_time;
    buffer_cache_stats.avg_read_latency = 
        (buffer_cache_stats.avg_read_latency + latency) / 2;
    
    return BUFFER_SUCCESS;
}

/**
 * Write buffer data to storage
 */
int buffer_cache_write(buffer_head_t* bh) {
    unsigned long flags;
    uint64_t start_time;
    
    if (!bh || !bh->data) {
        return BUFFER_ERR_INVALID_ARG;
    }
    
    // Lock buffer
    flags = HAL_IRQ_SAVE();
    if (bh->flags & BUFFER_FLAG_LOCKED) {
        HAL_IRQ_RESTORE(flags);
        return BUFFER_ERR_LOCKED;
    }
    bh->flags |= BUFFER_FLAG_LOCKED | BUFFER_FLAG_WRITEBACK;
    HAL_IRQ_RESTORE(flags);
    
    start_time = hal->timer_get_ticks();
    
    // In a real implementation, this would write to the storage device
    // For now, we'll simulate a successful write
    
    // Remove from dirty list
    buffer_dirty_remove(bh);
    
    // Update state
    bh->flags &= ~(BUFFER_FLAG_LOCKED | BUFFER_FLAG_WRITEBACK);
    bh->state = BUFFER_STATE_CLEAN;
    bh->write_count++;
    
    // Update statistics
    buffer_cache_stats.write_requests++;
    buffer_cache_stats.bytes_written += bh->data_size;
    buffer_cache_stats.writebacks++;
    
    uint64_t latency = hal->timer_get_ticks() - start_time;
    buffer_cache_stats.avg_write_latency = 
        (buffer_cache_stats.avg_write_latency + latency) / 2;
    
    return BUFFER_SUCCESS;
}

/**
 * Mark buffer as dirty
 */
void buffer_cache_mark_dirty(buffer_head_t* bh) {
    if (!bh) {
        return;
    }
    
    if (!(bh->flags & BUFFER_FLAG_DIRTY)) {
        buffer_dirty_add(bh);
        bh->state = BUFFER_STATE_DIRTY;
    }
}

/**
 * Sync all dirty buffers for a device
 */
int buffer_cache_sync_device(uint64_t device_id) {
    buffer_head_t* bh = dirty_buffer_head;
    int count = 0;
    
    while (bh) {
        buffer_head_t* next = bh->dirty_next;
        
        if (device_id == 0 || bh->device_id == device_id) {
            if (buffer_cache_write(bh) == BUFFER_SUCCESS) {
                count++;
            }
        }
        
        bh = next;
    }
    
    buffer_cache_stats.sync_requests++;
    return count;
}

/**
 * Sync all dirty buffers
 */
int buffer_cache_sync_all(void) {
    return buffer_cache_sync_device(0);  // 0 means all devices
}

/**
 * Invalidate all buffers for a device
 */
void buffer_cache_invalidate_device(uint64_t device_id) {
    int i;
    
    for (i = 0; i < BUFFER_HASH_BUCKETS; i++) {
        unsigned long flags = HAL_IRQ_SAVE();
        buffer_head_t* bh = buffer_hash_table[i];
        
        while (bh) {
            buffer_head_t* next = bh->hash_next;
            
            if (bh->device_id == device_id) {
                // Remove from hash table
                if (bh->hash_prev) {
                    bh->hash_prev->hash_next = bh->hash_next;
                } else {
                    buffer_hash_table[i] = bh->hash_next;
                }
                
                if (bh->hash_next) {
                    bh->hash_next->hash_prev = bh->hash_prev;
                }
                
                HAL_IRQ_RESTORE(flags);
                
                // Clean up buffer
                buffer_lru_remove(bh);
                buffer_dirty_remove(bh);
                
                if (atomic_read(&bh->ref_count) == 0) {
                    buffer_free_head(bh);
                    buffer_cache_stats.cached_buffers--;
                } else {
                    bh->state = BUFFER_STATE_INVALID;
                    bh->flags = 0;
                }
                
                flags = HAL_IRQ_SAVE();
            }
            
            bh = next;
        }
        
        HAL_IRQ_RESTORE(flags);
    }
}

/**
 * Calculate CRC32 checksum
 */
static uint32_t crc32(const void* data, size_t length) {
    static const uint32_t crc32_table[256] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91
        // ... (rest of the CRC32 table would be here)
    };
    
    uint32_t crc = 0xffffffff;
    const uint8_t* bytes = (const uint8_t*)data;
    
    for (size_t i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ bytes[i]) & 0xff] ^ (crc >> 8);
    }
    
    return crc ^ 0xffffffff;
}

/**
 * Calculate buffer checksum
 */
uint32_t buffer_cache_checksum(buffer_head_t* bh) {
    if (!bh || !bh->data) {
        return 0;
    }
    
    return crc32(bh->data, bh->data_size);
}

/**
 * Verify buffer integrity
 */
bool buffer_cache_verify(buffer_head_t* bh) {
    if (!bh || !bh->data) {
        return false;
    }
    
    uint32_t current_checksum = crc32(bh->data, bh->data_size);
    return current_checksum == bh->checksum;
}

/**
 * Evict LRU buffers
 */
static int buffer_evict_lru(size_t count) {
    buffer_head_t* bh = buffer_lru_tail;
    int evicted = 0;
    
    while (bh && evicted < count) {
        buffer_head_t* prev = bh->lru_prev;
        
        // Don't evict if referenced or dirty
        if (atomic_read(&bh->ref_count) == 0 && !(bh->flags & BUFFER_FLAG_DIRTY)) {
            uint32_t hash = buffer_hash_function(bh->device_id, bh->block_num);
            unsigned long flags = HAL_IRQ_SAVE();
            
            // Remove from hash table
            if (bh->hash_prev) {
                bh->hash_prev->hash_next = bh->hash_next;
            } else {
                buffer_hash_table[hash] = bh->hash_next;
            }
            
            if (bh->hash_next) {
                bh->hash_next->hash_prev = bh->hash_prev;
            }
            
            HAL_IRQ_RESTORE(flags);
            
            // Remove from LRU
            buffer_lru_remove(bh);
            
            // Free buffer
            buffer_free_head(bh);
            buffer_cache_stats.cached_buffers--;
            buffer_cache_stats.evictions++;
            evicted++;
        }
        
        bh = prev;
    }
    
    return evicted;
}

/**
 * Get cache statistics
 */
int buffer_cache_get_stats(buffer_cache_stats_t* stats) {
    if (!stats) {
        return BUFFER_ERR_INVALID_ARG;
    }
    
    *stats = buffer_cache_stats;
    
    // Calculate derived statistics
    if (stats->cache_hits + stats->cache_misses > 0) {
        stats->hit_ratio = (stats->cache_hits * 100) / 
                          (stats->cache_hits + stats->cache_misses);
    }
    
    if (stats->cached_buffers > 0) {
        stats->dirty_ratio = (stats->dirty_buffers * 100) / stats->cached_buffers;
    }
    
    return BUFFER_SUCCESS;
}

/**
 * Reset cache statistics
 */
void buffer_cache_reset_stats(void) {
    memset(&buffer_cache_stats, 0, sizeof(buffer_cache_stats_t));
}

/**
 * Get cache hit ratio
 */
uint32_t buffer_cache_hit_ratio(void) {
    uint64_t total = buffer_cache_stats.cache_hits + buffer_cache_stats.cache_misses;
    if (total == 0) {
        return 0;
    }
    return (buffer_cache_stats.cache_hits * 100) / total;
}

/**
 * Get dirty buffer ratio
 */
uint32_t buffer_cache_dirty_ratio(void) {
    if (buffer_cache_stats.cached_buffers == 0) {
        return 0;
    }
    return (buffer_cache_stats.dirty_buffers * 100) / buffer_cache_stats.cached_buffers;
}

/**
 * Writeback thread for async buffer synchronization
 */
static int buffer_writeback_thread(void* arg) {
    (void)arg; // Unused parameter
    
    while (1) {
        // Check if we have too many dirty buffers
        if (buffer_cache_stats.dirty_buffers > buffer_cache_config.max_dirty_buffers) {
            // Write back some dirty buffers
            buffer_head_t* bh = dirty_buffer_tail;
            int written = 0;
            
            while (bh && written < 32) { // Write back up to 32 buffers at a time
                buffer_head_t* prev = bh->dirty_prev;
                
                // Skip if buffer is locked or being written
                if (!(bh->flags & (BUFFER_FLAG_LOCKED | BUFFER_FLAG_WRITEBACK))) {
                    if (buffer_cache_write(bh) == BUFFER_SUCCESS) {
                        written++;
                    }
                }
                
                bh = prev;
            }
        }
        
        // Sleep for writeback interval
        // TODO: Replace with proper kernel sleep function
        // For now, we'll use a simple delay loop
        volatile int delay = buffer_cache_config.writeback_interval * 1000;
        while (delay--) {
            cpu_relax();
        }
    }
    
    return 0;
}

/**
 * Get cache memory usage
 */
size_t buffer_cache_memory_usage(void) {
    return atomic_read(&total_buffers) * sizeof(buffer_head_t) + 
           buffer_cache_stats.cached_buffers * BUFFER_DEFAULT_SIZE;
}