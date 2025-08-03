/**
 * Advanced Process and Thread Management for RaeenOS
 * Implements complete process lifecycle, scheduling, signals, and IPC
 */

#include "process/process.h"
#include "process_advanced.h"
#include "memory_advanced.h"
#include "scheduler.h"
#include "signals.h"
#include "ipc.h"
#include "security.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================
// ADVANCED PROCESS STRUCTURES
// ============================================================================

typedef enum {
    PROCESS_STATE_NEW,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_SUSPENDED,
    PROCESS_STATE_ZOMBIE,
    PROCESS_STATE_TERMINATED
} process_state_t;

typedef enum {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_WAITING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_SUSPENDED,
    THREAD_STATE_TERMINATED
} thread_state_t;

typedef struct thread {
    uint32_t tid;
    char name[32];
    thread_state_t state;

    // CPU context
    cpu_context_t context;
    void* stack_base;
    size_t stack_size;
    void* kernel_stack;

    // Scheduling information
    int priority;
    int nice_value;
    uint64_t cpu_time_used;
    uint64_t last_scheduled;
    uint32_t time_slice;
    uint32_t remaining_slice;

    // Thread-specific data
    void* tls_data;
    size_t tls_size;

    // Synchronization
    struct wait_queue* wait_queue;
    uint32_t wait_reason;
    void* wait_object;

    // Signal handling
    sigset_t pending_signals;
    sigset_t blocked_signals;
    struct sigaction signal_handlers[MAX_SIGNALS];

    // Parent process
    struct process* process;

    // Thread list management
    struct thread* next;
    struct thread* prev;

    // Performance counters
    struct {
        uint64_t context_switches;
        uint64_t page_faults;
        uint64_t system_calls;
        uint64_t cache_misses;
        uint64_t instructions_executed;
    } perf_counters;
} thread_t;

typedef struct process {
    uint32_t pid;
    uint32_t ppid;
    char name[64];
    char cmdline[256];
    process_state_t state;

    // Memory management
    address_space_t* address_space;

    // Thread management
    thread_t* main_thread;
    thread_t* threads;
    uint32_t thread_count;
    spinlock_t thread_lock;

    // File descriptors
    struct file_descriptor_table* fd_table;

    // Security context
    struct security_context* security;
    uid_t uid, euid, suid;
    gid_t gid, egid, sgid;
    uint32_t capabilities;

    // Resource limits
    struct resource_limits {
        uint64_t max_memory;
        uint64_t max_cpu_time;
        uint32_t max_threads;
        uint32_t max_files;
        uint32_t max_processes;
    } limits;

    // Resource usage
    struct resource_usage {
        uint64_t memory_used;
        uint64_t cpu_time_used;
        uint64_t disk_io_bytes;
        uint64_t network_io_bytes;
        uint32_t open_files;
    } usage;

    // Process tree
    struct process* parent;
    struct process* children;
    struct process* sibling_next;
    struct process* sibling_prev;

    // Exit information
    int exit_code;
    bool has_exited;
    struct wait_queue* exit_waiters;

    // Debugging support
    struct {
        bool being_traced;
        uint32_t tracer_pid;
        uint32_t debug_flags;
        struct breakpoint* breakpoints;
        uint32_t breakpoint_count;
    } debug_info;

    // IPC objects
    struct {
        struct message_queue* msg_queues;
        struct shared_memory* shared_mem;
        struct semaphore* semaphores;
        struct pipe* pipes;
    } ipc;

    // Process list management
    struct process* next;
    struct process* prev;

    // Timing information
    uint64_t creation_time;
    uint64_t start_time;
    uint64_t end_time;

    // Environment
    char** environment;
    uint32_t env_count;

    spinlock_t lock;
} process_t;

// ============================================================================
// SCHEDULER STRUCTURES
// ============================================================================

typedef enum {
    SCHED_POLICY_NORMAL,
    SCHED_POLICY_FIFO,
    SCHED_POLICY_RR,
    SCHED_POLICY_BATCH,
    SCHED_POLICY_IDLE,
    SCHED_POLICY_REALTIME
} sched_policy_t;

typedef struct run_queue {
    thread_t* head;
    thread_t* tail;
    uint32_t count;
    spinlock_t lock;
} run_queue_t;

typedef struct cpu_scheduler {
    uint32_t cpu_id;
    thread_t* current_thread;
    thread_t* idle_thread;

    // Multi-level feedback queues
    run_queue_t priority_queues[MAX_PRIORITY_LEVELS];
    run_queue_t realtime_queue;
    run_queue_t batch_queue;

    // Load balancing
    uint64_t load_average;
    uint32_t running_tasks;

    // Timing
    uint64_t last_tick;
    uint32_t tick_count;

    spinlock_t lock;
} cpu_scheduler_t;

