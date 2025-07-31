/**
 * @file vfs_events.h
 * @brief RaeenOS Advanced Filesystem Event Notification System
 * 
 * Enterprise-grade event system providing:
 * - Real-time filesystem monitoring with low latency
 * - Hierarchical event filtering and routing
 * - High-performance event delivery with batching
 * - Security audit trail integration
 * - User-space and kernel-space event delivery
 * - Event aggregation and rate limiting
 * - Memory-efficient event storage and delivery
 * 
 * Version: 2.0 - Production Ready
 * Performance Target: >1M events/sec, <100μs delivery latency
 * Security Level: Enterprise Grade
 */

#ifndef VFS_EVENTS_H
#define VFS_EVENTS_H

#include <stdint.h>
#include <stdbool.h>
#include "../include/types.h"
#include "../sync.h"
#include "../include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Event types (bit flags for efficient filtering)
#define VFS_EVENT_CREATE         0x00000001  // File/directory created
#define VFS_EVENT_DELETE         0x00000002  // File/directory deleted
#define VFS_EVENT_MODIFY         0x00000004  // File content modified
#define VFS_EVENT_METADATA       0x00000008  // Metadata changed (chmod, chown, etc.)
#define VFS_EVENT_MOVE           0x00000010  // File/directory moved/renamed
#define VFS_EVENT_OPEN           0x00000020  // File opened
#define VFS_EVENT_CLOSE          0x00000040  // File closed
#define VFS_EVENT_ACCESS         0x00000080  // File accessed (read)
#define VFS_EVENT_MOUNT          0x00000100  // Filesystem mounted
#define VFS_EVENT_UNMOUNT        0x00000200  // Filesystem unmounted
#define VFS_EVENT_LINK           0x00000400  // Hard link created
#define VFS_EVENT_UNLINK         0x00000800  // Hard link removed
#define VFS_EVENT_SYMLINK        0x00001000  // Symbolic link created
#define VFS_EVENT_TRUNCATE       0x00002000  // File truncated
#define VFS_EVENT_SETXATTR       0x00004000  // Extended attribute set
#define VFS_EVENT_REMOVEXATTR    0x00008000  // Extended attribute removed
#define VFS_EVENT_LOCK           0x00010000  // File lock acquired
#define VFS_EVENT_UNLOCK         0x00020000  // File lock released
#define VFS_EVENT_MMAP           0x00040000  // File memory mapped
#define VFS_EVENT_SYNC           0x00080000  // File/filesystem synced
#define VFS_EVENT_ERROR          0x00100000  // Error occurred
#define VFS_EVENT_SECURITY       0x00200000  // Security-related event
#define VFS_EVENT_QUOTA          0x00400000  // Quota-related event
#define VFS_EVENT_SNAPSHOT       0x00800000  // Snapshot created/deleted

// Event priorities
typedef enum {
    VFS_EVENT_PRIORITY_LOW = 0,
    VFS_EVENT_PRIORITY_NORMAL = 1,
    VFS_EVENT_PRIORITY_HIGH = 2,
    VFS_EVENT_PRIORITY_CRITICAL = 3
} vfs_event_priority_t;

// Event delivery modes
typedef enum {
    VFS_EVENT_DELIVERY_SYNC = 0,    // Synchronous delivery
    VFS_EVENT_DELIVERY_ASYNC = 1,   // Asynchronous delivery
    VFS_EVENT_DELIVERY_BATCH = 2    // Batched delivery
} vfs_event_delivery_t;

// Event source types
typedef enum {
    VFS_EVENT_SOURCE_KERNEL = 0,    // Kernel-generated event
    VFS_EVENT_SOURCE_USER = 1,      // User-space generated event
    VFS_EVENT_SOURCE_NETWORK = 2,   // Network filesystem event
    VFS_EVENT_SOURCE_SECURITY = 3   // Security subsystem event
} vfs_event_source_t;

// Forward declarations
struct vfs_inode;
struct vfs_dentry;
struct vfs_file;
struct vfs_event;
struct vfs_event_watcher;
struct vfs_event_filter;

