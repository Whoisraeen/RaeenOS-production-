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
// Using types.h for kernel build

// File operation flags (POSIX-compatible)
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0040
#define O_EXCL      0x0080
#define O_TRUNC     0x0200
#define O_APPEND    0x0400

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
    // For now, assume the path and argv are valid kernel addresses

    // Count arguments in argv
    int argc = 0;
    if (argv) {
        while (argv[argc] != NULL) {
            argc++;
        }
    }

    // Load the executable using exec_load_into_address_space
    process_t* current = get_current_process();
    
    // Free the current user-space pages
    paging_free_user_pages(current->page_directory);
    
    // Load the new executable into the current address space
    uint32_t entry_point = exec_load_into_address_space(path, current->page_directory);
    
    if (entry_point == 0) {
        return -ENOENT; // Failed to load executable
    }

    // Set up the new user stack with arguments
    // TODO: Implement proper argument passing on the stack
    uintptr_t user_stack_ptr = USER_STACK_TOP - sizeof(uintptr_t);

    // Modify the interrupt frame to start the new program
    regs->eip = entry_point;
    regs->useresp = user_stack_ptr;
    
    // Reset the stack pointer for user mode
    regs->esp = user_stack_ptr;

    // exec does not return on success - the return value is irrelevant
    // as the process will start executing at the new entry point
    return 0;
}

// --- File I/O System Calls ---

int sys_open(const char* path, int flags, int mode) {
    // Validate user-space pointer
    if (!is_valid_userspace_ptr((void*)path, strlen(path) + 1)) {
        return -EFAULT;
    }

    process_t* current = get_current_process();
    
    // Find an available file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_PROCESS_FDS; i++) {
        if (current->fd_table[i] == NULL) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        return -EMFILE; // Too many open files
    }

    // Use VFS to open the file
    vfs_node_t* node = vfs_find(path);
    if (!node) {
        if (flags & O_CREAT) {
            // Create the file if it doesn't exist and O_CREAT is set
            // TODO: Implement file creation through VFS
            return -ENOENT; // For now, return file not found
        } else {
            return -ENOENT; // File not found
        }
    }

    // Check permissions
    // TODO: Implement proper permission checking based on mode and node permissions

    // Open the file through VFS
    if (node->open) {
        node->open(node, flags);
    }

    // Assign the node to the file descriptor
    current->fd_table[fd] = node;
    
    return fd;
}

int sys_close(int fd) {
    process_t* current = get_current_process();
    
    // Validate file descriptor
    if (fd < 0 || fd >= MAX_PROCESS_FDS || current->fd_table[fd] == NULL) {
        return -EBADF; // Bad file descriptor
    }

    vfs_node_t* node = current->fd_table[fd];
    
    // Close the file through VFS
    if (node->close) {
        node->close(node);
    }

    // Clear the file descriptor
    current->fd_table[fd] = NULL;
    
    return 0;
}

int sys_read(int fd, void* buffer, size_t count) {
    process_t* current = get_current_process();
    
    // Validate file descriptor
    if (fd < 0 || fd >= MAX_PROCESS_FDS || current->fd_table[fd] == NULL) {
        return -EBADF; // Bad file descriptor
    }

    // Validate user-space buffer
    if (!is_valid_userspace_ptr(buffer, count)) {
        return -EFAULT;
    }

    vfs_node_t* node = current->fd_table[fd];
    
    // Check if the node supports reading
    if (!node->read) {
        return -EINVAL; // Invalid operation
    }

    // Read from the file through VFS
    // TODO: Handle file position properly - for now assume sequential reads from 0
    uint32_t bytes_read = node->read(node, 0, count, (uint8_t*)buffer);
    
    return (int)bytes_read;
}

int sys_write(int fd, const void* buffer, size_t count) {
    process_t* current = get_current_process();
    
    // Validate file descriptor
    if (fd < 0 || fd >= MAX_PROCESS_FDS || current->fd_table[fd] == NULL) {
        return -EBADF; // Bad file descriptor
    }

    // Validate user-space buffer
    if (!is_valid_userspace_ptr((void*)buffer, count)) {
        return -EFAULT;
    }

    vfs_node_t* node = current->fd_table[fd];
    
    // Check if the node supports writing
    if (!node->write) {
        return -EINVAL; // Invalid operation
    }

    // Write to the file through VFS
    // TODO: Handle file position properly - for now assume sequential writes from 0
    uint32_t bytes_written = node->write(node, 0, count, (const uint8_t*)buffer);
    
    return (int)bytes_written;
}