// ============================================================================
// SIGNAL HANDLING
// ============================================================================

typedef struct signal_info {
    int signal_number;
    int code;
    union {
        struct {
            pid_t sender_pid;
            uid_t sender_uid;
        } kill;
        struct {
            void* addr;
            int code;
        } fault;
        struct {
            int fd;
            int band;
        } poll;
    } data;
} signal_info_t;

typedef struct signal_queue {
    signal_info_t* signals;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t capacity;
    spinlock_t lock;
} signal_queue_t;

// ============================================================================
// GLOBAL STATE
// ============================================================================

static process_t* process_list = NULL;
static process_t* init_process = NULL;
static uint32_t next_pid = 1;
static uint32_t next_tid = 1;
static spinlock_t process_list_lock;

static cpu_scheduler_t* cpu_schedulers;
static uint32_t num_cpus;

static signal_queue_t global_signal_queue;

// ============================================================================
// PROCESS MANAGEMENT IMPLEMENTATION
// ============================================================================

int process_system_init(void) {
    // Initialize process list
    spinlock_init(&process_list_lock);

    // Initialize per-CPU schedulers
    num_cpus = get_cpu_count();
    cpu_schedulers = kmalloc(sizeof(cpu_scheduler_t) * num_cpus);
    if (!cpu_schedulers) {
        return -ENOMEM;
    }

    for (uint32_t i = 0; i < num_cpus; i++) {
        cpu_scheduler_t* sched = &cpu_schedulers[i];
        sched->cpu_id = i;
        sched->current_thread = NULL;
        sched->load_average = 0;
        sched->running_tasks = 0;
        spinlock_init(&sched->lock);

        // Initialize priority queues
        for (int j = 0; j < MAX_PRIORITY_LEVELS; j++) {
            run_queue_init(&sched->priority_queues[j]);
        }
        run_queue_init(&sched->realtime_queue);
        run_queue_init(&sched->batch_queue);

        // Create idle thread for this CPU
        sched->idle_thread = create_idle_thread(i);
    }

    // Initialize signal system
    signal_system_init();

    // Create init process (PID 1)
    init_process = create_init_process();
    if (!init_process) {
        return -ENOMEM;
    }

    return 0;
}

process_t* process_create(const char* name, process_t* parent) {
    process_t* proc = kmalloc(sizeof(process_t));
    if (!proc) {
        return NULL;
    }

    memset(proc, 0, sizeof(process_t));

    // Assign PID
    spinlock_acquire(&process_list_lock);
    proc->pid = next_pid++;
    spinlock_release(&process_list_lock);

    // Basic initialization
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->state = PROCESS_STATE_NEW;
    proc->creation_time = get_system_time();

    // Set parent relationship
    if (parent) {
        proc->ppid = parent->pid;
        proc->parent = parent;

        // Add to parent's children list
        spinlock_acquire(&parent->lock);
        proc->sibling_next = parent->children;
        if (parent->children) {
            parent->children->sibling_prev = proc;
        }
        parent->children = proc;
        spinlock_release(&parent->lock);

        // Inherit security context
        proc->uid = parent->uid;
        proc->gid = parent->gid;
        proc->euid = parent->euid;
        proc->egid = parent->egid;
    } else {
        proc->ppid = 0;
        proc->uid = proc->euid = 0;  // Root
        proc->gid = proc->egid = 0;
    }

    // Create address space
    proc->address_space = address_space_create(proc->pid);
    if (!proc->address_space) {
        kfree(proc);
        return NULL;
    }

    // Initialize file descriptor table
    proc->fd_table = fd_table_create();
    if (!proc->fd_table) {
        address_space_destroy(proc->address_space);
        kfree(proc);
        return NULL;
    }

    // Initialize locks
    spinlock_init(&proc->lock);
    spinlock_init(&proc->thread_lock);

    // Set default resource limits
    set_default_resource_limits(proc);

    // Add to global process list
    spinlock_acquire(&process_list_lock);
    proc->next = process_list;
    if (process_list) {
        process_list->prev = proc;
    }
    process_list = proc;
    spinlock_release(&process_list_lock);

    return proc;
}

