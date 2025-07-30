#include "include/syscall.h"

int write(int fd, const void* buf, unsigned int count) {
    int result;
    __asm__ volatile ("int $0x80" : "=a" (result) : "a" (SYS_WRITE), "b" (fd), "c" (buf), "d" (count));
    return result;
}

int read(int fd, void* buf, unsigned int count) {
    int result;
    __asm__ volatile ("int $0x80" : "=a" (result) : "a" (SYS_READ), "b" (fd), "c" (buf), "d" (count));
    return result;
}

int open(const char* path, int flags) {
    int result;
    __asm__ volatile ("int $0x80" : "=a" (result) : "a" (SYS_OPEN), "b" (path), "c" (flags));
    return result;
}

int close(int fd) {
    int result;
    __asm__ volatile ("int $0x80" : "=a" (result) : "a" (SYS_CLOSE), "b" (fd));
    return result;
}

int fork(void) {
    int result;
    __asm__ volatile ("int $0x80" : "=a" (result) : "a" (SYS_FORK));
    return result;
}

int exec(const char* path, char* const argv[]) {
    int result;
    __asm__ volatile ("int $0x80" : "=a" (result) : "a" (SYS_EXEC), "b" (path), "c" (argv));
    return result;
}

void exit(int code) {
    // This syscall does not return.
    __asm__ volatile ("int $0x80" : : "a" (SYS_EXIT), "b" (code));
    // Loop forever if exit fails to terminate the process
    for(;;);
}
