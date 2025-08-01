#ifndef TYPES_H
#define TYPES_H

// Basic type definitions for RaeenOS kernel

// Only define types if not already defined by standard headers
#ifndef __STDINT_H
#ifndef _STDINT_H
#ifndef __STDINT_H_
#ifndef _STDINT_H_

// Unsigned integer types
#ifndef UINT8_MAX
#ifndef __uint8_t_defined
typedef unsigned char uint8_t;
#define __uint8_t_defined
#endif
#endif
#ifndef UINT16_MAX
#ifndef __uint16_t_defined
typedef unsigned short uint16_t;
#define __uint16_t_defined
#endif
#endif
#ifndef UINT32_MAX
#ifndef __uint32_t_defined
typedef unsigned int uint32_t;
#define __uint32_t_defined
#endif
#endif
#ifndef UINT64_MAX
#ifndef __uint64_t_defined
#ifdef __LP64__
typedef unsigned long uint64_t;
#else
typedef unsigned long long uint64_t;
#endif
#define __uint64_t_defined
#endif
#endif

// Signed integer types  
#ifndef INT8_MAX
#ifndef __int8_t_defined
typedef signed char int8_t;
#define __int8_t_defined
#endif
#endif
#ifndef INT16_MAX
#ifndef __int16_t_defined
typedef signed short int16_t;
#define __int16_t_defined
#endif
#endif
#ifndef INT32_MAX
#ifndef __int32_t_defined
typedef signed int int32_t;
#define __int32_t_defined
#endif
#endif
#ifndef INT64_MAX
#ifndef __int64_t_defined
#ifdef __LP64__
typedef signed long int64_t;
#else
typedef signed long long int64_t;
#endif
#define __int64_t_defined
#endif
#endif

#endif // _STDINT_H_
#endif // __STDINT_H_
#endif // _STDINT_H
#endif // __STDINT_H

// Standard C types - architecture aware
#ifndef _SIZE_T_DEFINED
#ifndef __SIZE_T
#ifndef _SIZE_T
#ifdef __LP64__
typedef uint64_t size_t;
#else
typedef uint32_t size_t;
#endif
#endif
#endif
#endif
#ifndef _SSIZE_T_DEFINED
#ifdef __LP64__
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#endif
#ifndef _PTRDIFF_T
#ifndef __PTRDIFF_T
#ifdef __LP64__
typedef int64_t ptrdiff_t;
#else
typedef int32_t ptrdiff_t;
#endif
#endif
#endif

// Pointer-sized integer - use standard if available
#ifndef _UINTPTR_T_DEFINED
#ifndef __uintptr_t_defined
#ifdef __LP64__
typedef unsigned long uintptr_t;
#else
typedef uint32_t uintptr_t;
#endif
#define __uintptr_t_defined
#endif
#endif

// Boolean type  
#ifndef __cplusplus
#ifndef bool
typedef enum { false = 0, true = 1 } bool;
#endif
#endif

// Null pointer
#ifndef NULL
#define NULL ((void*)0)
#endif

// Standard limits
#ifndef UINT8_MAX
#define UINT8_MAX   255
#endif
#ifndef UINT16_MAX
#define UINT16_MAX  65535
#endif
#ifndef UINT32_MAX
#define UINT32_MAX  4294967295U
#endif
#ifndef UINT64_MAX
#define UINT64_MAX  18446744073709551615ULL
#endif
#ifndef INT8_MAX
#define INT8_MAX    127
#endif
#ifndef INT16_MAX
#define INT16_MAX   32767
#endif
#ifndef INT32_MAX
#define INT32_MAX   2147483647
#endif
#ifndef INT64_MAX
#define INT64_MAX   9223372036854775807LL
#endif

// Memory alignment
#define ALIGN(x) __attribute__((aligned(x)))
#define PACKED __attribute__((packed))

// Physical address type
typedef uint64_t phys_addr_t;

// File offset type
typedef int64_t off_t;

// POSIX-style types for filesystem operations
typedef uint32_t dev_t;    // Device identifier
typedef uint32_t uid_t;    // User identifier
typedef uint32_t gid_t;    // Group identifier
typedef uint32_t pid_t;    // Process identifier
typedef uint32_t mode_t;   // File mode/permissions

// Page size constants
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

// Number of CPUs (simplified for now)
#define NR_CPUS 8

// Memory map entry structure (from multiboot)
typedef struct mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) mmap_entry_t;

// Filesystem statistics structure
struct statfs {
    uint32_t f_type;     // Filesystem type
    uint32_t f_bsize;    // Block size
    uint64_t f_blocks;   // Total blocks
    uint64_t f_bfree;    // Free blocks
    uint64_t f_bavail;   // Available blocks
    uint64_t f_files;    // Total inodes
    uint64_t f_ffree;    // Free inodes
    uint64_t f_fsid;     // Filesystem ID
    uint32_t f_namelen;  // Maximum filename length
    uint32_t f_frsize;   // Fragment size
    uint32_t f_flags;    // Mount flags
    uint32_t f_spare[4]; // Spare for future use
};

// POSIX file locking structure
struct flock {
    short l_type;       // Type of lock: F_RDLCK, F_WRLCK, F_UNLCK
    short l_whence;     // How to interpret l_start: SEEK_SET, SEEK_CUR, SEEK_END
    off_t l_start;      // Starting offset for lock
    off_t l_len;        // Number of bytes to lock
    pid_t l_pid;        // PID of process blocking our lock (F_GETLK only)
};

// POSIX lock commands (for fcntl)
#define F_GETLK     5   // Get lock information
#define F_SETLK     6   // Set lock (non-blocking)
#define F_SETLKW    7   // Set lock (blocking)

// POSIX lock types
#define F_RDLCK     0   // Read lock
#define F_WRLCK     1   // Write lock
#define F_UNLCK     2   // Unlock

// Seek whence values
#ifndef SEEK_SET
#define SEEK_SET    0   // Seek from beginning of file
#define SEEK_CUR    1   // Seek from current position
#define SEEK_END    2   // Seek from end of file
#endif

#endif