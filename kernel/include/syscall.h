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

// System call handlers are internal to syscall.c
// External interface is through syscall interrupt only

#endif // SYSCALL_H
