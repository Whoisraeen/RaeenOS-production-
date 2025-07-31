/**
 * @file buffer_cache.h
 * @brief RaeenOS Advanced Buffer Cache System
 * 
 * High-performance buffer cache with:
 * - Write-back optimization with configurable sync intervals
 * - LRU eviction with smart prefetching
 * - NUMA-aware buffer allocation
 * - Compression and deduplication support
 * - Statistics and performance monitoring
 * 
 * Version: 2.0 - Production Ready
 * Performance Target: >10GB/s throughput, <1ms latency
 */

#ifndef BUFFER_CACHE_H
#define BUFFER_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include "../sync.h"
#include "../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Buffer cache configuration
#define BUFFER_CACHE_SIZE 65536        // Number of cache entries
#define BUFFER_DEFAULT_SIZE 4096       // Default buffer size (4KB)
#define BUFFER_MAX_SIZE 1048576        // Maximum buffer size (1MB)
#define BUFFER_MIN_SIZE 512            // Minimum buffer size
#define BUFFER_HASH_BUCKETS 16384      // Hash table size
#define BUFFER_WRITEBACK_INTERVAL 5000 // Writeback interval in ms
#define BUFFER_MAX_DIRTY_RATIO 20      // Max dirty buffers as percentage
#define BUFFER_READAHEAD_PAGES 32      // Read-ahead window in pages

// Buffer states
typedef enum {
    BUFFER_STATE_INVALID = 0,
    BUFFER_STATE_CLEAN,
    BUFFER_STATE_DIRTY,
    BUFFER_STATE_WRITEBACK,
    BUFFER_STATE_LOCKED,
    BUFFER_STATE_ERROR
} buffer_state_t;

// Buffer flags
#define BUFFER_FLAG_UPTODATE   0x01    // Buffer contains valid data
#define BUFFER_FLAG_DIRTY      0x02    // Buffer needs to be written
#define BUFFER_FLAG_LOCKED     0x04    // Buffer is locked
#define BUFFER_FLAG_WRITEBACK  0x08    // Buffer is being written
#define BUFFER_FLAG_READ_AHEAD 0x10    // Buffer was read ahead
#define BUFFER_FLAG_COMPRESSED 0x20    // Buffer is compressed
#define BUFFER_FLAG_ENCRYPTED  0x40    // Buffer is encrypted
#define BUFFER_FLAG_PINNED     0x80    // Buffer cannot be evicted

// I/O request structure
typedef struct buffer_io_request {
    uint64_t device_id;        // Device identifier
    uint64_t block_num;        // Block number
    size_t block_size;         // Block size
    void* data;                // Data buffer
    size_t data_size;          // Data size
    bool is_write;             // Write operation flag
    int priority;              // I/O priority
    
    // Completion callback
    void (*callback)(struct buffer_io_request* req, int status);
    void* callback_data;
    
    // Request state
    int status;                // Completion status
    uint64_t submit_time;      // When request was submitted
    uint64_t complete_time;    // When request completed
} buffer_io_request_t;

// Buffer head structure
typedef struct buffer_head {
    uint64_t device_id;        // Device identifier
    uint64_t block_num;        // Block number on device
    size_t block_size;         // Size of the block
    uint32_t flags;            // Buffer flags
    buffer_state_t state;      // Buffer state
    
    void* data;                // Buffer data
    size_t data_size;          // Actual data size
    
    // Reference counting
    atomic_t ref_count;        // Reference count
    spinlock_t lock;           // Buffer lock
    
    // LRU management
    struct buffer_head* lru_next;
    struct buffer_head* lru_prev;
    uint64_t last_access;      // Last access timestamp
    uint32_t access_count;     // Access frequency counter
    
    // Hash table linkage
    struct buffer_head* hash_next;
    struct buffer_head* hash_prev;
    
    // Dirty list linkage
    struct buffer_head* dirty_next;
    struct buffer_head* dirty_prev;
    uint64_t dirty_time;       // When buffer became dirty
    
    // I/O state
    buffer_io_request_t* pending_io; // Pending I/O request
    struct list_head io_waiters;     // Threads waiting for I/O
    
    // Statistics
    uint64_t read_count;       // Number of reads
    uint64_t write_count;      // Number of writes
    uint64_t hit_count;        // Cache hit count
    
    // Compression/encryption
    void* compressed_data;     // Compressed data
    size_t compressed_size;    // Compressed data size
    uint32_t checksum;         // Data integrity checksum
} buffer_head_t;

// Buffer cache statistics
typedef struct buffer_cache_stats {
    uint64_t total_buffers;    // Total buffers allocated
    uint64_t cached_buffers;   // Buffers in cache
    uint64_t dirty_buffers;    // Dirty buffers
    uint64_t locked_buffers;   // Locked buffers
    
    uint64_t cache_hits;       // Cache hits
    uint64_t cache_misses;     // Cache misses
    uint64_t read_requests;    // Read requests
    uint64_t write_requests;   // Write requests
    uint64_t sync_requests;    // Sync requests
    
    uint64_t readahead_hits;   // Read-ahead hits
    uint64_t readahead_misses; // Read-ahead misses
    uint64_t evictions;        // Buffer evictions
    uint64_t writebacks;       // Write-back operations
    
    uint64_t bytes_read;       // Total bytes read
    uint64_t bytes_written;    // Total bytes written
    uint64_t compression_saves; // Bytes saved by compression
    
    // Performance metrics
    uint64_t avg_read_latency; // Average read latency (ns)
    uint64_t avg_write_latency; // Average write latency (ns)
    uint32_t hit_ratio;        // Cache hit ratio (percentage)
    uint32_t dirty_ratio;      // Dirty buffer ratio (percentage)
} buffer_cache_stats_t;

