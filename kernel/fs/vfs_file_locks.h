/**
 * @file vfs_file_locks.h
 * @brief RaeenOS Advanced File Locking System
 * 
 * Enterprise-grade file locking with:
 * - Mandatory and advisory locking mechanisms
 * - POSIX-compliant byte-range locks
 * - Deadlock detection and prevention
 * - Lock inheritance and priority boosting
 * - Distributed locking support for network filesystems
 * - Performance optimizations for high-contention scenarios
 * 
 * Version: 2.0 - Production Ready
 * Security Level: Enterprise Grade
 * Compliance: POSIX.1-2008, NFSv4 locking
 */

#ifndef VFS_FILE_LOCKS_H
#define VFS_FILE_LOCKS_H

#include <stdint.h>
#include <stdbool.h>
#include "../include/types.h"
#include "../sync.h"
#include "../include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Lock types
typedef enum {
    VFS_LOCK_NONE = 0,
    VFS_LOCK_READ = 1,          // Shared/read lock
    VFS_LOCK_WRITE = 2,         // Exclusive/write lock
    VFS_LOCK_UPGRADE = 3,       // Upgrade from read to write
    VFS_LOCK_DOWNGRADE = 4      // Downgrade from write to read
} vfs_lock_type_t;

// Lock modes
typedef enum {
    VFS_LOCK_ADVISORY = 0,      // Advisory locking (default)
    VFS_LOCK_MANDATORY = 1      // Mandatory locking (enforced by kernel)
} vfs_lock_mode_t;

// Lock states
typedef enum {
    VFS_LOCK_STATE_PENDING = 0, // Lock request is pending
    VFS_LOCK_STATE_GRANTED = 1, // Lock has been granted
    VFS_LOCK_STATE_BLOCKED = 2, // Lock is blocked by another lock
    VFS_LOCK_STATE_CANCELED = 3 // Lock request was canceled
} vfs_lock_state_t;

// Lock flags
#define VFS_LOCK_FLAG_NONBLOCK    0x01  // Don't block on lock conflicts
#define VFS_LOCK_FLAG_LEASE       0x02  // File lease (break on access)
#define VFS_LOCK_FLAG_FLOCK       0x04  // BSD-style whole file lock
#define VFS_LOCK_FLAG_POSIX       0x08  // POSIX byte-range lock
#define VFS_LOCK_FLAG_REMOTE      0x10  // Remote/network lock
#define VFS_LOCK_FLAG_INHERITED   0x20  // Lock inherited from parent process
#define VFS_LOCK_FLAG_PRIORITY    0x40  // High-priority lock (anti-starvation)

// Forward declarations
struct vfs_file;
struct vfs_inode;
struct vfs_lock_manager;

// Lock request structure
typedef struct vfs_lock_request {
    uint64_t id;                    // Unique lock ID
    pid_t owner_pid;                // Process ID of lock owner
    uint64_t owner_tid;             // Thread ID of lock owner
    
    vfs_lock_type_t type;           // Lock type (read/write)
    vfs_lock_mode_t mode;           // Advisory or mandatory
    vfs_lock_state_t state;         // Current lock state
    uint32_t flags;                 // Lock flags
    
    // Byte range
    uint64_t start;                 // Start offset
    uint64_t end;                   // End offset (0 = EOF)
    uint64_t length;                // Lock length
    
    // Timing
    uint64_t request_time;          // When lock was requested
    uint64_t grant_time;            // When lock was granted
    uint64_t timeout_ms;            // Lock timeout (0 = no timeout)
    
    // Wait queue and priority
    int priority;                   // Lock priority (higher = more important)
    struct vfs_lock_request* next_waiter;  // Next waiter in queue
    struct vfs_lock_request* prev_waiter;  // Previous waiter in queue
    
    // Callback for asynchronous locks
    void (*callback)(struct vfs_lock_request* req, int status);
    void* callback_data;
    
    // Lock manager reference
    struct vfs_lock_manager* manager;
    
    // Conflict detection
    struct vfs_lock_request* conflicts[16]; // Conflicting locks
    int conflict_count;
    
    // Statistics
    uint64_t wait_time;             // Time spent waiting
    uint32_t retry_count;           // Number of retries
    
    spinlock_t lock;                // Protect this structure
} vfs_lock_request_t;

