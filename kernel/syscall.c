// RaeenOS System Call Implementation

#include "include/syscall.h"
#include "idt.h"
#include "exec.h"
#include "vga.h"
#include "process/process.h"
#include "fs/vfs.h"
#include "string.h"
#include "paging.h"
#include "exec.h"
#include "process/process.h"
#include "include/errno.h"
#include "include/time.h"
#include "include/fcntl.h"

// Syscall handler prototypes
static int sys_exit(int code);
static int sys_write(int fd, const void* buf, size_t count);
static int sys_read(int fd, void* buf, size_t count);
static int sys_open(const char* path, int flags);
static int sys_close(int fd);
static int sys_fork(void);
static int sys_exec(const char* path, char* const argv[]);
static int sys_wait(int* status);

// Extern the current_process so we can access its fds
extern volatile process_t* current_process;
// Extern the full process table for waiting
extern process_t process_table[MAX_PROCESSES];

// Array of syscall handlers, indexed by syscall number
static void* syscalls[] = {
    [SYS_EXIT]  = sys_exit,
    [SYS_WRITE] = sys_write,
    [SYS_OPEN]  = sys_open,
    [SYS_CLOSE] = sys_close,
    [SYS_READ]  = sys_read,
    [SYS_FORK] = sys_fork,
    [SYS_EXEC] = sys_exec,
    [SYS_WAIT] = sys_wait,
    [SYS_GETPID] = sys_getpid,
    [SYS_WAITPID] = sys_waitpid,
    [SYS_PIPE] = sys_pipe,
    [SYS_DUP2] = sys_dup2,
    [SYS_KILL] = sys_kill,
    [SYS_GETTIMEOFDAY] = sys_gettimeofday,
};

#define NUM_SYSCALLS (sizeof(syscalls) / sizeof(void*))

// The main system call dispatcher
static void syscall_dispatcher(struct registers_t* regs) {
    if (regs->eax >= NUM_SYSCALLS || !syscalls[regs->eax]) {
        regs->eax = -1; // Error: Invalid syscall
        return;
    }

    // Special case for exec, which needs the full register frame to modify
    // the instruction pointer and stack pointer.
    if (regs->eax == SYS_EXEC) {
        // The handler for exec will modify the regs structure directly.
        // It returns 0 on success (but doesn't actually return to the caller),
        // or a negative error code on failure.
        regs->eax = ((int (*)(struct registers_t*))syscalls[SYS_EXEC])(regs);
        return;
    }

    // For all other syscalls, we use the standard 3-argument convention.
    int (*handler)(int, int, int) = (int (*)(int, int, int))syscalls[regs->eax];

    // Call the handler with arguments from registers
    regs->eax = handler(regs->ebx, regs->ecx, regs->edx);
}

// Initializes the system call interface
void syscall_init(void) {
    register_interrupt_handler(0x80, (isr_t)syscall_dispatcher);
}

// --- SYSCALL IMPLEMENTATIONS ---

static int sys_exit(int code) {
    // 1. Store the exit code
    current_process->exit_code = code;

    // 2. Mark the process as a zombie
    current_process->state = PROCESS_STATE_ZOMBIE;

    // 3. Wake up the parent if it's waiting
    int parent_pid = current_process->parent_pid;
    if (parent_pid > 0) {
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            if (process_table[i].pid == parent_pid && process_table[i].state == PROCESS_STATE_BLOCKED) {
                process_table[i].state = PROCESS_STATE_READY;
                break;
            }
        }
    }

    // 4. Yield the CPU. This process will never run again.
    schedule();

    // Should be unreachable
    return 0;
}

