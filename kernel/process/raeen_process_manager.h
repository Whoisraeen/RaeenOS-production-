#pragma once

/**
 * RaeenOS Advanced Process Management System
 * 
 * A comprehensive process manager that provides:
 * - Multi-threaded process execution
 * - Advanced scheduling algorithms (CFS, RT, Gaming-optimized)
 * - Inter-process communication (IPC)
 * - Memory management and protection
 * - Process monitoring and debugging
 * - Container/sandbox support
 * - Real-time capabilities for gaming and multimedia
 */

#include "process.h"
#include "../include/types.h"
#include "../include/sync.h"
#include "../memory.h"
#include "../fs/vfs_production.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ADVANCED PROCESS TYPES
// ============================================================================

typedef struct RaeenProcess RaeenProcess;
typedef struct RaeenThread RaeenThread;
typedef struct RaeenProcessGroup RaeenProcessGroup;
typedef struct RaeenScheduler RaeenScheduler;

// Process types for optimization
typedef enum {
    RAEEN_PROCESS_UNKNOWN = 0,
    RAEEN_PROCESS_SYSTEM,        // System/kernel processes
    RAEEN_PROCESS_SERVICE,       // Background services
    RAEEN_PROCESS_APPLICATION,   // User applications
    RAEEN_PROCESS_GAME,          // Games (high priority, low latency)
    RAEEN_PROCESS_REALTIME,      // Real-time processes
    RAEEN_PROCESS_BATCH,         // Batch/background processing
    RAEEN_PROCESS_COMPATIBILITY  // Wine/compatibility layer processes
} RaeenProcessType;

// Scheduling policies
typedef enum {
    RAEEN_SCHED_NORMAL = 0,      // Standard CFS scheduling
    RAEEN_SCHED_FIFO,            // First-in-first-out real-time
    RAEEN_SCHED_RR,              // Round-robin real-time
    RAEEN_SCHED_BATCH,           // Batch processing (low priority)
    RAEEN_SCHED_IDLE,            // Idle processes
    RAEEN_SCHED_GAMING,          // Gaming-optimized scheduling
    RAEEN_SCHED_INTERACTIVE      // Interactive applications
} RaeenSchedulingPolicy;

// Process states (extended from basic process.h)
typedef enum {
    RAEEN_STATE_NEW = 0,         // Process being created
    RAEEN_STATE_READY,           // Ready to run
    RAEEN_STATE_RUNNING,         // Currently executing
    RAEEN_STATE_BLOCKED,         // Blocked on I/O or resource
    RAEEN_STATE_SUSPENDED,       // Suspended by user/system
    RAEEN_STATE_ZOMBIE,          // Terminated, waiting for parent
    RAEEN_STATE_STOPPED,         // Stopped by signal
    RAEEN_STATE_SLEEPING,        // Sleeping/waiting
    RAEEN_STATE_TERMINATED       // Completely terminated
} RaeenProcessState;

// Thread states
typedef enum {
    RAEEN_THREAD_NEW = 0,
    RAEEN_THREAD_RUNNABLE,
    RAEEN_THREAD_RUNNING,
    RAEEN_THREAD_BLOCKED,
    RAEEN_THREAD_WAITING,
    RAEEN_THREAD_TIMED_WAITING,
    RAEEN_THREAD_TERMINATED
} RaeenThreadState;

// Process capabilities and limits
typedef struct {
    // Resource limits (rlimit-style)
    uint64_t max_memory;         // Maximum memory usage (bytes)
    uint64_t max_cpu_time;       // Maximum CPU time (microseconds)
    uint32_t max_threads;        // Maximum number of threads
    uint32_t max_files;          // Maximum open file descriptors
    uint32_t max_processes;      // Maximum child processes
    uint64_t max_core_size;      // Maximum core dump size
    uint64_t max_stack_size;     // Maximum stack size per thread
    uint32_t max_priority;       // Maximum allowed priority
    
    // Capabilities (capability-based security)
    uint64_t capabilities;       // Process capabilities bitmask
    bool can_create_processes;   // Can fork/exec new processes
    bool can_access_network;     // Can use network interfaces
    bool can_access_hardware;    // Can access hardware devices
    bool can_modify_system;      // Can modify system settings
    bool can_debug_others;       // Can debug other processes
    bool can_change_priority;    // Can change process priority
    bool can_use_realtime;       // Can use real-time scheduling
} RaeenProcessLimits;

