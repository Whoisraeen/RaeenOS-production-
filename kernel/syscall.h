// RaeenOS System Call Interface

#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt.h"

// Defines the syscall numbers for RaeenOS
enum syscall_num {
    SYS_EXIT,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_READ,
    SYS_FORK,
    SYS_EXEC,
    SYS_GETPID,
    SYS_WAITPID,
    SYS_PIPE,
    SYS_DUP2,
    SYS_KILL,
    SYS_GETTIMEOFDAY,
    // Add more syscalls here
};

// The main syscall handler, called from the assembly stub
void syscall_handler(struct registers_t regs);

// Initializes the system call handler
void syscall_init(void);

#endif // SYSCALL_H
