#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stddef.h>

// System call numbers
#define SYS_EXIT  0
#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_READ  4
#define SYS_FORK  5
#define SYS_EXEC   6
#define SYS_WAIT   7

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/**
 * @brief Executes a system call.
 *
 * @param syscall_num The number of the system call.
 * @param arg1        The first argument.
 * @param arg2        The second argument.
 * @param arg3        The third argument.
 * @return int        The return value of the system call.
 */
static inline int syscall(int syscall_num, int arg1, int arg2, int arg3) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret)
        : "a" (syscall_num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    return ret;
}

// --- User-friendly syscall wrappers ---

static inline void exit(int status) {
    syscall(SYS_EXIT, status, 0, 0);
}

static inline int write(int fd, const void* buf, size_t count) {
    return syscall(SYS_WRITE, fd, (int)buf, count);
}

static inline int read(int fd, void* buf, size_t count) {
    return syscall(SYS_READ, fd, (int)buf, count);
}

static inline int open(const char* path, int flags) {
    return syscall(SYS_OPEN, (int)path, flags, 0);
}

static inline int close(int fd) {
    return syscall(SYS_CLOSE, fd, 0, 0);
}

static inline int fork(void) {
    return syscall(SYS_FORK, 0, 0, 0);
}

static inline int exec(const char* path, char* const argv[]) {
    return syscall(SYS_EXEC, (int)path, (int)argv, 0);
}

static inline int wait(int* status) {
    return syscall(SYS_WAIT, (int)status, 0, 0);
}

#endif // _SYSCALL_H
