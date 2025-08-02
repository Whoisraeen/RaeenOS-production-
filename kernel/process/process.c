// RaeenOS Process Management Implementation

#include "process.h"
#include "../vga.h"
#include "../pmm.h"
#include "../paging.h"
#include "../memory.h"     // For kmalloc/kfree
#include "../string.h"    // For memset and string functions
#include "../sync.h"      // For spinlock

// The process table
process_t process_table[MAX_PROCESSES];

// The currently running process
volatile process_t* current_process = NULL;

// The ready queues for each priority level
process_t* ready_queues[NUM_PRIORITY_LEVELS];

// The next available Process ID
static uint32_t next_pid = 0;

// Spinlock for process table and scheduler access
static spinlock_t process_lock = SPINLOCK_INIT;

// Initializes the process management system.
void process_init(void) {
    vga_puts("Initializing process management...\n");

    spinlock_acquire(&process_lock);

    // Clear the process table
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        process_table[i].pid = 0; // 0 indicates a free slot
        process_table[i].state = PROCESS_STATE_TERMINATED;
    }

    // Create the kernel's main task (PID 0)
    // This process represents the code we are currently running.
    current_process = &process_table[0];
    current_process->pid = next_pid++;
    current_process->state = PROCESS_STATE_RUNNING;
    current_process->priority_class = PRIORITY_CLASS_REALTIME; // Kernel is real-time
    memset((void*)&current_process->cpu_state, 0, sizeof(cpu_state_t));
    current_process->page_directory = paging_get_kernel_directory(); // From paging.c

    // Initialize the ready queues
    for (int i = 0; i < NUM_PRIORITY_LEVELS; ++i) {
        ready_queues[i] = NULL;
    }

    spinlock_release(&process_lock);
    vga_puts("Process management initialized. Kernel is PID 0.\n");
}

// Creates a new process.
process_t* process_create(void (*entry_point)(void)) {
    spinlock_acquire(&process_lock);

    for (int i = 1; i < MAX_PROCESSES; ++i) { // Start from 1, 0 is the kernel
        if (process_table[i].state == PROCESS_STATE_TERMINATED || process_table[i].state == PROCESS_STATE_UNUSED) {
            process_t* p = &process_table[i];
            p->pid = next_pid++;
            p->parent_pid = current_process ? current_process->pid : 0;
            p->state = PROCESS_STATE_READY;
            p->priority_class = PRIORITY_CLASS_NORMAL; // Default priority for new processes

            // Initialize file descriptor table
            for (int j = 0; j < MAX_PROCESS_FDS; ++j) {
                p->fds[j] = NULL;
                p->fd_table[j] = NULL;
            }

            // Allocate a kernel stack
            p->kernel_stack_top = (uintptr_t)pmm_alloc_frame() + PMM_FRAME_SIZE;
            if (!p->kernel_stack_top) {
                spinlock_release(&process_lock);
                return NULL; // Out of memory
            }

            // Create a new address space for the process
            p->page_directory = paging_clone_directory(current_process->page_directory);
            if (!p->page_directory) {
                pmm_free_frame((void*)(p->kernel_stack_top - PMM_FRAME_SIZE));
                spinlock_release(&process_lock);
                return NULL; // Out of memory
            }

            // Set up the initial stack for the new process
            // The stack needs to be set up to simulate a context switch return
            // The order of pushes must match the pop order in switch.asm
            uint32_t* stack = (uint32_t*)p->kernel_stack_top;

            // Push dummy values for registers that switch_context will pop
            *--stack = (uint32_t)entry_point; // EIP
            *--stack = 0x08;                  // CS (Kernel Code Segment)
            *--stack = 0x202;                 // EFLAGS (Interrupts enabled)
            *--stack = 0;                     // ESP (dummy, will be set by switch)
            *--stack = 0;                     // EBP
            *--stack = 0;                     // EBX
            *--stack = 0;                     // ESI
            *--stack = 0;                     // EDI
            *--stack = 0;                     // EDX (dummy)
            *--stack = 0;                     // ECX (dummy)
            *--stack = 0;                     // EAX (dummy)

            p->esp = (uintptr_t)stack; // Save the initial stack pointer for the process

            // Initialize the main thread for the process
            p->threads = (thread_t*)kmalloc(sizeof(thread_t));
            if (!p->threads) {
                paging_free_directory(p->page_directory);
                pmm_free_frame((void*)(p->kernel_stack_top - PMM_FRAME_SIZE));
                spinlock_release(&process_lock);
                return NULL; // Out of memory
            }
            p->threads->id = 0;
            p->threads->esp = p->esp;
            p->threads->ebp = (uintptr_t)stack; // EBP is usually the same as ESP initially for a new stack frame
            p->threads->eip = (uint32_t)entry_point;
            p->threads->parent = p;
            p->threads->next = NULL;

            // Add to the ready queue
            if (ready_queues[p->priority_class] == NULL) {
                ready_queues[p->priority_class] = p;
                p->next = p;
            } else {
                p->next = ready_queues[p->priority_class]->next;
                ready_queues[p->priority_class]->next = p;
            }

            spinlock_release(&process_lock);
            return p;
        }
    }
    spinlock_release(&process_lock);
    return NULL; // No free process slots
}

