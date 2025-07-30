// RaeenOS System Call Implementation (Recreated)
// -----------------------------------------------
// This file provides the implementation for the system call interface.
// It was created to replace a corrupted and inaccessible syscall.c file.

#include "include/syscall.h"
#include "idt.h"
#include "process/process.h"
#include "fs/vfs.h"
#include "exec.h"
#include "paging.h"
#include "string.h"
#include "../userland/include/errno.h"
#include <stdbool.h>

// Array of function pointers for the system call handlers.
static void* syscall_routines[] = {
    [SYS_EXIT] = sys_exit,
    [SYS_FORK] = sys_fork,
    [SYS_EXEC] = sys_exec,
    [SYS_WAIT] = sys_wait,
    [SYS_OPEN] = sys_open,
    [SYS_CLOSE] = sys_close,
    [SYS_READ] = sys_read,
    [SYS_WRITE] = sys_write,
};

// The main system call handler, called from the interrupt handler.
void syscall_dispatcher(struct registers_t* regs) {
    // Check if the syscall number is valid
    if (regs->eax >= sizeof(syscall_routines) / sizeof(void*)) {
        regs->eax = -ENOSYS; // Invalid syscall
        return;
    }

    // Get the handler function from the table
    void* handler = syscall_routines[regs->eax];

    if (handler) {
        // The syscall ABI: eax = num, ebx, ecx, edx are args.
        // We need a way to pass the full register state to handlers that need it (fork, exec).
        // For now, we cast based on the known signature.
        // A more advanced ABI would use a single struct pointer.

        int ret;
        if (regs->eax == SYS_FORK || regs->eax == SYS_EXEC) {
            // These handlers need the full register frame
            ret = ((int (*)(struct registers_t*))handler)(regs);
        } else {
            // Standard handlers
            ret = ((int (*)())handler)(regs->ebx, regs->ecx, regs->edx);
        }
        regs->eax = ret;
    }
}

// --- Process Management Syscalls ---

void sys_exit(int status) {
    process_t* current = get_current_process();

    // Don't let the kernel process exit
    if (current->pid == 0) {
        // In a real scenario, this would be a kernel panic.
        // For now, we just prevent it.
        return;
    }

    // Clean up resources (VFS nodes, etc.) - to be implemented

    // Set state to zombie and save exit code
    current->state = PROCESS_STATE_ZOMBIE;
    current->exit_code = status;

    // Find parent and wake it if it's waiting
    process_t* parent = get_process(current->parent_pid);
    if (parent && parent->state == PROCESS_STATE_WAITING) {
        parent->state = PROCESS_STATE_READY;
    }

    // Yield the CPU. This process will not run again.
    schedule();
}

int sys_wait(int* status) {
    process_t* current = get_current_process();

    while (1) {
        bool has_children = false;
        // Find a zombie child
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].state != PROCESS_STATE_UNUSED && process_table[i].parent_pid == current->pid) {
                has_children = true;
                if (process_table[i].state == PROCESS_STATE_ZOMBIE) {
                    pid_t child_pid = process_table[i].pid;
                    if (status) {
                        // TODO: Need to safely copy to user memory
                        *status = process_table[i].exit_code;
                    }
                    // Clean up the zombie process
                    process_cleanup(&process_table[i]);
                    return child_pid;
                }
            }
        }

        if (!has_children) {
            return -ECHILD; // No children to wait for
        }

        // If no zombie child was found, wait.
        current->state = PROCESS_STATE_WAITING;
        schedule();
    }
}

int sys_fork(struct registers_t* regs) {
    process_t* parent = get_current_process();
    process_t* child = process_create(NULL); // Create a new process

    if (!child) {
        return -ENOMEM;
    }

    // Clone the parent's address space
    child->page_directory = paging_clone_directory(parent->page_directory);
    if (!child->page_directory) {
        process_cleanup(child);
        return -ENOMEM;
    }

    // Copy parent's file descriptors
    memcpy(child->fd_table, parent->fd_table, sizeof(parent->fd_table));

    // The child process starts from here, but with a different return value.
    // We copy the parent's register state to the child.
    // The child's kernel stack is already set up by process_create.
    // We need to set up the interrupt frame for the new process.
    // A simple way is to copy the parent's frame and modify it.
    uintptr_t child_stack_ptr = child->kernel_stack_top;
    struct registers_t* child_regs = (struct registers_t*)(child_stack_ptr - sizeof(struct registers_t));
    memcpy(child_regs, regs, sizeof(struct registers_t));

    // Set the return value for the child process to 0
    child_regs->eax = 0;

    // Set the child's ESP to point to the copied registers
    child->esp = (uintptr_t)child_regs;

    // Set child's parent
    child->parent_pid = parent->pid;

    // Set child state to ready
    child->state = PROCESS_STATE_READY;

    // Return the child's PID to the parent
    return child->pid;
}

int sys_exec(struct registers_t* regs) {
    // Extract arguments from parent's registers
    const char* path = (const char*)regs->ebx;
    const char* const* argv = (const char* const*)regs->ecx;

    // TODO: Safely read path and argv from user space

    // Load the executable
    uintptr_t entry_point;
    page_directory_t* new_page_dir = exec_load(path, argv, &entry_point);

    if (!new_page_dir) {
        return -ENOENT; // Or other error from exec_load
    }

    // Switch to the new address space
    process_t* current = get_current_process();
    paging_free_user_pages(current->page_directory);
    // kfree(current->page_directory); // We should free the old directory structure
    current->page_directory = new_page_dir;
    paging_switch_directory(new_page_dir);

    // TODO: Set up the new user stack with arguments
    uintptr_t user_stack_ptr = USER_STACK_TOP;

    // Modify the interrupt frame to start the new program
    regs->eip = entry_point;
    regs->useresp = user_stack_ptr;

    // exec does not return on success
    return 0; // This return value will be in eax when the new program starts
}