// Event data structure
typedef struct vfs_event {
    uint64_t id;                    // Unique event ID
    uint32_t type;                  // Event type (bit mask)
    vfs_event_priority_t priority;  // Event priority
    vfs_event_source_t source;      // Event source
    
    // Timing
    uint64_t timestamp;             // Event timestamp (nanoseconds)
    uint64_t sequence;              // Sequence number for ordering
    
    // File system objects
    struct vfs_inode* inode;        // Associated inode (if any)
    struct vfs_dentry* dentry;      // Associated dentry (if any)
    struct vfs_file* file;          // Associated file (if any)
    
    // Path information
    char path[VFS_PATH_MAX];        // Full path of affected object
    char old_path[VFS_PATH_MAX];    // Old path (for move operations)
    
    // Process information
    pid_t pid;                      // Process ID that caused event
    uid_t uid;                      // User ID
    gid_t gid;                      // Group ID
    
    // Event-specific data
    union {
        struct {
            uint64_t old_size;      // Previous file size
            uint64_t new_size;      // New file size
        } modify;
        
        struct {
            mode_t old_mode;        // Previous mode
            mode_t new_mode;        // New mode
            uid_t old_uid;          // Previous owner
            uid_t new_uid;          // New owner
            gid_t old_gid;          // Previous group
            gid_t new_gid;          // New group
        } metadata;
        
        struct {
            uint32_t flags;         // Open flags
            mode_t mode;            // Open mode
        } open;
        
        struct {
            int error_code;         // Error code
            char description[256];  // Error description
        } error;
        
        struct {
            char label[256];        // Security label
            uint32_t action;        // Security action
        } security;
        
        uint8_t raw[512];           // Raw event data
    } data;
    
    // Event metadata
    uint32_t flags;                 // Event flags
    size_t data_size;               // Size of event-specific data
    void* extra_data;               // Additional data (dynamically allocated)
    
    // List linkage
    struct vfs_event* next;         // Next event in queue
    struct vfs_event* prev;         // Previous event in queue
    
    // Reference counting
    atomic_t ref_count;             // Reference count
    
    // Memory pool linkage
    struct vfs_event* pool_next;    // Next in memory pool
} vfs_event_t;

// Event filter specification
typedef struct vfs_event_filter {
    uint32_t event_mask;            // Event types to watch
    char path_pattern[VFS_PATH_MAX]; // Path pattern (supports wildcards)
    pid_t pid_filter;               // Filter by process ID (0 = all)
    uid_t uid_filter;               // Filter by user ID (-1 = all)
    gid_t gid_filter;               // Filter by group ID (-1 = all)
    
    // Advanced filtering
    bool recursive;                 // Watch subdirectories recursively
    bool follow_symlinks;           // Follow symbolic links
    uint32_t min_priority;          // Minimum event priority
    uint64_t rate_limit;            // Rate limit (events per second)
    
    // Time-based filtering
    uint64_t start_time;            // Start watching from this time
    uint64_t end_time;              // Stop watching at this time (0 = never)
    
    struct vfs_event_filter* next;  // Next filter in chain
} vfs_event_filter_t;

// Event watcher (subscriber)
typedef struct vfs_event_watcher {
    uint64_t id;                    // Unique watcher ID
    char name[64];                  // Watcher name (for debugging)
    
    // Callback function
    void (*callback)(struct vfs_event_watcher* watcher, vfs_event_t* event);
    void* user_data;                // User data for callback
    
    // Filtering
    vfs_event_filter_t* filters;   // Event filters
    uint32_t filter_count;          // Number of filters
    
    // Delivery configuration
    vfs_event_delivery_t delivery;  // Delivery mode
    uint32_t batch_size;            // Batch size for batched delivery
    uint32_t batch_timeout_ms;      // Batch timeout
    
    // Event queue (for async delivery)
    vfs_event_t* queue_head;        // Head of event queue
    vfs_event_t* queue_tail;        // Tail of event queue
    uint32_t queue_size;            // Current queue size
    uint32_t max_queue_size;        // Maximum queue size
    
    // Statistics
    uint64_t events_received;       // Total events received
    uint64_t events_filtered;       // Events filtered out
    uint64_t events_dropped;        // Events dropped due to queue overflow
    uint64_t avg_delivery_time;     // Average delivery time
    
    // Synchronization
    spinlock_t lock;                // Protect watcher state
    
    // List linkage
    struct vfs_event_watcher* next; // Next watcher in global list
    struct vfs_event_watcher* prev; // Previous watcher in global list
    
    // Performance optimization
    uint64_t last_event_time;       // Last event time (for rate limiting)
    uint32_t event_burst_count;     // Current burst count
    bool enabled;                   // Whether watcher is enabled
} vfs_event_watcher_t;

