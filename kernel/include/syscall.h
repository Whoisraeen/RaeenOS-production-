// RaeenOS System Call Interface (Corrected)
// -------------------------------------------
// This file defines the interface for kernel system calls.

#ifndef SYSCALL_H
#define SYSCALL_H

#include "../idt.h" // For registers_t struct

// System Call Numbers
enum syscall_num {
    SYS_EXIT,
    SYS_FORK,
    SYS_EXEC,
    SYS_WAIT,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_READ,
    SYS_WRITE,
    SYS_GETPID,
    SYS_WAITPID,
    SYS_PIPE,
    SYS_DUP2,
    SYS_KILL,
    SYS_GETTIMEOFDAY,

    NUM_SYSCALLS
};

// Initialize the system call dispatcher
void syscall_init();

// System call handler function declarations
// These are implemented in syscall_impl.c
void sys_exit(int status);
int sys_fork(struct registers_t* regs);
int sys_exec(struct registers_t* regs);
int sys_wait(int* status);
int sys_open(const char* path, int flags, int mode);
int sys_close(int fd);
int sys_read(int fd, void* buffer, size_t count);
int sys_write(int fd, const void* buffer, size_t count);

#endif // SYSCALL_H