// Performance and resource monitoring
typedef struct {
    // CPU usage
    uint64_t cpu_time_user;      // User-mode CPU time (microseconds)
    uint64_t cpu_time_kernel;    // Kernel-mode CPU time (microseconds)
    uint64_t cpu_time_total;     // Total CPU time
    float cpu_usage_percent;     // Current CPU usage percentage
    
    // Memory usage
    uint64_t memory_rss;         // Resident set size (physical memory)
    uint64_t memory_vss;         // Virtual set size (virtual memory)
    uint64_t memory_shared;      // Shared memory
    uint64_t memory_text;        // Text (code) segment size
    uint64_t memory_data;        // Data segment size
    uint64_t memory_stack;       // Stack size
    uint64_t memory_heap;        // Heap size
    
    // I/O statistics
    uint64_t io_reads;           // Number of read operations
    uint64_t io_writes;          // Number of write operations
    uint64_t io_bytes_read;      // Bytes read
    uint64_t io_bytes_written;   // Bytes written
    
    // Context switches
    uint64_t context_switches_voluntary;   // Voluntary context switches
    uint64_t context_switches_involuntary; // Involuntary context switches
    
    // System calls
    uint64_t syscall_count;      // Total system calls made
    
    // Timing
    uint64_t start_time;         // Process start time
    uint64_t last_scheduled;     // Last time scheduled
    uint64_t total_runtime;      // Total runtime
} RaeenProcessStats;

// Extended thread structure
struct RaeenThread {
    uint32_t tid;                // Thread ID
    char name[64];               // Thread name
    RaeenThreadState state;      // Thread state
    int priority;                // Thread priority (-20 to +19)
    uint32_t nice_value;         // Nice value for scheduling
    
    // CPU state and registers
    cpu_state_t cpu_state;       // Saved CPU state
    void* stack_base;            // Thread stack base
    size_t stack_size;           // Thread stack size
    void* kernel_stack;          // Kernel stack for system calls
    
    // Scheduling information
    uint64_t runtime;            // Total runtime
    uint64_t last_scheduled;     // Last scheduled time
    uint64_t time_slice;         // Current time slice
    uint32_t cpu_affinity;       // CPU affinity mask
    
    // Synchronization
    void* waiting_on;            // What the thread is waiting on
    uint64_t wakeup_time;        // Time to wake up (for sleeping threads)
    
    // Parent process
    RaeenProcess* process;       // Parent process
    
    // Thread list linkage
    RaeenThread* next;           // Next thread in process
    RaeenThread* sched_next;     // Next thread in scheduler queue
    
    // Thread-local storage
    void* tls_data;              // Thread-local storage
    
    // Signal handling
    uint32_t signal_mask;        // Signal mask
    void* signal_handlers[32];   // Signal handlers
    
    // Performance monitoring
    RaeenProcessStats stats;     // Per-thread statistics
};

// Extended process structure
struct RaeenProcess {
    // Basic process information
    pid_t pid;                   // Process ID
    pid_t ppid;                  // Parent process ID
    pid_t pgid;                  // Process group ID
    pid_t sid;                   // Session ID
    char name[256];              // Process name/command
    char* cmdline;               // Full command line
    char** argv;                 // Argument vector
    char** envp;                 // Environment variables
    
    // Process state and type
    RaeenProcessState state;     // Current state
    RaeenProcessType type;       // Process type for optimization
    RaeenSchedulingPolicy sched_policy; // Scheduling policy
    int exit_code;               // Exit code when terminated
    uint32_t flags;              // Process flags
    
    // Security and permissions
    uid_t uid;                   // Real user ID
    uid_t euid;                  // Effective user ID
    gid_t gid;                   // Real group ID
    gid_t egid;                  // Effective group ID
    uint32_t capabilities;       // Process capabilities
    char* security_context;      // Security context (SELinux-style)
    
    // Memory management
    page_directory_t* page_directory; // Virtual memory page directory
    uint64_t memory_base;        // Base address of process memory
    uint64_t memory_size;        // Total allocated memory
    uint64_t heap_start;         // Heap start address
    uint64_t heap_end;           // Heap end address
    uint64_t stack_start;        // Stack start address
    uint64_t stack_end;          // Stack end address
    