// Lock manager for each inode
typedef struct vfs_lock_manager {
    struct vfs_inode* inode;        // Associated inode
    
    // Active locks
    vfs_lock_request_t* read_locks; // List of read locks
    vfs_lock_request_t* write_locks; // List of write locks
    
    // Waiting locks
    vfs_lock_request_t* wait_queue_head;
    vfs_lock_request_t* wait_queue_tail;
    
    // Lock statistics
    uint32_t active_read_locks;
    uint32_t active_write_locks;
    uint32_t waiting_locks;
    uint64_t total_locks_granted;
    uint64_t total_locks_denied;
    uint64_t avg_wait_time;
    
    // Configuration
    bool mandatory_locking;         // Enable mandatory locking
    uint32_t max_locks_per_file;    // Maximum locks per file
    uint32_t max_wait_time_ms;      // Maximum wait time
    
    // Synchronization
    rwlock_t manager_lock;          // Reader-writer lock for manager
    spinlock_t wait_queue_lock;     // Protect wait queue
    
    // Deadlock detection
    uint64_t last_deadlock_check;   // Last deadlock detection run
    uint32_t deadlock_check_interval; // Check interval in ms
    
    // Performance optimization
    uint64_t lock_bitmap[8];        // Fast conflict detection for small ranges
    bool use_bitmap;                // Whether to use bitmap optimization
} vfs_lock_manager_t;

// Global lock system statistics
typedef struct vfs_lock_stats {
    uint64_t total_lock_requests;   // Total lock requests
    uint64_t locks_granted;         // Locks successfully granted
    uint64_t locks_denied;          // Locks denied due to conflicts
    uint64_t locks_timeout;         // Locks that timed out
    uint64_t locks_canceled;        // Locks canceled by user
    
    uint64_t read_locks_active;     // Currently active read locks
    uint64_t write_locks_active;    // Currently active write locks
    uint64_t locks_waiting;         // Locks currently waiting
    
    uint64_t deadlocks_detected;    // Deadlocks detected
    uint64_t deadlocks_resolved;    // Deadlocks resolved
    
    uint64_t avg_lock_hold_time;    // Average lock hold time (ms)
    uint64_t avg_wait_time;         // Average wait time (ms)
    uint64_t max_wait_time;         // Maximum wait time observed
    
    uint32_t lock_managers_active;  // Active lock managers
    uint64_t memory_usage;          // Memory used by lock system
} vfs_lock_stats_t;

// Lock system configuration
typedef struct vfs_lock_config {
    uint32_t max_locks_per_process; // Max locks per process
    uint32_t max_locks_global;      // Global lock limit
    uint32_t default_timeout_ms;    // Default lock timeout
    uint32_t deadlock_timeout_ms;   // Deadlock detection timeout
    uint32_t deadlock_check_interval; // Deadlock check interval
    bool enable_mandatory_locking;  // Global mandatory locking
    bool enable_deadlock_detection; // Enable deadlock detection
    bool enable_lock_inheritance;   // Enable lock inheritance
    uint32_t priority_boost_time;   // Time before priority boost (ms)
} vfs_lock_config_t;

// Global lock system state
extern vfs_lock_stats_t vfs_lock_stats;
extern vfs_lock_config_t vfs_lock_config;
extern spinlock_t vfs_global_lock_list_lock;

// Core lock API

/**
 * Initialize the file locking system
 */
int vfs_locks_init(const vfs_lock_config_t* config);

/**
 * Shutdown the file locking system
 */
void vfs_locks_shutdown(void);

/**
 * Initialize lock manager for an inode
 */
vfs_lock_manager_t* vfs_lock_manager_create(struct vfs_inode* inode);

/**
 * Destroy lock manager
 */
void vfs_lock_manager_destroy(vfs_lock_manager_t* manager);

/**
 * Request a file lock
 */
vfs_lock_request_t* vfs_lock_request(struct vfs_file* file, 
                                    vfs_lock_type_t type,
                                    uint64_t start, 
                                    uint64_t length,
                                    uint32_t flags);

/**
 * Release a file lock
 */
int vfs_lock_release(vfs_lock_request_t* lock);

/**
 * Test if a lock can be acquired (non-blocking)
 */
int vfs_lock_test(struct vfs_file* file, 
                  vfs_lock_type_t type,
                  uint64_t start, 
                  uint64_t length,
                  vfs_lock_request_t** conflicting_lock);

/**
 * Wait for a lock to be granted
 */
int vfs_lock_wait(vfs_lock_request_t* lock, uint32_t timeout_ms);

/**
 * Cancel a pending lock request
 */
int vfs_lock_cancel(vfs_lock_request_t* lock);

/**
 * Upgrade a read lock to write lock
 */
int vfs_lock_upgrade(vfs_lock_request_t* lock);

/**
 * Downgrade a write lock to read lock
 */
int vfs_lock_downgrade(vfs_lock_request_t* lock);

// POSIX-style locking functions

/**
 * POSIX fcntl-style locking
 */
int vfs_posix_lock(struct vfs_file* file, int cmd, struct flock* fl);

/**
 * BSD flock-style whole-file locking
 */
int vfs_flock(struct vfs_file* file, int operation);

// Advanced features

/**
 * Set mandatory locking mode for a file
 */
int vfs_set_mandatory_locking(struct vfs_inode* inode, bool enabled);

/**
 * Check if mandatory locking is enabled
 */
bool vfs_is_mandatory_locking(struct vfs_inode* inode);

/**
 * Enforce mandatory locks on I/O operations
 */
int vfs_check_mandatory_locks(struct vfs_file* file, uint64_t start, uint64_t length, bool is_write);