thread_t* thread_create(process_t* process, void* entry_point, void* arg,
                       size_t stack_size) {
    if (!process || !entry_point) {
        return NULL;
    }

    thread_t* thread = kmalloc(sizeof(thread_t));
    if (!thread) {
        return NULL;
    }

    memset(thread, 0, sizeof(thread_t));

    // Assign TID
    thread->tid = atomic_inc(&next_tid);
    thread->state = THREAD_STATE_NEW;
    thread->process = process;

    // Allocate stack
    if (stack_size == 0) {
        stack_size = DEFAULT_STACK_SIZE;
    }

    thread->stack_base = memory_map_region(process->address_space,
        0, stack_size, MEMORY_PROT_READ | MEMORY_PROT_WRITE, MAP_PRIVATE);
    if (!thread->stack_base) {
        kfree(thread);
        return NULL;
    }
    thread->stack_size = stack_size;

    // Allocate kernel stack
    thread->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    if (!thread->kernel_stack) {
        memory_unmap_region(process->address_space,
            (uintptr_t)thread->stack_base, stack_size);
        kfree(thread);
        return NULL;
    }

    // Initialize CPU context
    cpu_context_init(&thread->context, entry_point,
        (void*)((uintptr_t)thread->stack_base + stack_size), arg);

    // Set default priority
    thread->priority = DEFAULT_PRIORITY;
    thread->time_slice = calculate_time_slice(thread->priority);
    thread->remaining_slice = thread->time_slice;

    // Initialize signal handling
    sigemptyset(&thread->pending_signals);
    sigemptyset(&thread->blocked_signals);
    init_default_signal_handlers(thread);

    // Add to process thread list
    spinlock_acquire(&process->thread_lock);
    thread->next = process->threads;
    if (process->threads) {
        process->threads->prev = thread;
    }
    process->threads = thread;
    process->thread_count++;

    // Set as main thread if this is the first thread
    if (!process->main_thread) {
        process->main_thread = thread;
    }

    spinlock_release(&process->thread_lock);

    return thread;
}

// ============================================================================
// ADVANCED SCHEDULER IMPLEMENTATION
// ============================================================================

void scheduler_tick(void) {
    uint32_t cpu_id = get_current_cpu_id();
    cpu_scheduler_t* sched = &cpu_schedulers[cpu_id];

    spinlock_acquire(&sched->lock);

    thread_t* current = sched->current_thread;
    if (!current || current == sched->idle_thread) {
        spinlock_release(&sched->lock);
        return;
    }

    // Update timing information
    uint64_t now = get_system_time();
    uint64_t time_used = now - sched->last_tick;
    current->cpu_time_used += time_used;
    current->process->usage.cpu_time_used += time_used;
    sched->last_tick = now;

    // Decrement time slice
    if (current->remaining_slice > time_used) {
        current->remaining_slice -= time_used;
    } else {
        current->remaining_slice = 0;
    }

    // Check if thread needs to be preempted
    bool should_preempt = false;

    // Time slice expired
    if (current->remaining_slice == 0) {
        should_preempt = true;
    }

    // Higher priority thread became ready
    if (has_higher_priority_ready_thread(sched, current->priority)) {
        should_preempt = true;
    }

    // Real-time thread became ready
    if (!run_queue_empty(&sched->realtime_queue)) {
        should_preempt = true;
    }

    if (should_preempt) {
        schedule_next_thread(sched);
    }

    spinlock_release(&sched->lock);
}

void schedule(void) {
    uint32_t cpu_id = get_current_cpu_id();
    cpu_scheduler_t* sched = &cpu_schedulers[cpu_id];

    spinlock_acquire(&sched->lock);
    schedule_next_thread(sched);
    spinlock_release(&sched->lock);
}

static void schedule_next_thread(cpu_scheduler_t* sched) {
    thread_t* current = sched->current_thread;
    thread_t* next = NULL;

    // Save current thread state if it's still runnable
    if (current && current->state == THREAD_STATE_RUNNING) {
        current->state = THREAD_STATE_READY;
        enqueue_thread(sched, current);
    }

    // Select next thread to run
    next = select_next_thread(sched);

    if (next == current) {
        // No need to switch
        if (current) {
            current->state = THREAD_STATE_RUNNING;
            current->remaining_slice = current->time_slice;
        }
        return;
    }

    // Perform context switch
    if (next) {
        next->state = THREAD_STATE_RUNNING;
        next->remaining_slice = next->time_slice;
        next->last_scheduled = get_system_time();
        next->perf_counters.context_switches++;
    }

    sched->current_thread = next;

    // Switch address space if needed
    if (current && next &&
        current->process->address_space != next->process->address_space) {
        switch_address_space(next->process->address_space);
    }

    // Perform actual CPU context switch
    if (current && next) {
        cpu_context_switch(&current->context, &next->context);
    } else if (next) {
        cpu_context_restore(&next->context);
    }
}

