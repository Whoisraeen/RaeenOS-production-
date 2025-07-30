#ifndef PROCESS_INTERFACE_H
#define PROCESS_INTERFACE_H

/**
 * @file process_interface.h
 * @brief Comprehensive Process and Thread Management Interface for RaeenOS
 * 
 * This interface defines the unified process and thread management API including
 * process creation, scheduling, IPC mechanisms, signal handling, and advanced
 * features like process containers and real-time scheduling.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Process management API version
#define PROCESS_API_VERSION 1

// Process and thread limits
#define MAX_PROCESSES       65536
#define MAX_THREADS_PER_PROCESS 8192
#define MAX_OPEN_FILES      1024
#define MAX_SIGNALS         64
#define MAX_GROUPS          64
#define MAX_NAMESPACES      16
#define PROCESS_NAME_MAX    256
#define COMMAND_LINE_MAX    4096

// Process states
typedef enum {
    PROCESS_STATE_CREATED,      // Just created, not yet running
    PROCESS_STATE_READY,        // Ready to run
    PROCESS_STATE_RUNNING,      // Currently executing
    PROCESS_STATE_BLOCKED,      // Blocked waiting for resource
    PROCESS_STATE_SUSPENDED,    // Suspended by signal
    PROCESS_STATE_ZOMBIE,       // Terminated, waiting for parent
    PROCESS_STATE_TERMINATED    // Completely terminated
} process_state_t;

// Thread states
typedef enum {
    THREAD_STATE_NEW,           // Thread created but not started
    THREAD_STATE_RUNNABLE,      // Ready to run
    THREAD_STATE_RUNNING,       // Currently executing
    THREAD_STATE_BLOCKED,       // Blocked on synchronization object
    THREAD_STATE_WAITING,       // Waiting for condition
    THREAD_STATE_SLEEPING,      // Sleeping for specified time
    THREAD_STATE_TERMINATED     // Thread has terminated
} thread_state_t;

// Process priority classes
typedef enum {
    PRIORITY_CLASS_IDLE,        // Lowest priority (background)
    PRIORITY_CLASS_NORMAL,      // Normal priority
    PRIORITY_CLASS_HIGH,        // High priority  
    PRIORITY_CLASS_REALTIME     // Real-time priority
} priority_class_t;

// Scheduling policies
typedef enum {
    SCHED_POLICY_NORMAL,        // Normal time-sharing
    SCHED_POLICY_BATCH,         // Batch processing
    SCHED_POLICY_IDLE,          // Idle tasks
    SCHED_POLICY_FIFO,          // Real-time FIFO
    SCHED_POLICY_RR,            // Real-time Round Robin
    SCHED_POLICY_DEADLINE       // Deadline scheduling
} sched_policy_t;

// Signal types
typedef enum {
    SIGNAL_HANGUP = 1,          // SIGHUP
    SIGNAL_INTERRUPT,           // SIGINT
    SIGNAL_QUIT,                // SIGQUIT  
    SIGNAL_ILLEGAL,             // SIGILL
    SIGNAL_TRAP,                // SIGTRAP
    SIGNAL_ABORT,               // SIGABRT
    SIGNAL_BUS_ERROR,           // SIGBUS
    SIGNAL_FPE,                 // SIGFPE
    SIGNAL_KILL,                // SIGKILL (uncatchable)
    SIGNAL_USER1,               // SIGUSR1
    SIGNAL_SEGV,                // SIGSEGV
    SIGNAL_USER2,               // SIGUSR2
    SIGNAL_PIPE,                // SIGPIPE
    SIGNAL_ALARM,               // SIGALRM
    SIGNAL_TERMINATE,           // SIGTERM
    SIGNAL_CHILD,               // SIGCHLD
    SIGNAL_CONTINUE,            // SIGCONT
    SIGNAL_STOP,                // SIGSTOP (uncatchable)
    SIGNAL_TSTP,                // SIGTSTP
    SIGNAL_TTIN,                // SIGTTIN
    SIGNAL_TTOU,                // SIGTTOU
    // ... additional signals up to MAX_SIGNALS
} signal_t;

// IPC types
typedef enum {
    IPC_TYPE_PIPE,
    IPC_TYPE_NAMED_PIPE,
    IPC_TYPE_MESSAGE_QUEUE,
    IPC_TYPE_SHARED_MEMORY,
    IPC_TYPE_SEMAPHORE,
    IPC_TYPE_MUTEX,
    IPC_TYPE_CONDITION_VARIABLE,
    IPC_TYPE_EVENT,
    IPC_TYPE_SOCKET,
    IPC_TYPE_RPC
} ipc_type_t;

// Forward declarations
typedef struct process process_t;
typedef struct thread thread_t;
typedef struct process_group process_group_t;
typedef struct session session_t;
typedef struct namespace namespace_t;

// Process credentials structure
typedef struct process_credentials {
    uint32_t real_uid, effective_uid, saved_uid;
    uint32_t real_gid, effective_gid, saved_gid;
    uint32_t* supplementary_groups;
    size_t num_groups;
    uint64_t capabilities;      // Process capabilities bitmap
    char* security_context;     // Security context string
} process_credentials_t;

// Resource limits structure
typedef struct resource_limits {
    struct {
        uint64_t soft_limit;    // Soft limit
        uint64_t hard_limit;    // Hard limit
        uint64_t current;       // Current usage
    } limits[16];               // RLIMIT_* values
} resource_limits_t;

// Signal handler structure
typedef struct signal_handler {
    void (*handler)(int signal, void* info, void* context);
    uint64_t mask;              // Signal mask during handler execution
    uint32_t flags;             // Handler flags (SA_*)
} signal_handler_t;

// Process statistics
typedef struct process_stats {
    uint64_t start_time;        // Process start time
    uint64_t cpu_time_user;     // User CPU time
    uint64_t cpu_time_kernel;   // Kernel CPU time
    uint64_t memory_usage;      // Current memory usage
    uint64_t peak_memory;       // Peak memory usage
    uint32_t page_faults_minor; // Minor page faults
    uint32_t page_faults_major; // Major page faults
    uint32_t context_switches;  // Context switch count
    uint32_t signals_sent;      // Signals sent
    uint32_t signals_received;  // Signals received
    uint32_t children_created;  // Child processes created
} process_stats_t;

// Thread information structure
typedef struct thread_info {
    uint32_t thread_id;         // Thread ID (TID)
    char name[64];              // Thread name
    thread_state_t state;       // Current state
    uint32_t priority;          // Thread priority (0-255)
    sched_policy_t policy;      // Scheduling policy
    uint64_t cpu_time;          // CPU time consumed
    void* stack_base;           // Stack base address
    size_t stack_size;          // Stack size
    uint32_t cpu_affinity;      // CPU affinity mask
    
    // Real-time attributes
    struct {
        uint32_t rt_priority;   // Real-time priority
        uint64_t deadline;      // Deadline for deadline scheduling
        uint64_t period;        // Period for periodic tasks
        uint64_t runtime;       // Runtime budget
    } rt;
    
    // Thread-local storage
    void* tls_data;
    
    // Statistics
    uint64_t context_switches;
    uint64_t preemptions;
    uint64_t yield_count;
} thread_info_t;

// Process structure (extended from existing)
struct process {
    // Basic identification
    uint32_t pid;               // Process ID
    uint32_t ppid;              // Parent process ID
    char name[PROCESS_NAME_MAX]; // Process name
    char* command_line;         // Command line arguments
    process_state_t state;      // Process state
    
    // Process hierarchy
    process_t* parent;          // Parent process
    process_t* children;        // Child processes (linked list)
    process_t* sibling;         // Next sibling
    process_group_t* pgrp;      // Process group
    session_t* session;         // Session
    
    // Threads
    thread_t* threads;          // Thread list
    uint32_t thread_count;      // Number of threads
    thread_t* main_thread;      // Main thread
    
    // Memory management
    memory_mapping_t* memory_map; // Virtual memory mappings
    void* page_directory;       // Page directory
    size_t memory_usage;        // Current memory usage
    
    // File descriptors
    struct {
        void** table;           // File descriptor table
        uint32_t count;         // Number of open files
        uint32_t max_count;     // Maximum allowed files
    } files;
    
    // Working directory and root
    void* cwd;                  // Current working directory
    void* root;                 // Root directory
    
    // Credentials and security
    process_credentials_t creds; // Process credentials
    void* security_data;        // Security framework data
    
    // Resource limits
    resource_limits_t limits;   // Resource limits
    
    // Signal handling
    signal_handler_t signal_handlers[MAX_SIGNALS];
    uint64_t pending_signals;   // Pending signals bitmap
    uint64_t blocked_signals;   // Blocked signals bitmap
    void* signal_stack;         // Alternate signal stack
    
    // Scheduling
    priority_class_t priority_class;
    int nice_value;             // Nice value (-20 to +19)
    sched_policy_t sched_policy;
    uint32_t cpu_affinity;      // CPU affinity mask
    
    // Statistics
    process_stats_t stats;
    
    // Namespaces (containers)
    namespace_t* namespaces[MAX_NAMESPACES];
    
    // Environment variables
    char** environment;         // Environment variables
    
    // Exit information
    int exit_code;              // Exit code
    bool zombie;                // Zombie state flag
    
    // Synchronization
    void* lock;                 // Process lock
    
    // Private data
    void* private_data;
};

// Thread structure
struct thread {
    // Basic identification
    uint32_t tid;               // Thread ID
    process_t* process;         // Parent process
    char name[64];              // Thread name
    thread_state_t state;       // Thread state
    
    // Linked list management
    thread_t* next;             // Next thread in process
    thread_t* prev;             // Previous thread in process
    
    // Scheduling information
    uint32_t priority;          // Thread priority
    sched_policy_t policy;      // Scheduling policy
    uint32_t cpu_affinity;      // CPU affinity mask  
    uint32_t current_cpu;       // Currently running CPU
    
    // Stack information
    void* stack_base;           // Stack base address
    size_t stack_size;          // Stack size
    void* stack_pointer;        // Current stack pointer
    
    // CPU context (architecture-specific)
    void* cpu_context;          // Saved CPU registers
    void* fpu_context;          // FPU/SIMD state
    
    // Real-time scheduling
    struct {
        uint32_t rt_priority;   // Real-time priority (1-99)
        uint64_t deadline;      // Absolute deadline
        uint64_t period;        // Period for periodic tasks
        uint64_t runtime;       // Remaining runtime budget
        uint64_t last_scheduled; // Last scheduling time
    } rt;
    
    // Thread synchronization
    void* wait_object;          // Object being waited on
    uint32_t wait_flags;        // Wait flags
    uint64_t timeout;           // Wait timeout
    
    // Thread-local storage
    void** tls_slots;           // TLS slots
    uint32_t tls_count;         // Number of TLS slots
    
    // Statistics
    thread_info_t info;
    
    // Exit information
    void* exit_value;           // Thread exit value
    bool detached;              // Detached thread flag
    
    // Synchronization
    void* lock;                 // Thread lock
    
    void* private_data;
};

// Process group structure  
struct process_group {
    uint32_t pgid;              // Process group ID
    process_t* leader;          // Process group leader
    process_t* processes;       // Processes in group
    session_t* session;         // Session containing this group
    uint32_t process_count;     // Number of processes
};

// Session structure
struct session {
    uint32_t sid;               // Session ID
    process_t* leader;          // Session leader
    process_group_t* groups;    // Process groups in session
    uint32_t group_count;       // Number of groups
    void* controlling_tty;      // Controlling terminal
};

// Namespace structure (for containers)
struct namespace {
    uint32_t ns_id;             // Namespace ID
    uint32_t type;              // Namespace type (PID, NET, MNT, etc.)
    uint32_t ref_count;         // Reference count
    void* private_data;         // Namespace-specific data
};

// IPC object structure
typedef struct ipc_object {
    uint32_t id;                // IPC object ID
    ipc_type_t type;            // IPC type
    char name[256];             // Object name (for named objects)
    uint32_t permissions;       // Access permissions
    process_credentials_t owner; // Owner credentials
    uint32_t ref_count;         // Reference count
    void* data;                 // Type-specific data
} ipc_object_t;

// Process creation parameters
typedef struct process_create_params {
    const char* executable_path; // Path to executable
    char* const* argv;          // Command line arguments
    char* const* envp;          // Environment variables
    const char* working_dir;    // Working directory
    process_credentials_t* creds; // Process credentials
    resource_limits_t* limits;  // Resource limits
    uint32_t flags;             // Creation flags
    priority_class_t priority;  // Priority class
    uint32_t cpu_affinity;      // CPU affinity
} process_create_params_t;

// Thread creation parameters
typedef struct thread_create_params {
    const char* name;           // Thread name
    void (*entry_point)(void*); // Thread entry point
    void* argument;             // Thread argument
    size_t stack_size;          // Stack size (0 = default)
    uint32_t priority;          // Thread priority
    sched_policy_t policy;      // Scheduling policy
    uint32_t cpu_affinity;      // CPU affinity
    uint32_t flags;             // Creation flags
} thread_create_params_t;

// Process and thread operations
typedef struct process_operations {
    // Process lifecycle
    int (*create_process)(process_create_params_t* params, process_t** process);
    int (*destroy_process)(process_t* process);
    int (*exec_process)(process_t* process, const char* path, char* const argv[], char* const envp[]);
    int (*fork_process)(process_t* parent, process_t** child);
    int (*wait_process)(process_t* parent, process_t* child, int* status);
    int (*exit_process)(process_t* process, int exit_code);
    
    // Thread lifecycle
    int (*create_thread)(process_t* process, thread_create_params_t* params, thread_t** thread);
    int (*destroy_thread)(thread_t* thread);
    int (*join_thread)(thread_t* thread, void** exit_value);
    int (*detach_thread)(thread_t* thread);
    void (*exit_thread)(thread_t* thread, void* exit_value);
    
    // Scheduling
    int (*schedule)(void);
    int (*yield)(thread_t* thread);
    int (*set_priority)(thread_t* thread, uint32_t priority);
    int (*set_policy)(thread_t* thread, sched_policy_t policy);
    int (*set_affinity)(thread_t* thread, uint32_t cpu_mask);
    int (*sleep)(thread_t* thread, uint64_t nanoseconds);
    
    // Signal handling
    int (*send_signal)(process_t* target, signal_t signal, void* info);
    int (*set_signal_handler)(process_t* process, signal_t signal, signal_handler_t* handler);
    int (*signal_mask)(process_t* process, uint64_t mask, uint64_t* old_mask);
    int (*signal_wait)(process_t* process, uint64_t mask, signal_t* signal);
    
    // IPC operations
    int (*create_ipc_object)(ipc_type_t type, const char* name, uint32_t permissions, ipc_object_t** obj);
    int (*destroy_ipc_object)(ipc_object_t* obj);
    int (*connect_ipc)(process_t* process, const char* name, ipc_object_t** obj);
    int (*send_message)(ipc_object_t* obj, const void* data, size_t size);
    int (*receive_message)(ipc_object_t* obj, void* buffer, size_t size, size_t* received);
    
    // Process information
    process_t* (*get_process)(uint32_t pid);
    thread_t* (*get_thread)(uint32_t tid);
    process_t* (*get_current_process)(void);
    thread_t* (*get_current_thread)(void);
    int (*enumerate_processes)(process_t** processes, size_t* count);
    int (*enumerate_threads)(process_t* process, thread_t** threads, size_t* count);
    
    // Process groups and sessions
    int (*create_process_group)(process_t* leader, process_group_t** pgrp);
    int (*join_process_group)(process_t* process, process_group_t* pgrp);
    int (*create_session)(process_t* leader, session_t** session);
    
    // Namespaces (containers)
    int (*create_namespace)(uint32_t type, namespace_t** ns);
    int (*join_namespace)(process_t* process, namespace_t* ns);
    int (*leave_namespace)(process_t* process, namespace_t* ns);
    
    // Resource management
    int (*set_resource_limit)(process_t* process, int resource, uint64_t soft, uint64_t hard);
    int (*get_resource_usage)(process_t* process, int resource, uint64_t* usage);
    
    // Statistics and monitoring
    int (*get_process_stats)(process_t* process, process_stats_t* stats);
    int (*get_thread_info)(thread_t* thread, thread_info_t* info);
    int (*get_system_stats)(uint32_t* process_count, uint32_t* thread_count);
} process_ops_t;

// Global process manager instance
extern process_ops_t* process_manager;

// Process management API functions

// Initialization
int process_init(void);
void process_cleanup(void);

// Process lifecycle
int process_create(process_create_params_t* params, process_t** process);
int process_fork(process_t** child);
int process_exec(const char* path, char* const argv[], char* const envp[]);
int process_wait(uint32_t pid, int* status);
void process_exit(int exit_code) __attribute__((noreturn));

// Thread lifecycle  
int thread_create(thread_create_params_t* params, thread_t** thread);
int thread_join(thread_t* thread, void** exit_value);
int thread_detach(thread_t* thread);
void thread_exit(void* exit_value) __attribute__((noreturn));

// Scheduling
int sched_yield(void);
int sched_set_priority(thread_t* thread, uint32_t priority);
int sched_get_priority(thread_t* thread, uint32_t* priority);
int sched_set_policy(thread_t* thread, sched_policy_t policy);
int sched_set_affinity(thread_t* thread, uint32_t cpu_mask);

// Signal handling
int signal_send(uint32_t pid, signal_t signal);
int signal_handle(signal_t signal, signal_handler_t* handler);
int signal_block(uint64_t mask);
int signal_unblock(uint64_t mask);

// Current process/thread access
process_t* current_process(void);
thread_t* current_thread(void);
uint32_t getpid(void);
uint32_t gettid(void);
uint32_t getppid(void);

// Process lookup
process_t* find_process(uint32_t pid);
thread_t* find_thread(uint32_t tid);

// Utility macros
#define THREAD_PRIORITY_MIN     0
#define THREAD_PRIORITY_NORMAL  128
#define THREAD_PRIORITY_MAX     255

#define RT_PRIORITY_MIN         1
#define RT_PRIORITY_MAX         99

#define SIGNAL_MASK(sig)        (1ULL << ((sig) - 1))
#define SIGNAL_MASK_ALL         (~0ULL)

// Process creation flags
#define PROCESS_FLAG_SUSPEND    (1 << 0)  // Create suspended
#define PROCESS_FLAG_DEBUG      (1 << 1)  // Enable debugging
#define PROCESS_FLAG_INHERIT_ENV (1 << 2) // Inherit environment
#define PROCESS_FLAG_NEW_SESSION (1 << 3) // Create new session

// Thread creation flags  
#define THREAD_FLAG_DETACHED    (1 << 0)  // Create detached
#define THREAD_FLAG_SUSPEND     (1 << 1)  // Create suspended
#define THREAD_FLAG_REALTIME    (1 << 2)  // Real-time thread

#ifdef __cplusplus
}
#endif

#endif // PROCESS_INTERFACE_H