    // Threading
    RaeenThread* main_thread;    // Main thread
    RaeenThread* threads;        // List of all threads
    uint32_t thread_count;       // Number of threads
    spinlock_t thread_lock;      // Lock for thread list
    
    // File descriptors and I/O
    vfs_file_t** fd_table;       // File descriptor table
    uint32_t max_fds;            // Maximum file descriptors
    spinlock_t fd_lock;          // Lock for FD table
    char* working_directory;     // Current working directory
    mode_t umask;                // File creation mask
    
    // Inter-process communication
    void* shared_memory;         // Shared memory segments
    void* message_queues;        // Message queues
    void* semaphores;            // Semaphores
    void* mutexes;               // Mutexes
    
    // Resource limits and monitoring
    RaeenProcessLimits limits;   // Resource limits
    RaeenProcessStats stats;     // Performance statistics
    uint64_t last_update_time;   // Last stats update time
    
    // Process relationships
    RaeenProcess* parent;        // Parent process
    RaeenProcess* children;      // First child process
    RaeenProcess* siblings;      // Next sibling process
    RaeenProcessGroup* group;    // Process group
    
    // Scheduling information
    int priority;                // Process priority
    int nice_value;              // Nice value
    uint32_t cpu_affinity;       // CPU affinity mask
    uint64_t runtime;            // Total runtime
    uint64_t last_scheduled;     // Last scheduled time
    
    // Gaming and real-time features
    bool is_game;                // Optimized for gaming
    bool is_realtime;            // Real-time process
    uint32_t rt_priority;        // Real-time priority
    uint64_t deadline;           // Deadline for real-time scheduling
    uint32_t gaming_profile;     // Gaming optimization profile
    
    // Container/sandbox support
    bool is_containerized;       // Running in container
    char* container_id;          // Container identifier
    void* namespace_info;        // Namespace information
    
    // Debugging and profiling
    bool is_being_debugged;      // Process is being debugged
    pid_t debugger_pid;          // PID of debugger process
    void* debug_info;            // Debug information
    bool profiling_enabled;      // Performance profiling enabled
    
    // Synchronization
    spinlock_t lock;             // Process structure lock
    
    // List linkage
    RaeenProcess* next;          // Next process in global list
    RaeenProcess* sched_next;    // Next process in scheduler queue
};

// Process group structure
struct RaeenProcessGroup {
    pid_t pgid;                  // Process group ID
    RaeenProcess* leader;        // Group leader process
    RaeenProcess* processes;     // Processes in group
    uint32_t process_count;      // Number of processes in group
    spinlock_t lock;             // Group lock
    struct RaeenProcessGroup* next; // Next group in list
};

// ============================================================================
// PROCESS MANAGEMENT API
// ============================================================================

// Process lifecycle
RaeenProcess* raeen_process_create(const char* executable, char* const argv[], char* const envp[]);
RaeenProcess* raeen_process_fork(RaeenProcess* parent);
int raeen_process_exec(RaeenProcess* process, const char* executable, char* const argv[], char* const envp[]);
int raeen_process_exit(RaeenProcess* process, int exit_code);
int raeen_process_kill(pid_t pid, int signal);
int raeen_process_wait(RaeenProcess* parent, pid_t pid, int* status);
int raeen_process_suspend(pid_t pid);
int raeen_process_resume(pid_t pid);

// Process queries and control
RaeenProcess* raeen_process_find(pid_t pid);
RaeenProcess* raeen_process_find_by_name(const char* name);
RaeenProcess** raeen_process_list(uint32_t* count);
RaeenProcess* raeen_process_current(void);
int raeen_process_set_priority(pid_t pid, int priority);
int raeen_process_set_affinity(pid_t pid, uint32_t cpu_mask);
int raeen_process_set_limits(pid_t pid, const RaeenProcessLimits* limits);

// Threading
RaeenThread* raeen_thread_create(RaeenProcess* process, void (*entry_point)(void* arg), void* arg);
int raeen_thread_join(RaeenThread* thread, void** return_value);
int raeen_thread_detach(RaeenThread* thread);
RaeenThread* raeen_thread_current(void);
int raeen_thread_yield(void);
int raeen_thread_sleep(uint64_t milliseconds);
int raeen_thread_set_priority(RaeenThread* thread, int priority);
int raeen_thread_set_affinity(RaeenThread* thread, uint32_t cpu_mask);

