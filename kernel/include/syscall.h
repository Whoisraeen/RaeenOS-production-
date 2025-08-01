// RaeenOS System Call Interface (Corrected)
// -------------------------------------------
// This file defines the interface for kernel system calls.

#ifndef SYSCALL_H
#define SYSCALL_H

#include "../idt.h" // For registers_t struct
#include "types.h"  // For uint64_t, size_t, bool, etc.

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

// System call statistics structure
struct syscall_stats {
    uint64_t total_calls;
    uint64_t failed_calls;
    uint64_t invalid_calls;
    uint64_t capability_denials;
    uint64_t validation_failures;
    uint64_t per_syscall_counts[256];  // MAX_SYSCALL_NUM + 1
};

// AI query parameters structure
struct sys_ai_query_params {
    const char* query;
    size_t query_length;
    void* response_buffer;
    size_t response_buffer_size;
    uint32_t flags;
};

// Initialize the system call dispatcher
#ifdef SYSCALL_PRODUCTION_MODE
int syscall_init(void);
#else
void syscall_init();
#endif

// Legacy system call handler function declarations
// These are implemented in syscall_impl.c
#ifndef SYSCALL_PRODUCTION_MODE
void sys_exit(int status);
int sys_fork(struct registers_t* regs);
int sys_exec(struct registers_t* regs);
int sys_wait(int* status);
int sys_open(const char* path, int flags, int mode);
int sys_close(int fd);
int sys_read(int fd, void* buffer, size_t count);
int sys_write(int fd, const void* buffer, size_t count);
#endif

// Production system call interface functions
typedef int64_t (*syscall_handler_t)(uint64_t arg1, uint64_t arg2, uint64_t arg3, 
                                      uint64_t arg4, uint64_t arg5, uint64_t arg6);

int syscall_register(uint32_t syscall_num, syscall_handler_t handler, const char* name,
                    uint32_t arg_count, uint32_t flags, uint32_t required_cap, bool audit);
int64_t syscall_dispatch(uint32_t syscall_num, uint64_t arg1, uint64_t arg2, 
                        uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int syscall_get_stats(struct syscall_stats* stats);
void syscall_cleanup(void);

#endif // SYSCALL_H
