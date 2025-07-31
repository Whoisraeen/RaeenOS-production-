#ifndef TYPES_H
#define TYPES_H

// Basic type definitions for RaeenOS kernel

// Unsigned integer types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// Signed integer types
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

// Standard C types
typedef uint32_t size_t;
typedef int32_t ssize_t;
typedef int32_t ptrdiff_t;
typedef uint32_t uintptr_t;  // Pointer-sized integer for 32-bit systems

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

#endif