// Process groups and sessions
RaeenProcessGroup* raeen_process_group_create(pid_t pgid);
int raeen_process_group_add(RaeenProcessGroup* group, RaeenProcess* process);
int raeen_process_group_remove(RaeenProcessGroup* group, RaeenProcess* process);
int raeen_process_set_group(pid_t pid, pid_t pgid);
int raeen_process_create_session(pid_t pid);

// ============================================================================
// SCHEDULING SYSTEM
// ============================================================================

// Scheduler structure
struct RaeenScheduler {
    // Scheduling queues
    RaeenProcess* ready_queues[8];    // Priority-based ready queues
    RaeenProcess* realtime_queue;     // Real-time processes
    RaeenProcess* gaming_queue;       // Gaming processes
    RaeenProcess* interactive_queue;  // Interactive processes
    RaeenProcess* batch_queue;        // Batch processes
    RaeenProcess* idle_queue;         // Idle processes
    
    // Current running processes per CPU
    RaeenProcess** current_processes; // Array of current processes (per CPU)
    RaeenThread** current_threads;    // Array of current threads (per CPU)
    
    // Scheduling statistics
    uint64_t total_context_switches;
    uint64_t total_preemptions;
    uint64_t load_average[3];         // 1, 5, 15 minute load averages
    
    // Configuration
    uint32_t time_slice_ms;           // Default time slice in milliseconds
    uint32_t gaming_boost;            // Priority boost for gaming processes
    bool preemption_enabled;          // Preemptive scheduling enabled
    bool load_balancing_enabled;      // CPU load balancing enabled
    
    // CPU information
    uint32_t cpu_count;               // Number of CPUs
    uint32_t* cpu_usage;              // Per-CPU usage percentages
    
    // Synchronization
    spinlock_t lock;                  // Scheduler lock
};

// Scheduler API
int raeen_scheduler_init(void);
void raeen_scheduler_tick(void);
void raeen_scheduler_yield(void);
RaeenProcess* raeen_scheduler_next(uint32_t cpu_id);
int raeen_scheduler_add_process(RaeenProcess* process);
int raeen_scheduler_remove_process(RaeenProcess* process);
int raeen_scheduler_preempt(uint32_t cpu_id);
void raeen_scheduler_balance_load(void);

// Gaming-optimized scheduling
int raeen_scheduler_enable_gaming_mode(bool enable);
int raeen_scheduler_set_gaming_process(pid_t pid, bool is_game);
int raeen_scheduler_set_gaming_profile(pid_t pid, uint32_t profile);

// Real-time scheduling
int raeen_scheduler_set_realtime(pid_t pid, uint32_t priority, uint64_t deadline);
int raeen_scheduler_remove_realtime(pid_t pid);

// ============================================================================
// INTER-PROCESS COMMUNICATION (IPC)
// ============================================================================

// IPC types
typedef enum {
    RAEEN_IPC_PIPE,
    RAEEN_IPC_NAMED_PIPE,
    RAEEN_IPC_MESSAGE_QUEUE,
    RAEEN_IPC_SHARED_MEMORY,
    RAEEN_IPC_SEMAPHORE,
    RAEEN_IPC_MUTEX,
    RAEEN_IPC_CONDITION,
    RAEEN_IPC_SOCKET
} RaeenIPCType;

// IPC handle
typedef struct {
    RaeenIPCType type;
    uint32_t id;
    void* handle;
    uint32_t permissions;
    pid_t owner_pid;
} RaeenIPCHandle;

// IPC API
RaeenIPCHandle* raeen_ipc_create(RaeenIPCType type, const char* name, uint32_t flags);
int raeen_ipc_connect(const char* name, uint32_t flags);
int raeen_ipc_send(RaeenIPCHandle* handle, const void* data, size_t size);
int raeen_ipc_receive(RaeenIPCHandle* handle, void* buffer, size_t size);
int raeen_ipc_close(RaeenIPCHandle* handle);
int raeen_ipc_destroy(RaeenIPCHandle* handle);

// Shared memory
void* raeen_shm_create(const char* name, size_t size, uint32_t permissions);
void* raeen_shm_attach(const char* name, uint32_t flags);
int raeen_shm_detach(void* address);
int raeen_shm_destroy(const char* name);