static int sys_waitpid(int pid, int* status, int options) {
    // TODO: Implement WNOHANG and other options
    // TODO: Implement process groups for pid == 0 and pid < -1

    if (status && !is_valid_userspace_ptr(status, sizeof(int))) {
        return -EFAULT; // Bad address for status
    }

    bool found_child = false;
    while (true) {
        found_child = false;
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            process_t* p = &process_table[i];
            if (p->pid > 0 && p->parent_pid == current_process->pid) {
                // This is a child process of the current process
                found_child = true;

                if (pid == -1 || p->pid == pid) { // Wait for any child or specific child
                    if (p->state == PROCESS_STATE_ZOMBIE) {
                        // Found a zombie child, collect it
                        int child_pid = p->pid;
                        if (status) {
                            *status = p->exit_code;
                        }

                        // Clean up the process entry
                        process_cleanup(p);

                        return child_pid;
                    }
                }
            }
        }

        if (!found_child) {
            return -ECHILD; // No children to wait for
        }

        // If WNOHANG is set and no zombie child was found, return immediately
        // For now, we don't have WNOHANG, so we always block if no zombie found.
        // TODO: Add WNOHANG support
        
        // If no zombie child found, block the current process
        current_process->state = PROCESS_STATE_BLOCKED;
        schedule(); // Yield CPU
    }
}

static int sys_pipe(int* fds) {
    // TODO: Validate user pointer for fds
    pipe_t* p = pipe_create();
    if (!p) {
        return -ENOMEM;
    }

    int read_fd = -1;
    int write_fd = -1;

    // Find two free file descriptors
    for (int i = 0; i < MAX_PROCESS_FDS; ++i) {
        if (current_process->fds[i] == NULL) {
            if (read_fd == -1) {
                read_fd = i;
            } else {
                write_fd = i;
                break;
            }
        }
    }

    if (read_fd == -1 || write_fd == -1) {
        pipe_destroy(p);
        return -EMFILE; // Too many open files
    }

    // Create VFS nodes for the pipe ends
    vfs_node_t* read_node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    vfs_node_t* write_node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));

    if (!read_node || !write_node) {
        if (read_node) kfree(read_node);
        if (write_node) kfree(write_node);
        pipe_destroy(p);
        return -ENOMEM;
    }

    memset(read_node, 0, sizeof(vfs_node_t));
    memset(write_node, 0, sizeof(vfs_node_t));

    read_node->flags = VFS_FLAG_PIPE | VFS_FLAG_READABLE;
    read_node->pipe = p;
    read_node->read = (vfs_read_t)pipe_read;

    write_node->flags = VFS_FLAG_PIPE | VFS_FLAG_WRITABLE;
    write_node->pipe = p;
    write_node->write = (vfs_write_t)pipe_write;

    current_process->fds[read_fd] = read_node;
    current_process->fds[write_fd] = write_node;

    // Return file descriptors to userland
    fds[0] = read_fd;
    fds[1] = write_fd;

    return 0;
}

static int sys_dup2(int oldfd, int newfd) {
    if (oldfd < 0 || oldfd >= MAX_PROCESS_FDS || !current_process->fds[oldfd]) {
        return -EBADF; // Bad file descriptor
    }

    if (newfd < 0 || newfd >= MAX_PROCESS_FDS) {
        return -EINVAL; // Invalid argument
    }

    // If newfd is already open, close it
    if (current_process->fds[newfd] != NULL) {
        sys_close(newfd);
    }

    current_process->fds[newfd] = current_process->fds[oldfd];
    return newfd;
}

static int sys_kill(pid_t pid, int sig) {
    // For now, we only support killing a process (sig 9, SIGKILL)
    if (sig != 9) {
        return -EINVAL; // Invalid signal
    }

    process_t* target_process = get_process(pid);
    if (!target_process) {
        return -ESRCH; // No such process
    }

    // Don't allow killing kernel process (PID 0) or current process for now
    if (pid == 0 || target_process == current_process) {
        return -EPERM; // Operation not permitted
    }

    // Mark the process as a zombie
    target_process->exit_code = -9; // Indicate killed by signal 9
    target_process->state = PROCESS_STATE_ZOMBIE;

    // Wake up the parent if it's waiting
    int parent_pid = target_process->parent_pid;
    if (parent_pid > 0) {
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            if (process_table[i].pid == parent_pid && process_table[i].state == PROCESS_STATE_BLOCKED) {
                process_table[i].state = PROCESS_STATE_READY;
                break;
            }
        }
    }

    return 0;
}

static int sys_getpid(void) {
    return current_process->pid;
}