// Creates a new thread in the current process.
thread_t* thread_create(void (*entry_point)(void)) {
    spinlock_acquire(&process_lock);

    thread_t* t = (thread_t*)kmalloc(sizeof(thread_t));
    if (!t) {
        spinlock_release(&process_lock);
        return NULL; // Out of memory
    }

    t->id = current_process->threads ? current_process->threads->id + 1 : 0;
    t->parent = (process_t*)current_process;
    t->next = current_process->threads;
    current_process->threads = t;

    // Allocate a kernel stack for the new thread
    uintptr_t kernel_stack_top = (uintptr_t)pmm_alloc_frame() + PMM_FRAME_SIZE;
    if (!kernel_stack_top) {
        kfree(t);
        spinlock_release(&process_lock);
        return NULL; // Out of memory
    }

    // Set up the initial stack for the new thread
    uintptr_t* stack = (uintptr_t*)kernel_stack_top;
    *--stack = (uintptr_t)entry_point; // EIP
    *--stack = 0x202;                  // EFLAGS with interrupts enabled
    *--stack = 0;                      // EAX
    *--stack = 0;                      // ECX
    *--stack = 0;                      // EDX
    *--stack = 0;                      // EBX
    *--stack = 0;                      // ESP_dummy
    *--stack = 0;                      // EBP
    *--stack = 0;                      // ESI
    *--stack = 0;                      // EDI

    t->esp = (uintptr_t)stack;
    t->ebp = (uintptr_t)stack;
    t->eip = (uintptr_t)entry_point;

    spinlock_release(&process_lock);
    return t;
}


void process_cleanup(process_t* proc) {
    if (!proc) return;

    spinlock_acquire(&process_lock);

    // Free the process's page directory
    if (proc->page_directory) {
        paging_free_directory(proc->page_directory);
        proc->page_directory = NULL;
    }

    // Free the kernel stack
    if (proc->kernel_stack_top) {
        pmm_free_frame((void*)(proc->kernel_stack_top - PMM_FRAME_SIZE));
        proc->kernel_stack_top = 0;
    }

    // Free thread structures (if any)
    thread_t* current_thread = proc->threads;
    while (current_thread) {
        thread_t* next_thread = current_thread->next;
        kfree(current_thread);
        current_thread = next_thread;
    }
    proc->threads = NULL;

    // Mark the process entry as unused
    proc->pid = 0; // Invalidate PID
    proc->parent_pid = 0;
    proc->state = PROCESS_STATE_UNUSED;
    proc->next = NULL;

    spinlock_release(&process_lock);
}