// Buffer cache configuration
typedef struct buffer_cache_config {
    size_t max_buffers;        // Maximum number of buffers
    size_t max_dirty_buffers;  // Maximum dirty buffers
    uint32_t writeback_interval; // Writeback interval (ms)
    uint32_t sync_interval;    // Sync interval (ms)
    bool compression_enabled;  // Enable compression
    bool encryption_enabled;   // Enable encryption
    uint32_t readahead_pages;  // Read-ahead window
    uint32_t dirty_ratio_limit; // Dirty ratio limit (%)
} buffer_cache_config_t;

// Global buffer cache instance
extern buffer_cache_stats_t buffer_cache_stats;
extern buffer_cache_config_t buffer_cache_config;

// Core buffer cache API

/**
 * Initialize buffer cache system
 */
int buffer_cache_init(const buffer_cache_config_t* config);

/**
 * Shutdown buffer cache system
 */
void buffer_cache_shutdown(void);

/**
 * Get a buffer from cache or allocate new one
 */
buffer_head_t* buffer_cache_get(uint64_t device_id, uint64_t block_num, size_t block_size);

/**
 * Put buffer back to cache (decrease reference count)
 */
void buffer_cache_put(buffer_head_t* bh);

/**
 * Read data into buffer
 */
int buffer_cache_read(buffer_head_t* bh);

/**
 * Write buffer data to storage
 */
int buffer_cache_write(buffer_head_t* bh);

/**
 * Mark buffer as dirty
 */
void buffer_cache_mark_dirty(buffer_head_t* bh);

/**
 * Sync all dirty buffers for a device
 */
int buffer_cache_sync_device(uint64_t device_id);

/**
 * Sync all dirty buffers
 */
int buffer_cache_sync_all(void);

/**
 * Invalidate all buffers for a device
 */
void buffer_cache_invalidate_device(uint64_t device_id);

/**
 * Invalidate specific buffer
 */
void buffer_cache_invalidate(buffer_head_t* bh);

/**
 * Flush dirty buffers to storage
 */
int buffer_cache_flush(uint64_t device_id);

/**
 * Pre-read buffers (read-ahead)
 */
int buffer_cache_readahead(uint64_t device_id, uint64_t start_block, size_t count);

// Advanced features

/**
 * Compress buffer data
 */
int buffer_cache_compress(buffer_head_t* bh);

/**
 * Decompress buffer data
 */
int buffer_cache_decompress(buffer_head_t* bh);

/**
 * Encrypt buffer data
 */
int buffer_cache_encrypt(buffer_head_t* bh, const uint8_t* key);

/**
 * Decrypt buffer data
 */
int buffer_cache_decrypt(buffer_head_t* bh, const uint8_t* key);

/**
 * Calculate buffer checksum
 */
uint32_t buffer_cache_checksum(buffer_head_t* bh);

/**
 * Verify buffer integrity
 */
bool buffer_cache_verify(buffer_head_t* bh);

// Statistics and monitoring

/**
 * Get cache statistics
 */
int buffer_cache_get_stats(buffer_cache_stats_t* stats);

/**
 * Reset cache statistics
 */
void buffer_cache_reset_stats(void);

/**
 * Get cache hit ratio
 */
uint32_t buffer_cache_hit_ratio(void);

/**
 * Get dirty buffer ratio
 */
uint32_t buffer_cache_dirty_ratio(void);

/**
 * Shrink cache (free unused buffers)
 */
int buffer_cache_shrink(size_t target_count);

/**
 * Get cache memory usage
 */
size_t buffer_cache_memory_usage(void);

// Configuration management

/**
 * Update cache configuration
 */
int buffer_cache_configure(const buffer_cache_config_t* config);

/**
 * Get current configuration
 */
int buffer_cache_get_config(buffer_cache_config_t* config);

/**
 * Enable/disable compression
 */
void buffer_cache_set_compression(bool enabled);

/**
 * Enable/disable encryption
 */
void buffer_cache_set_encryption(bool enabled);

/**
 * Set read-ahead window size
 */
void buffer_cache_set_readahead(uint32_t pages);

// Internal functions (for debugging and advanced usage)

/**
 * Lock buffer (exclusive access)
 */
int buffer_cache_lock(buffer_head_t* bh);

/**
 * Unlock buffer
 */
void buffer_cache_unlock(buffer_head_t* bh);

/**
 * Wait for buffer I/O completion
 */
int buffer_cache_wait_io(buffer_head_t* bh);

/**
 * Check if buffer is up to date
 */
bool buffer_cache_uptodate(buffer_head_t* bh);

/**
 * Check if buffer is dirty
 */
bool buffer_cache_dirty(buffer_head_t* bh);

/**
 * Check if buffer is locked
 */
bool buffer_cache_locked(buffer_head_t* bh);

/**
 * Pin buffer in memory (prevent eviction)
 */
void buffer_cache_pin(buffer_head_t* bh);

/**
 * Unpin buffer
 */
void buffer_cache_unpin(buffer_head_t* bh);

// Error codes
#define BUFFER_SUCCESS          0
#define BUFFER_ERR_NO_MEMORY   -3001
#define BUFFER_ERR_IO_ERROR    -3002
#define BUFFER_ERR_INVALID_ARG -3003
#define BUFFER_ERR_BUSY        -3004
#define BUFFER_ERR_TIMEOUT     -3005
#define BUFFER_ERR_CORRUPTED   -3006
#define BUFFER_ERR_LOCKED      -3007
#define BUFFER_ERR_NOT_FOUND   -3008

#ifdef __cplusplus
}
#endif

#endif // BUFFER_CACHE_H