// Event system statistics
typedef struct vfs_event_stats {
    uint64_t total_events;          // Total events generated
    uint64_t events_delivered;      // Events successfully delivered
    uint64_t events_dropped;        // Events dropped
    uint64_t events_filtered;       // Events filtered out
    
    uint64_t watchers_active;       // Active watchers
    uint64_t watchers_total;        // Total watchers created
    
    uint64_t avg_generation_time;   // Average event generation time
    uint64_t avg_delivery_time;     // Average event delivery time
    uint64_t max_delivery_time;     // Maximum delivery time observed
    
    uint64_t queue_overflows;       // Queue overflow events
    uint64_t memory_usage;          // Memory used by event system
    
    // Per-event-type statistics
    uint64_t event_counts[32];      // Count per event type
} vfs_event_stats_t;

// Event system configuration
typedef struct vfs_event_config {
    uint32_t max_watchers;          // Maximum number of watchers
    uint32_t max_events_pending;    // Maximum pending events
    uint32_t default_queue_size;    // Default watcher queue size
    uint32_t max_queue_size;        // Maximum watcher queue size
    uint32_t batch_timeout_ms;      // Default batch timeout
    uint32_t gc_interval_ms;        // Garbage collection interval
    bool enable_security_events;    // Enable security event generation
    bool enable_performance_events; // Enable performance monitoring events
    uint32_t rate_limit_default;    // Default rate limit (events/sec)
} vfs_event_config_t;

// Global event system state
extern vfs_event_stats_t vfs_event_stats;
extern vfs_event_config_t vfs_event_config;
extern vfs_event_watcher_t* vfs_event_watchers;
extern spinlock_t vfs_event_watchers_lock;

// Core event system API

/**
 * Initialize the filesystem event system
 */
int vfs_events_init(const vfs_event_config_t* config);

/**
 * Shutdown the filesystem event system
 */
void vfs_events_shutdown(void);

/**
 * Generate a filesystem event
 */
int vfs_event_generate(uint32_t type, 
                      struct vfs_inode* inode,
                      struct vfs_dentry* dentry,
                      const char* path,
                      vfs_event_priority_t priority,
                      const void* event_data,
                      size_t data_size);

/**
 * Create an event watcher
 */
vfs_event_watcher_t* vfs_event_watcher_create(const char* name,
                                             void (*callback)(vfs_event_watcher_t*, vfs_event_t*),
                                             void* user_data);

/**
 * Destroy an event watcher
 */
int vfs_event_watcher_destroy(vfs_event_watcher_t* watcher);

/**
 * Add filter to event watcher
 */
int vfs_event_watcher_add_filter(vfs_event_watcher_t* watcher,
                                const vfs_event_filter_t* filter);

/**
 * Remove filter from event watcher
 */
int vfs_event_watcher_remove_filter(vfs_event_watcher_t* watcher,
                                   uint32_t filter_index);

/**
 * Enable/disable event watcher
 */
int vfs_event_watcher_set_enabled(vfs_event_watcher_t* watcher, bool enabled);

/**
 * Configure event watcher delivery mode
 */
int vfs_event_watcher_set_delivery(vfs_event_watcher_t* watcher,
                                  vfs_event_delivery_t mode,
                                  uint32_t batch_size,
                                  uint32_t timeout_ms);

// Event management

/**
 * Allocate event structure
 */
vfs_event_t* vfs_event_alloc(void);

/**
 * Free event structure
 */
void vfs_event_free(vfs_event_t* event);

/**
 * Reference event (increase ref count)
 */
vfs_event_t* vfs_event_ref(vfs_event_t* event);

/**
 * Unreference event (decrease ref count)
 */
void vfs_event_unref(vfs_event_t* event);

/**
 * Clone event
 */
vfs_event_t* vfs_event_clone(vfs_event_t* event);

// High-level event generation functions

/**
 * Generate file creation event
 */
void vfs_event_file_create(struct vfs_inode* inode, const char* path);

/**
 * Generate file deletion event
 */
