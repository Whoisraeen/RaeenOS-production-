// syscalls.h - System call numbers for RaeenOS

#ifndef SYSCALLS_H
#define SYSCALLS_H

// Process Management
#define SYS_EXIT        0
#define SYS_GETPID      1

// Console I/O
#define SYS_WRITE       2

// Filesystem Operations
#define SYS_OPEN        3
#define SYS_CLOSE       4
#define SYS_READ        5
#define SYS_WRITE_FILE  6 // Renamed from SYS_WRITE to avoid conflict
#define SYS_CREATE      7
#define SYS_READDIR     8

// Input
#define SYS_READ_KEY    9

#endif // SYSCALLS_H
