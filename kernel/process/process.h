// RaeenOS Process Management
// --------------------------
// This header defines the core structures and functions for process creation,
// scheduling, and management in the RaeenOS kernel.

#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include "../paging.h"
#include "../fs/vfs.h"

#define MAX_PROCESSES 64
#define MAX_PROCESS_FDS 32

// A process ID is just an integer.
typedef int pid_t;

// Represents the state of a process.
typedef enum {
    PROCESS_STATE_UNUSED,   // This process slot is free.
    PROCESS_STATE_RUNNING,  // The process is currently running or ready to run.
    PROCESS_STATE_SLEEPING, // The process is waiting for an event.
    PROCESS_STATE_WAITING,  // The process is waiting for a child to exit.
    PROCESS_STATE_ZOMBIE,   // The process has exited but is waiting for its parent to collect it.
    PROCESS_STATE_BLOCKED,  // The process is blocked on a resource (e.g., semaphore, I/O)
} process_state_t;

// Defines the CPU state that is saved and restored during a context switch.
// This must match the layout expected by switch.asm.
typedef struct cpu_state {
    uint32_t edi, esi, ebp, ebx;
    uint32_t eip, cs, eflags, esp;
} cpu_state_t;

typedef struct thread {
    uint32_t id;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    struct process* parent;
    struct thread* next;
} thread_t;

typedef enum {
    PRIORITY_CLASS_REALTIME,
    PRIORITY_CLASS_HIGH,
    PRIORITY_CLASS_NORMAL,
    PRIORITY_CLASS_LOW
} priority_class_t;

// The Process Control Block (PCB).
typedef struct process {
    pid_t pid;                      // Process ID.
    process_state_t state;          // Current process state.
    priority_class_t priority_class; // Scheduling priority class
    cpu_state_t cpu_state;          // Saved CPU state for context switching.
    page_directory_t* page_directory; // Page directory for the process's address space.
    struct process* parent;         // Pointer to the parent process.
    int exit_code;                  // Exit code, valid when state is ZOMBIE.
    vfs_node_t* fds[MAX_PROCESS_FDS]; // Per-process file descriptor table.
    thread_t* threads;              // List of threads in this process.
    struct process* next;           // For linked list in ready queue
    int parent_pid;                 // PID of the parent process
    uintptr_t kernel_stack_top;     // Top of the kernel stack for this process
    uintptr_t esp;                  // Saved ESP for context switching
    uint32_t pending_signals;       // Bitmask of pending signals
    struct registers_t regs;        // Saved registers for system calls and interrupts
} process_t;

#define NUM_PRIORITY_LEVELS 4

// The ready queue for each priority level
extern process_t* ready_queues[NUM_PRIORITY_LEVELS];

// The global process table.
extern process_t process_table[MAX_PROCESSES];

// Initializes the process management system.
void process_init(void);

// Creates a new process.
process_t* process_create(void (*entry_point)(void));

// Cleans up a terminated process.
void process_cleanup(process_t* proc);

// Gets a process by its PID.
process_t* get_process(pid_t pid);

// Gets the currently running process.
process_t* get_current_process(void);

// The scheduler.
void schedule(void);

// Creates a new thread in the current process.
thread_t* thread_create(void (*entry_point)(void));

#endif // _PROCESS_H