static thread_t* select_next_thread(cpu_scheduler_t* sched) {
    thread_t* next = NULL;

    // Check real-time queue first
    if (!run_queue_empty(&sched->realtime_queue)) {
        next = run_queue_dequeue(&sched->realtime_queue);
        if (next) return next;
    }

    // Check priority queues (highest to lowest)
    for (int i = MAX_PRIORITY_LEVELS - 1; i >= 0; i--) {
        if (!run_queue_empty(&sched->priority_queues[i])) {
            next = run_queue_dequeue(&sched->priority_queues[i]);
            if (next) return next;
        }
    }

    // Check batch queue
    if (!run_queue_empty(&sched->batch_queue)) {
        next = run_queue_dequeue(&sched->batch_queue);
        if (next) return next;
    }

    // Fall back to idle thread
    return sched->idle_thread;
}

// ============================================================================
// SIGNAL HANDLING IMPLEMENTATION
// ============================================================================

int signal_send(pid_t pid, int signal) {
    process_t* target = find_process_by_pid(pid);
    if (!target) {
        return -ESRCH;
    }

    // Check permissions
    if (!can_send_signal_to_process(get_current_process(), target, signal)) {
        return -EPERM;
    }

    return signal_deliver_to_process(target, signal, NULL);
}

int signal_deliver_to_process(process_t* process, int signal,
                             signal_info_t* info) {
    if (!process || signal < 1 || signal >= MAX_SIGNALS) {
        return -EINVAL;
    }

    spinlock_acquire(&process->lock);

    // Find a thread to deliver the signal to
    thread_t* target_thread = NULL;

    // Try to find a thread that's not blocking this signal
    thread_t* thread = process->threads;
    while (thread) {
        if (!sigismember(&thread->blocked_signals, signal)) {
            target_thread = thread;
            break;
        }
        thread = thread->next;
    }

    // If all threads are blocking, deliver to main thread
    if (!target_thread) {
        target_thread = process->main_thread;
    }

    if (target_thread) {
        signal_deliver_to_thread(target_thread, signal, info);
    }

    spinlock_release(&process->lock);
    return 0;
}

int signal_deliver_to_thread(thread_t* thread, int signal,
                            signal_info_t* info) {
    if (!thread || signal < 1 || signal >= MAX_SIGNALS) {
        return -EINVAL;
    }

    // Add signal to pending set
    sigaddset(&thread->pending_signals, signal);

    // Queue detailed signal information if provided
    if (info) {
        signal_queue_add(&thread->signal_queue, info);
    }

    // Wake up thread if it's waiting
    if (thread->state == THREAD_STATE_WAITING ||
        thread->state == THREAD_STATE_BLOCKED) {
        thread_wakeup(thread);
    }

    return 0;
}

void signal_handle_pending(thread_t* thread) {
    if (!thread) return;

    for (int signal = 1; signal < MAX_SIGNALS; signal++) {
        if (sigismember(&thread->pending_signals, signal) &&
            !sigismember(&thread->blocked_signals, signal)) {

            // Remove from pending set
            sigdelset(&thread->pending_signals, signal);

            // Get signal handler
            struct sigaction* action = &thread->signal_handlers[signal];

            if (action->sa_handler == SIG_IGN) {
                // Ignore signal
                continue;
            } else if (action->sa_handler == SIG_DFL) {
                // Default action
                signal_default_action(thread, signal);
            } else {
                // Custom handler
                signal_execute_handler(thread, signal, action);
            }
        }
    }
}

// ============================================================================
// PROCESS DEBUGGING SUPPORT
// ============================================================================

int process_attach_debugger(pid_t pid, pid_t debugger_pid) {
    process_t* target = find_process_by_pid(pid);
    process_t* debugger = find_process_by_pid(debugger_pid);

    if (!target || !debugger) {
        return -ESRCH;
    }

    // Check permissions
    if (!can_debug_process(debugger, target)) {
        return -EPERM;
    }

    spinlock_acquire(&target->lock);

    if (target->debug_info.being_traced) {
        spinlock_release(&target->lock);
        return -EBUSY;
    }

    target->debug_info.being_traced = true;
    target->debug_info.tracer_pid = debugger_pid;
    target->debug_info.debug_flags = DEBUG_FLAG_ATTACHED;

    spinlock_release(&target->lock);

    return 0;
}