// Message queues
int raeen_mqueue_create(const char* name, uint32_t max_messages, uint32_t message_size);
int raeen_mqueue_send(const char* name, const void* message, size_t size, uint32_t priority);
int raeen_mqueue_receive(const char* name, void* buffer, size_t size, uint32_t* priority);
int raeen_mqueue_destroy(const char* name);

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

// Performance monitoring API
int raeen_process_get_stats(pid_t pid, RaeenProcessStats* stats);
int raeen_process_reset_stats(pid_t pid);
int raeen_process_enable_profiling(pid_t pid, bool enable);
int raeen_process_get_performance_counters(pid_t pid, uint64_t* counters, uint32_t count);

// System-wide monitoring
typedef struct {
    uint32_t total_processes;
    uint32_t running_processes;
    uint32_t sleeping_processes;
    uint32_t zombie_processes;
    uint64_t total_memory_used;
    uint64_t total_cpu_time;
    float average_load;
    uint64_t context_switches;
    uint64_t interrupts;
    uint32_t active_threads;
} RaeenSystemStats;

int raeen_system_get_stats(RaeenSystemStats* stats);
float raeen_system_get_load_average(int period);  // 0=1min, 1=5min, 2=15min
uint32_t raeen_system_get_process_count(void);
uint64_t raeen_system_get_uptime(void);

// ============================================================================
// DEBUGGING AND PROFILING
// ============================================================================

// Debug API
int raeen_process_attach_debugger(pid_t target_pid, pid_t debugger_pid);
int raeen_process_detach_debugger(pid_t target_pid);
int raeen_process_set_breakpoint(pid_t pid, void* address);
int raeen_process_clear_breakpoint(pid_t pid, void* address);
int raeen_process_single_step(pid_t pid);
int raeen_process_read_memory(pid_t pid, void* address, void* buffer, size_t size);
int raeen_process_write_memory(pid_t pid, void* address, const void* data, size_t size);

// Core dump generation
int raeen_process_generate_core_dump(pid_t pid, const char* filename);
int raeen_process_enable_core_dumps(pid_t pid, bool enable);

// ============================================================================
// SECURITY AND SANDBOXING
// ============================================================================

// Security API
int raeen_process_set_capabilities(pid_t pid, uint64_t capabilities);
int raeen_process_drop_capabilities(pid_t pid, uint64_t capabilities);
int raeen_process_check_capability(pid_t pid, uint32_t capability);
int raeen_process_set_security_context(pid_t pid, const char* context);

// Sandboxing
typedef struct {
    bool filesystem_isolation;
    bool network_isolation;
    bool device_isolation;
    char** allowed_paths;
    uint32_t allowed_path_count;
    char** blocked_paths;
    uint32_t blocked_path_count;
    uint64_t memory_limit;
    uint64_t cpu_limit;
} RaeenSandboxConfig;

int raeen_process_create_sandbox(const RaeenSandboxConfig* config);
int raeen_process_enter_sandbox(pid_t pid, const char* sandbox_id);
int raeen_process_exit_sandbox(pid_t pid);
int raeen_process_destroy_sandbox(const char* sandbox_id);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Process utilities
char* raeen_process_get_name(pid_t pid);
char* raeen_process_get_cmdline(pid_t pid);
char** raeen_process_get_environment(pid_t pid);
char* raeen_process_get_working_directory(pid_t pid);
int raeen_process_set_working_directory(pid_t pid, const char* path);

// Time utilities
uint64_t raeen_get_current_time_us(void);  // Microseconds since boot
uint64_t raeen_get_process_runtime(pid_t pid);
uint64_t raeen_get_thread_runtime(RaeenThread* thread);

// Memory utilities
uint64_t raeen_process_get_memory_usage(pid_t pid);
int raeen_process_set_memory_limit(pid_t pid, uint64_t limit);
int raeen_process_get_memory_info(pid_t pid, uint64_t* rss, uint64_t* vss, uint64_t* shared);

// CPU utilities
float raeen_process_get_cpu_usage(pid_t pid);
int raeen_process_get_cpu_affinity(pid_t pid);
uint32_t raeen_get_cpu_count(void);
float raeen_get_cpu_usage(uint32_t cpu_id);

#ifdef __cplusplus
}
#endif