// Exits the current process.
void process_exit(int exit_code) {
    spinlock_acquire(&process_lock);

    process_t* proc = (process_t*)current_process;
    proc->exit_code = exit_code;
    proc->state = PROCESS_STATE_ZOMBIE; // Mark as zombie

    // Remove from ready queue
    process_t* current_queue = ready_queues[proc->priority_class];
    if (current_queue == proc) {
        if (proc->next == proc) {
            ready_queues[proc->priority_class] = NULL;
        } else {
            process_t* tail = proc->next;
            while (tail->next != proc) {
                tail = tail->next;
            }
            tail->next = proc->next;
            ready_queues[proc->priority_class] = proc->next;
        }
    } else {
        process_t* prev = current_queue;
        while (prev && prev->next != proc) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = proc->next;
        }
    }

    // If parent is waiting, wake it up
    // TODO: Implement proper waiting mechanism (e.g., waitpid syscall)

    spinlock_release(&process_lock);
    schedule(); // Force a context switch
}

// Sends a signal to a process.
void process_send_signal(pid_t pid, uint32_t signal) {
    spinlock_acquire(&process_lock);

    process_t* proc = get_process(pid);
    if (proc) {
        proc->pending_signals |= (1 << signal); // Set the signal bit
        // TODO: Implement actual signal handling logic (e.g., interrupt process, run handler)
    }

    spinlock_release(&process_lock);
}

// Multi-level feedback queue scheduler.
void schedule(void) {
    spinlock_acquire(&process_lock);

    if (!current_process) {
        spinlock_release(&process_lock);
        return;
    }

    // Find the next ready process based on priority
    process_t* next_proc = NULL;
    for (int i = 0; i < NUM_PRIORITY_LEVELS; ++i) {
        if (ready_queues[i] != NULL) {
            // Implement round-robin within the same priority level
            next_proc = ready_queues[i]->next; // Get the next process in the circular list
            if (next_proc == ready_queues[i]) { // If only one process in this queue
                // No change in next_proc
            }
            break;
        }
    }

    if (next_proc != NULL) {
        // If the current process is still running and has higher or equal priority,
        // and is not the same as the next_proc, don't switch unless forced.
        // This is a basic preemption check.
        if (current_process->state == PROCESS_STATE_RUNNING && 
            current_process->priority_class <= next_proc->priority_class &&
            current_process != next_proc) {
            // No preemption, current process continues
            spinlock_release(&process_lock);
            return;
        }

        // Remove the process from the ready queue
        if (next_proc->next == next_proc) {
            ready_queues[next_proc->priority_class] = NULL;
        } else {
            process_t* tail = next_proc;
            while (tail->next != next_proc) {
                tail = tail->next;
            }
            tail->next = next_proc->next;
            ready_queues[next_proc->priority_class] = next_proc->next;
        }

        // Save current process state and switch to the new process
        process_t* prev_proc = (process_t*)current_process;
        current_process = next_proc;
        prev_proc->state = PROCESS_STATE_READY;
        next_proc->state = PROCESS_STATE_RUNNING;

        // Add the previous process back to the ready queue if it's not exiting
        if (prev_proc->state != PROCESS_STATE_ZOMBIE && prev_proc->state != PROCESS_STATE_TERMINATED) {
            if (ready_queues[prev_proc->priority_class] == NULL) {
                ready_queues[prev_proc->priority_class] = prev_proc;
                prev_proc->next = prev_proc;
            } else {
                prev_proc->next = ready_queues[prev_proc->priority_class]->next;
                ready_queues[prev_proc->priority_class]->next = prev_proc;
            }
        }

        context_switch(&prev_proc->esp, next_proc->esp);
    }

    spinlock_release(&process_lock);
}

// Gets a process by its PID.
process_t* get_process(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (process_table[i].pid == pid) {
            return &process_table[i];
        }
    }
    return NULL;
}

// Gets the currently running process.
process_t* get_current_process(void) {
    return (process_t*)current_process;
}