int process_set_breakpoint(pid_t pid, uintptr_t address,
                          breakpoint_type_t type) {
    process_t* target = find_process_by_pid(pid);
    if (!target) {
        return -ESRCH;
    }

    if (!target->debug_info.being_traced) {
        return -EPERM;
    }

    // Allocate new breakpoint
    struct breakpoint* bp = kmalloc(sizeof(struct breakpoint));
    if (!bp) {
        return -ENOMEM;
    }

    bp->address = address;
    bp->type = type;
    bp->enabled = true;
    bp->hit_count = 0;

    // Save original instruction
    if (type == BREAKPOINT_TYPE_EXECUTION) {
        if (copy_from_user(&bp->original_data, (void*)address,
                          sizeof(bp->original_data)) < 0) {
            kfree(bp);
            return -EFAULT;
        }

        // Write breakpoint instruction
        uint8_t bp_instruction = BREAKPOINT_INSTRUCTION;
        if (copy_to_user((void*)address, &bp_instruction, 1) < 0) {
            kfree(bp);
            return -EFAULT;
        }
    }

    // Add to breakpoint list
    spinlock_acquire(&target->lock);
    bp->next = target->debug_info.breakpoints;
    target->debug_info.breakpoints = bp;
    target->debug_info.breakpoint_count++;
    spinlock_release(&target->lock);

    return 0;
}

// ============================================================================
// INTER-PROCESS COMMUNICATION
// ============================================================================

int ipc_create_message_queue(process_t* process, key_t key, int flags) {
    struct message_queue* mq = kmalloc(sizeof(struct message_queue));
    if (!mq) {
        return -ENOMEM;
    }

    mq->key = key;
    mq->flags = flags;
    mq->max_messages = DEFAULT_MAX_MESSAGES;
    mq->max_message_size = DEFAULT_MAX_MESSAGE_SIZE;
    mq->message_count = 0;
    mq->messages = NULL;
    mq->owner_pid = process->pid;
    spinlock_init(&mq->lock);

    // Add to process IPC objects
    spinlock_acquire(&process->lock);
    mq->next = process->ipc.msg_queues;
    process->ipc.msg_queues = mq;
    spinlock_release(&process->lock);

    return mq->id;
}

int ipc_send_message(int queue_id, const void* message, size_t size,
                    int priority) {
    struct message_queue* mq = find_message_queue(queue_id);
    if (!mq) {
        return -EINVAL;
    }

    if (size > mq->max_message_size) {
        return -EMSGSIZE;
    }

    spinlock_acquire(&mq->lock);

    if (mq->message_count >= mq->max_messages) {
        spinlock_release(&mq->lock);
        return -EAGAIN;
    }

    // Allocate message
    struct message* msg = kmalloc(sizeof(struct message) + size);
    if (!msg) {
        spinlock_release(&mq->lock);
        return -ENOMEM;
    }

    msg->size = size;
    msg->priority = priority;
    msg->sender_pid = get_current_process()->pid;
    msg->timestamp = get_system_time();
    memcpy(msg->data, message, size);

    // Insert into queue (ordered by priority)
    insert_message_by_priority(mq, msg);
    mq->message_count++;

    // Wake up waiting receivers
    wait_queue_wakeup_all(&mq->receive_waiters);

    spinlock_release(&mq->lock);
    return 0;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

void process_update_performance_counters(process_t* process) {
    if (!process) return;

    // Update CPU usage
    uint64_t total_cpu_time = 0;
    thread_t* thread = process->threads;
    while (thread) {
        total_cpu_time += thread->cpu_time_used;
        thread = thread->next;
    }
    process->usage.cpu_time_used = total_cpu_time;

    // Update memory usage
    process->usage.memory_used = calculate_process_memory_usage(process);

    // Update I/O statistics
    update_process_io_stats(process);
}

void get_process_statistics(pid_t pid, process_stats_t* stats) {
    process_t* process = find_process_by_pid(pid);
    if (!process || !stats) {
        return;
    }

    memset(stats, 0, sizeof(process_stats_t));

    spinlock_acquire(&process->lock);

    stats->pid = process->pid;
    stats->ppid = process->ppid;
    strncpy(stats->name, process->name, sizeof(stats->name) - 1);
    stats->state = process->state;
    stats->thread_count = process->thread_count;

    stats->memory_usage = process->usage.memory_used;
    stats->cpu_time = process->usage.cpu_time_used;
    stats->creation_time = process->creation_time;

    stats->page_faults = 0;
    stats->context_switches = 0;

    // Aggregate thread statistics
    thread_t* thread = process->threads;
    while (thread) {
        stats->page_faults += thread->perf_counters.page_faults;
        stats->context_switches += thread->perf_counters.context_switches;
        thread = thread->next;
    }

    spinlock_release(&process->lock);
}