static int sys_open(const char* path, int flags) {
    // Find an empty file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_PROCESS_FDS; ++i) {
        if (current_process->fds[i] == NULL) {
            fd = i;
            break;
        }
    }

    if (fd == -1) {
        return -EMFILE; // Error: No available file descriptors
    }

    // Find the file in the VFS
    vfs_node_t* node = vfs_find((char*)path);
    if (!node) {
        return -ENOENT; // Error: File not found
    }

    // Assign the node to the file descriptor
    current_process->fds[fd] = node;

    // Call the VFS open operation
    vfs_open(node, (flags & O_RDONLY) || (flags & O_RDWR), (flags & O_WRONLY) || (flags & O_RDWR));

    return fd;
}

static int sys_close(int fd) {
    if (fd < 0 || fd >= MAX_PROCESS_FDS || !current_process->fds[fd]) {
        return -EBADF; // Error: Invalid file descriptor
    }

    vfs_node_t* node = current_process->fds[fd];
    if (node && node->close) {
        vfs_close(node);
    }
    current_process->fds[fd] = NULL;
    return 0;
}

static int sys_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_PROCESS_FDS || !current_process->fds[fd]) {
        return -EBADF; // Error: Invalid file descriptor
    }

    if (!is_valid_userspace_ptr((void*)buf, count)) {
        return -EFAULT; // Bad address
    }

    vfs_node_t* node = current_process->fds[fd];
    return vfs_write(node, 0, count, (uint8_t*)buf);
}

static int sys_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_PROCESS_FDS || !current_process->fds[fd]) {
        return -EBADF; // Error: Invalid file descriptor
    }

    if (!is_valid_userspace_ptr(buf, count)) {
        return -EFAULT; // Bad address
    }

    vfs_node_t* node = current_process->fds[fd];
    return vfs_read(node, 0, count, (uint8_t*)buf);
}

static int sys_fork(void) {
    // 1. Clone the address space
    page_directory_t* new_dir = paging_clone_directory(current_process->page_directory);
    if (!new_dir) {
        return -1; // Error: Failed to clone address space
    }

    // 2. Create a new process
    process_t* child = process_create(NULL); // We pass NULL because we'll set the context manually
    if (!child) {
        // TODO: Free the cloned page directory
        return -1; // Error: Failed to create process
    }

    // 3. Copy parent's state to child
    child->page_directory = new_dir;
    memcpy(&child->regs, (void*)&current_process->regs, sizeof(struct registers_t));
    memcpy(child->fds, current_process->fds, sizeof(vfs_node_t*) * MAX_PROCESS_FDS);

    // 4. Differentiate child from parent
    child->regs.eax = 0; // The child process gets 0 as the return value from fork

    // 5. The parent process gets the child's PID
    return child->pid;
}