void vfs_event_file_delete(struct vfs_inode* inode, const char* path);

/**
 * Generate file modification event
 */
void vfs_event_file_modify(struct vfs_inode* inode, const char* path, 
                          uint64_t old_size, uint64_t new_size);

/**
 * Generate file access event
 */
void vfs_event_file_access(struct vfs_inode* inode, const char* path);

/**
 * Generate file open event
 */
void vfs_event_file_open(struct vfs_file* file, uint32_t flags);

/**
 * Generate file close event
 */
void vfs_event_file_close(struct vfs_file* file);

/**
 * Generate file move event
 */
void vfs_event_file_move(struct vfs_inode* inode, 
                        const char* old_path, const char* new_path);

/**
 * Generate metadata change event
 */
void vfs_event_metadata_change(struct vfs_inode* inode, const char* path,
                              mode_t old_mode, mode_t new_mode,
                              uid_t old_uid, uid_t new_uid,
                              gid_t old_gid, gid_t new_gid);

/**
 * Generate security event
 */
void vfs_event_security(struct vfs_inode* inode, const char* path,
                       const char* label, uint32_t action);

/**
 * Generate error event
 */
void vfs_event_error(struct vfs_inode* inode, const char* path,
                    int error_code, const char* description);

// Event filtering and delivery

/**
 * Check if event matches filter
 */
bool vfs_event_matches_filter(vfs_event_t* event, vfs_event_filter_t* filter);

/**
 * Deliver event to watcher
 */
int vfs_event_deliver(vfs_event_watcher_t* watcher, vfs_event_t* event);

/**
 * Flush batched events for watcher
 */
int vfs_event_flush_batch(vfs_event_watcher_t* watcher);

/**
 * Process pending events
 */
void vfs_event_process_pending(void);

// Statistics and monitoring

/**
 * Get event system statistics
 */
int vfs_get_event_stats(vfs_event_stats_t* stats);

/**
 * Reset event statistics
 */
void vfs_reset_event_stats(void);

/**
 * Get watcher statistics
 */
int vfs_get_watcher_stats(vfs_event_watcher_t* watcher, 
                         uint64_t* events_received,
                         uint64_t* events_filtered,
                         uint64_t* events_dropped);

// Configuration management

/**
 * Update event system configuration
 */
int vfs_configure_events(const vfs_event_config_t* config);

/**
 * Get current event configuration
 */
int vfs_get_event_config(vfs_event_config_t* config);

// Utility functions

/**
 * Convert event type to string
 */
const char* vfs_event_type_string(uint32_t type);

/**
 * Convert priority to string
 */
const char* vfs_event_priority_string(vfs_event_priority_t priority);

/**
 * Check if path matches pattern (with wildcards)
 */
bool vfs_path_matches_pattern(const char* path, const char* pattern);

/**
 * Get current timestamp in nanoseconds
 */
uint64_t vfs_event_timestamp(void);

// Memory management

/**
 * Garbage collect unused events
 */
void vfs_event_gc(void);

/**
 * Get event system memory usage
 */
size_t vfs_event_memory_usage(void);

// Integration with audit system

/**
 * Send event to audit subsystem
 */
void vfs_event_audit(vfs_event_t* event);

/**
 * Enable/disable audit integration
 */
void vfs_event_set_audit_enabled(bool enabled);

// Performance optimization

/**
 * Enable fast path for high-frequency events
 */
void vfs_event_enable_fast_path(uint32_t event_types);

/**
 * Set event generation rate limit
 */
int vfs_event_set_rate_limit(uint32_t event_type, uint32_t events_per_sec);

// Error codes
#define VFS_EVENT_SUCCESS           0
#define VFS_EVENT_ERR_NO_MEMORY    -5001
#define VFS_EVENT_ERR_INVALID_ARG  -5002
#define VFS_EVENT_ERR_NOT_FOUND    -5003
#define VFS_EVENT_ERR_EXISTS       -5004
#define VFS_EVENT_ERR_PERMISSION   -5005
#define VFS_EVENT_ERR_OVERFLOW     -5006
#define VFS_EVENT_ERR_TIMEOUT      -5007
#define VFS_EVENT_ERR_RATE_LIMITED -5008

#ifdef __cplusplus
}
#endif

#endif // VFS_EVENTS_H