/**
 * Break file leases
 */
int vfs_break_lease(struct vfs_inode* inode, unsigned int mode);

/**
 * Grant a file lease
 */
int vfs_grant_lease(struct vfs_file* file, int type);

// Deadlock detection and resolution

/**
 * Run deadlock detection
 */
int vfs_detect_deadlocks(void);

/**
 * Resolve detected deadlocks
 */
int vfs_resolve_deadlocks(void);

/**
 * Check for circular wait in lock dependency graph
 */
bool vfs_check_circular_wait(vfs_lock_request_t* lock);

// Lock inheritance and priority

/**
 * Inherit locks from parent process
 */
int vfs_inherit_locks(pid_t parent_pid, pid_t child_pid);

/**
 * Boost lock priority to prevent starvation
 */
int vfs_boost_lock_priority(vfs_lock_request_t* lock);

/**
 * Clean up locks for a process
 */
void vfs_cleanup_process_locks(pid_t pid);

// Statistics and monitoring

/**
 * Get lock system statistics
 */
int vfs_get_lock_stats(vfs_lock_stats_t* stats);

/**
 * Reset lock statistics
 */
void vfs_reset_lock_stats(void);

/**
 * Get locks for a specific file
 */
int vfs_get_file_locks(struct vfs_inode* inode, vfs_lock_request_t** locks, size_t* count);

/**
 * Get locks for a specific process
 */
int vfs_get_process_locks(pid_t pid, vfs_lock_request_t** locks, size_t* count);

// Configuration management

/**
 * Update lock system configuration
 */
int vfs_configure_locks(const vfs_lock_config_t* config);

/**
 * Get current lock configuration
 */
int vfs_get_lock_config(vfs_lock_config_t* config);

// Utility functions

/**
 * Check if two lock ranges overlap
 */
bool vfs_locks_overlap(uint64_t start1, uint64_t end1, uint64_t start2, uint64_t end2);

/**
 * Check if two locks conflict
 */
bool vfs_locks_conflict(vfs_lock_request_t* lock1, vfs_lock_request_t* lock2);

/**
 * Convert lock type to string (for debugging)
 */
const char* vfs_lock_type_string(vfs_lock_type_t type);

/**
 * Convert lock state to string (for debugging)
 */
const char* vfs_lock_state_string(vfs_lock_state_t state);

// Network filesystem support

/**
 * Register network lock operations
 */
int vfs_register_network_locks(const char* fstype, 
                              int (*acquire)(vfs_lock_request_t*),
                              int (*release)(vfs_lock_request_t*),
                              int (*test)(vfs_lock_request_t*));

/**
 * Handle remote lock conflicts
 */
int vfs_handle_remote_conflict(vfs_lock_request_t* local_lock, 
                              vfs_lock_request_t* remote_lock);

// Performance optimization

/**
 * Enable fast path for non-conflicting locks
 */
void vfs_enable_lock_fast_path(struct vfs_inode* inode);

/**
 * Optimize lock manager for high contention
 */
int vfs_optimize_lock_manager(vfs_lock_manager_t* manager);

// Error codes
#define VFS_LOCK_SUCCESS           0
#define VFS_LOCK_ERR_CONFLICT     -4001
#define VFS_LOCK_ERR_TIMEOUT      -4002
#define VFS_LOCK_ERR_DEADLOCK     -4003
#define VFS_LOCK_ERR_NO_MEMORY    -4004
#define VFS_LOCK_ERR_INVALID_ARG  -4005
#define VFS_LOCK_ERR_PERMISSION   -4006
#define VFS_LOCK_ERR_NOT_FOUND    -4007
#define VFS_LOCK_ERR_CANCELED     -4008
#define VFS_LOCK_ERR_WOULD_BLOCK  -4009
#define VFS_LOCK_ERR_TOO_MANY     -4010

// POSIX lock commands (for fcntl)
#define F_GETLK     5   // Get lock information
#define F_SETLK     6   // Set lock (non-blocking)
#define F_SETLKW    7   // Set lock (blocking)

// POSIX lock types
#define F_RDLCK     0   // Read lock
#define F_WRLCK     1   // Write lock
#define F_UNLCK     2   // Unlock

// flock operations
#define LOCK_SH     1   // Shared lock
#define LOCK_EX     2   // Exclusive lock
#define LOCK_NB     4   // Non-blocking
#define LOCK_UN     8   // Unlock

// POSIX flock structure
struct flock {
    short l_type;       // Type of lock: F_RDLCK, F_WRLCK, F_UNLCK
    short l_whence;     // How to interpret l_start: SEEK_SET, SEEK_CUR, SEEK_END
    off_t l_start;      // Starting offset for lock
    off_t l_len;        // Number of bytes to lock
    pid_t l_pid;        // PID of process blocking our lock (F_GETLK only)
};

#ifdef __cplusplus
}
#endif

#endif // VFS_FILE_LOCKS_H