static int sys_exec(struct registers_t* regs) {
    const char* path = (const char*)regs->ebx;
    char* const* argv = (char* const*)regs->ecx;
    // TODO: Validate user pointers for path and argv

    // 1. Create a new address space. We do this first so that if loading fails,
    //    we haven't destroyed the original process's memory.
    page_directory_t* new_page_dir = paging_create_address_space();
    if (!new_page_dir) {
        return -1; // Error: Out of memory
    }

    // 2. Load the executable into the new address space.
    uint32_t entry_point = exec_load_into_address_space(path, new_page_dir);
    if (entry_point == 0) {
        // TODO: Free the new_page_dir and its frames
        return -1; // Error: Failed to load executable
    }

    // 3. Allocate a new stack in the new address space.
    for (uint32_t addr = USER_STACK_TOP - USER_STACK_SIZE; addr < USER_STACK_TOP; addr += PAGE_SIZE) {
        void* p_addr = pmm_alloc_frame();
        if (!p_addr) {
            // TODO: Clean up everything allocated so far
            return -1; // Error: Out of memory for stack
        }
        paging_map_page(new_page_dir, (void*)addr, p_addr, true, true);
    }

    // 4. At this point, the new process image is ready. We can commit the changes.
    page_directory_t* old_page_dir = current_process->page_directory;
    current_process->page_directory = new_page_dir;
    paging_switch_directory(new_page_dir);

    // Free the old address space. This is the point of no return.
    paging_free_directory(old_page_dir);

    // 5. Prepare the trap frame to "return" to the new program's entry point.
    //    This system call will not return to the caller.
    current_process->registers.eip = entry_point;
    current_process->regs.user_esp = USER_STACK_TOP;

    // 5. Set up the new stack with argc and argv
    uint32_t user_stack_ptr = USER_STACK_TOP;
    int argc = 0;
    char** user_argv = NULL;

    if (argv) {
        // Count the arguments
        while (argv[argc] != NULL) {
            argc++;
        }

        // Allocate space on the new stack for the argument strings
        for (int i = argc - 1; i >= 0; i--) {
            int len = strlen(argv[i]) + 1;
            user_stack_ptr -= len;
            // TODO: Need a way to copy from another process's address space!
            // For now, we assume kernel has access, which is true.
            memcpy((void*)user_stack_ptr, argv[i], len);
        }

        // Align stack pointer
        user_stack_ptr &= ~0x3; // Align to 4 bytes

        // Allocate space for the argv pointer array
        user_stack_ptr -= sizeof(char*) * (argc + 1);
        user_argv = (char**)user_stack_ptr;

        // Fill the argv pointer array
        uint32_t string_ptr = USER_STACK_TOP;
        for (int i = argc - 1; i >= 0; i--) {
            int len = strlen(argv[i]) + 1;
            string_ptr -= len;
            user_argv[i] = (char*)string_ptr;
        }
        user_argv[argc] = NULL;

        // Push argc and argv onto the stack for _start
        user_stack_ptr -= sizeof(char**);
        *((char***)user_stack_ptr) = user_argv;

        user_stack_ptr -= sizeof(int);
        *((int*)user_stack_ptr) = argc;
    }

    // 6. Prepare the interrupt frame to "return" to the new program's entry point.
    //    This is the key to exec: we modify the saved register state so that when
    //    the syscall handler returns, it returns to the new program, not the old one.
    regs->eip = entry_point;
    regs->useresp = user_stack_ptr;

    // We return 0, but this value will be in the eax register when the new program starts.
    return 0;
}

static int sys_wait(int* status) {
    // This is a simplified wait(). It waits for any child.
    // A more complete implementation would have waitpid().

    // TODO: Validate user pointer for status before writing to it.

    while (1) {
        bool has_children = false;
        // Scan the process table for a zombie child
        for (int i = 0; i < MAX_PROCESSES; ++i) {
            if (process_table[i].parent_pid == current_process->pid && process_table[i].pid != -1) {
                has_children = true;
                if (process_table[i].state == PROCESS_STATE_ZOMBIE) {
                    // Found a zombie child, collect it!
                    int child_pid = process_table[i].pid;
                    if (status) {
                        *status = process_table[i].exit_code;
                    }

                    // Clean up the process entry, making it available again.
                    process_table[i].state = PROCESS_STATE_TERMINATED;
                    process_table[i].pid = -1; // Invalidate PID
                    process_table[i].parent_pid = -1;

                    return child_pid;
                }
            }
        }

        // If we scanned the whole table and found no children at all.
        if (!has_children) {
            return -ECHILD; // Error: No children to wait for (ECHILD).
        }

        // If we have children but none are zombies, block and wait.
        current_process->state = PROCESS_STATE_BLOCKED;
        schedule(); // Yield the CPU.
    }
}

static int sys_gettimeofday(struct timeval* tv, struct timezone* tz) {
    // For now, return a dummy time. Real implementation needs a RTC driver.
    if (tv) {
        if (!is_valid_userspace_ptr(tv, sizeof(struct timeval))) {
            return -EFAULT;
        }
        tv->tv_sec = 0; // Placeholder
        tv->tv_usec = 0; // Placeholder
    }
    if (tz) {
        if (!is_valid_userspace_ptr(tz, sizeof(struct timezone))) {
            return -EFAULT;
        }
        tz->tz_minuteswest = 0; // Placeholder
        tz->tz_dsttime = 0; // Placeholder
    }
    return 0;
}
