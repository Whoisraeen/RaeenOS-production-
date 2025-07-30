# RaeenOS Coding Standards

**Version:** 1.0  
**Last Updated:** 2025-07-30  
**Applies To:** All RaeenOS development agents and contributors

## Table of Contents

1. [General Principles](#general-principles)
2. [C/C++ Coding Standards](#cc-coding-standards)
3. [Assembly Language Guidelines](#assembly-language-guidelines)
4. [Python Scripting Standards](#python-scripting-standards)
5. [Naming Conventions](#naming-conventions)
6. [File Organization](#file-organization)
7. [Documentation Requirements](#documentation-requirements)
8. [Error Handling](#error-handling)
9. [Memory Management](#memory-management)
10. [Threading and Concurrency](#threading-and-concurrency)
11. [Security Guidelines](#security-guidelines)
12. [Performance Considerations](#performance-considerations)

## General Principles

### Code Quality Tenets
- **Clarity over cleverness**: Write code that is easy to understand and maintain
- **Consistency**: Follow established patterns throughout the codebase
- **Robustness**: Handle errors gracefully and defensively
- **Performance**: Write efficient code, but not at the expense of maintainability
- **Security**: Always consider security implications of code changes

### Code Review Requirements
- All code must be reviewed by at least one other agent or contributor
- Critical kernel components require review by a kernel-architect or lead-os-developer
- Security-sensitive code requires review by a privacy-security-engineer

## C/C++ Coding Standards

### Code Style

#### Indentation and Formatting
```c
// Use 4 spaces for indentation (no tabs)
if (condition) {
    do_something();
    if (nested_condition) {
        do_nested_action();
    }
}
```

#### Braces
```c
// Opening brace on same line for functions and control structures
void function_name(void) {
    if (condition) {
        // code here
    } else {
        // alternative code
    }
}
```

#### Line Length
- Maximum line length: 100 characters
- Break long lines at logical points
- Align continuation lines appropriately

```c
// Good
int result = some_very_long_function_name(parameter1, parameter2,
                                          parameter3, parameter4);

// Bad
int result = some_very_long_function_name(parameter1, parameter2, parameter3, parameter4);
```

#### Spacing
```c
// Spaces around operators
int result = a + b * c;

// Space after keywords, no space between function name and parentheses
if (condition) {
    function_call(argument);
}

// Spaces after commas
function_call(arg1, arg2, arg3);
```

### Variable Declarations
```c
// Declare variables at the beginning of their scope
void function_name(void) {
    int local_var = 0;
    char* buffer = NULL;
    size_t buffer_size = 0;
    
    // Function body
}

// Initialize variables when declared
int count = 0;
char* name = NULL;
```

### Function Definitions
```c
/**
 * Brief description of what the function does
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * 
 * @note Any important notes about usage
 * @warning Any warnings about potential issues
 */
return_type function_name(parameter_type param1, parameter_type param2) {
    // Function implementation
}
```

### Header Guards
```c
#ifndef MODULE_NAME_H
#define MODULE_NAME_H

// Header content

#endif // MODULE_NAME_H
```

### Include Order
```c
// System includes first
#include <stdint.h>
#include <stddef.h>

// Kernel includes second
#include "kernel/memory.h"
#include "kernel/process.h"

// Local includes last
#include "local_header.h"
```

## Assembly Language Guidelines

### Syntax and Style
```asm
; Use Intel syntax (NASM)
; Comments start with semicolon, align for readability

section .text
global _start

_start:
    mov     eax, 1          ; System call number for exit
    mov     ebx, 0          ; Exit status
    int     0x80            ; Call kernel
```

### Register Usage
```asm
; Document register usage at function entry
; Input:  EAX = parameter 1, EBX = parameter 2
; Output: EAX = result
; Clobbers: ECX, EDX

function_name:
    push    ebp             ; Save base pointer
    mov     ebp, esp        ; Set up stack frame
    
    ; Function body
    
    mov     esp, ebp        ; Restore stack
    pop     ebp             ; Restore base pointer
    ret                     ; Return to caller
```

### Labels and Comments
```asm
; Use descriptive labels
memory_copy_loop:
    mov     al, [esi]       ; Load byte from source
    mov     [edi], al       ; Store byte to destination
    inc     esi             ; Advance source pointer
    inc     edi             ; Advance destination pointer
    loop    memory_copy_loop ; Repeat until ECX = 0
```

## Python Scripting Standards

### Code Style (PEP 8 Compliance)
```python
"""Module docstring describing the module's purpose."""

import os
import sys
from typing import List, Optional, Dict

# Constants in UPPER_CASE
MAX_RETRY_COUNT = 3
DEFAULT_TIMEOUT = 30


class DevToolHelper:
    """Class for development tool operations."""
    
    def __init__(self, config_path: str) -> None:
        """Initialize the helper with configuration."""
        self.config_path = config_path
        self.is_initialized = False
    
    def process_files(self, file_list: List[str]) -> Dict[str, bool]:
        """Process a list of files and return results."""
        results = {}
        
        for file_path in file_list:
            try:
                result = self._process_single_file(file_path)
                results[file_path] = result
            except Exception as e:
                logger.error(f"Failed to process {file_path}: {e}")
                results[file_path] = False
        
        return results
```

### Error Handling
```python
# Use specific exception types
try:
    with open(file_path, 'r') as f:
        content = f.read()
except FileNotFoundError:
    logger.error(f"File not found: {file_path}")
    return None
except PermissionError:
    logger.error(f"Permission denied: {file_path}")
    return None
except IOError as e:
    logger.error(f"IO error reading {file_path}: {e}")
    return None
```

## Naming Conventions

### C/C++ Naming
```c
// Types: PascalCase with _t suffix
typedef struct {
    int field;
} my_struct_t;

// Functions: snake_case
void init_system(void);
int calculate_checksum(const char* data, size_t length);

// Variables: snake_case
int global_counter = 0;
static char* internal_buffer = NULL;

// Constants: UPPER_SNAKE_CASE
#define MAX_BUFFER_SIZE 1024
#define KERNEL_VERSION "1.0.0"

// Macros: UPPER_SNAKE_CASE
#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))
```

### File Naming
```
// Header files: lowercase with underscores
memory_manager.h
process_scheduler.h

// Source files: match header names
memory_manager.c
process_scheduler.c

// Directory names: lowercase with hyphens for multi-word
kernel/
drivers/
user-interface/
```

### Assembly Naming
```asm
; Labels: snake_case
boot_entry:
interrupt_handler:

; Constants: UPPER_SNAKE_CASE
KERNEL_BASE_ADDR    equ 0x100000
STACK_SIZE          equ 4096
```

## File Organization

### Directory Structure
```
raeenos/
├── kernel/           # Core kernel components
│   ├── include/      # Kernel headers
│   ├── arch/         # Architecture-specific code
│   ├── mm/           # Memory management
│   ├── fs/           # Filesystem
│   └── drivers/      # Kernel drivers
├── drivers/          # Device drivers
├── userland/         # User-space applications
├── tools/            # Development tools
├── tests/            # Test suites
├── docs/             # Documentation
└── scripts/          # Build and utility scripts
```

### Header File Organization
```c
// File: kernel/include/memory.h

#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

// System includes
#include <stdint.h>
#include <stddef.h>

// Forward declarations
struct page;
struct vm_area;

// Constants
#define PAGE_SIZE       4096
#define PAGE_SHIFT      12

// Type definitions
typedef struct page {
    uint32_t flags;
    uint32_t ref_count;
} page_t;

// Function declarations
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif // KERNEL_MEMORY_H
```

## Documentation Requirements

### Function Documentation
```c
/**
 * @brief Allocates memory from the kernel heap
 * 
 * This function allocates a block of memory from the kernel heap.
 * The memory is guaranteed to be physically contiguous and is
 * suitable for DMA operations.
 * 
 * @param size Number of bytes to allocate (must be > 0)
 * @param flags Allocation flags (GFP_KERNEL, GFP_ATOMIC, etc.)
 * 
 * @return Pointer to allocated memory on success, NULL on failure
 * 
 * @note The returned memory is uninitialized
 * @warning This function may sleep if GFP_KERNEL is used
 * 
 * @see kfree() for memory deallocation
 * @since RaeenOS 1.0
 */
void* kmalloc_flags(size_t size, uint32_t flags);
```

### File Headers
```c
/**
 * @file memory.c
 * @brief Kernel memory management implementation
 * 
 * This file implements the core memory management functionality for
 * RaeenOS, including physical memory allocation, virtual memory
 * mapping, and kernel heap management.
 * 
 * @author RaeenOS Development Team
 * @date 2025-07-30
 * @version 1.0
 * 
 * @copyright Copyright (c) 2025 RaeenOS Project
 * @license MIT License
 */
```

### Module Documentation
```c
/**
 * @defgroup memory Memory Management
 * @brief Core memory management subsystem
 * 
 * The memory management subsystem provides:
 * - Physical memory allocation (PMM)
 * - Virtual memory management (VMM)  
 * - Kernel heap management
 * - Memory mapping and protection
 * 
 * @{
 */

// Module implementation

/** @} */ // End of memory group
```

## Error Handling

### Error Codes
```c
// Use consistent error code system
typedef enum {
    RAEEN_OK = 0,           // Success
    RAEEN_ERROR = -1,       // Generic error
    RAEEN_ENOMEM = -2,      // Out of memory
    RAEEN_EINVAL = -3,      // Invalid argument
    RAEEN_ENOENT = -4,      // No such entry
    RAEEN_EPERM = -5,       // Permission denied
    RAEEN_EIO = -6,         // I/O error
} raeen_error_t;
```

### Error Handling Patterns
```c
// Function that can fail
raeen_error_t initialize_subsystem(const config_t* config) {
    if (!config) {
        return RAEEN_EINVAL;
    }
    
    void* buffer = kmalloc(BUFFER_SIZE);
    if (!buffer) {
        return RAEEN_ENOMEM;
    }
    
    if (setup_hardware() != 0) {
        kfree(buffer);
        return RAEEN_EIO;
    }
    
    return RAEEN_OK;
}

// Calling the function
raeen_error_t result = initialize_subsystem(&config);
if (result != RAEEN_OK) {
    log_error("Failed to initialize subsystem: %d", result);
    return result;
}
```

### Assertions and Debugging
```c
// Debug assertions (removed in release builds)
#ifdef DEBUG
    #define ASSERT(condition) \
        do { \
            if (!(condition)) { \
                panic("Assertion failed: %s at %s:%d", \
                      #condition, __FILE__, __LINE__); \
            } \
        } while (0)
#else
    #define ASSERT(condition) do { } while (0)
#endif

// Runtime checks (always present)
#define VERIFY(condition) \
    do { \
        if (!(condition)) { \
            panic("Verification failed: %s at %s:%d", \
                  #condition, __FILE__, __LINE__); \
        } \
    } while (0)
```

## Memory Management

### Memory Allocation
```c
// Always check allocation success
void* buffer = kmalloc(size);
if (!buffer) {
    log_error("Memory allocation failed");
    return RAEEN_ENOMEM;
}

// Initialize allocated memory
memset(buffer, 0, size);

// Use buffer...

// Always free allocated memory
kfree(buffer);
buffer = NULL; // Prevent double-free
```

### Resource Management (RAII Pattern)
```c
// Cleanup function
void cleanup_resources(resource_t* res) {
    if (res) {
        if (res->buffer) {
            kfree(res->buffer);
            res->buffer = NULL;
        }
        if (res->file_handle) {
            close_file(res->file_handle);
            res->file_handle = NULL;
        }
        kfree(res);
    }
}

// Function with automatic cleanup
raeen_error_t process_data(const char* filename) {
    resource_t* res = kmalloc(sizeof(resource_t));
    if (!res) {
        return RAEEN_ENOMEM;
    }
    
    memset(res, 0, sizeof(resource_t));
    
    raeen_error_t result = RAEEN_OK;
    
    // Allocate buffer
    res->buffer = kmalloc(BUFFER_SIZE);
    if (!res->buffer) {
        result = RAEEN_ENOMEM;
        goto cleanup;
    }
    
    // Open file
    res->file_handle = open_file(filename);
    if (!res->file_handle) {
        result = RAEEN_EIO;
        goto cleanup;
    }
    
    // Process data...
    
cleanup:
    cleanup_resources(res);
    return result;
}
```

## Threading and Concurrency

### Locking Patterns
```c
// Spinlock usage
static spinlock_t global_lock = SPINLOCK_INIT;

void thread_safe_function(void) {
    unsigned long flags;
    
    spin_lock_irqsave(&global_lock, flags);
    
    // Critical section
    modify_shared_data();
    
    spin_unlock_irqrestore(&global_lock, flags);
}
```

### Atomic Operations
```c
// Use atomic operations for simple counters
static atomic_t reference_count = ATOMIC_INIT(0);

void acquire_reference(void) {
    atomic_inc(&reference_count);
}

void release_reference(void) {
    if (atomic_dec_and_test(&reference_count)) {
        // Last reference released
        cleanup_resource();
    }
}
```

### Thread Safety Documentation
```c
/**
 * @brief Thread-safe counter increment
 * 
 * This function atomically increments a counter and is safe to call
 * from multiple threads simultaneously.
 * 
 * @param counter Pointer to counter to increment
 * 
 * @thread_safety This function is thread-safe
 * @lock_order None (uses atomic operations)
 */
void atomic_increment(atomic_t* counter);
```

## Security Guidelines

### Input Validation
```c
// Always validate input parameters
raeen_error_t copy_user_data(void* dest, const void* src, size_t size) {
    // Validate pointers
    if (!dest || !src) {
        return RAEEN_EINVAL;
    }
    
    // Validate size
    if (size == 0 || size > MAX_COPY_SIZE) {
        return RAEEN_EINVAL;
    }
    
    // Validate user space pointers
    if (!is_user_address(src) || !is_user_address((char*)src + size - 1)) {
        return RAEEN_EPERM;
    }
    
    // Perform copy with error checking
    if (copy_from_user(dest, src, size) != 0) {
        return RAEEN_EIO;
    }
    
    return RAEEN_OK;
}
```

### Buffer Overflow Prevention
```c
// Use safe string functions
void safe_string_copy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return;
    }
    
    // Ensure null termination
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// Check array bounds
bool safe_array_access(int* array, size_t array_size, size_t index) {
    if (!array || index >= array_size) {
        return false;
    }
    
    return true;
}
```

### Cryptographic Standards
```c
// Use secure random number generation
void generate_secure_key(uint8_t* key, size_t key_size) {
    ASSERT(key != NULL);
    ASSERT(key_size > 0);
    
    // Use hardware random number generator when available
    if (has_hardware_rng()) {
        hardware_random_bytes(key, key_size);
    } else {
        // Fallback to cryptographically secure PRNG
        secure_random_bytes(key, key_size);
    }
}
```

## Performance Considerations

### Hot Path Optimization
```c
// Mark hot paths for compiler optimization
__attribute__((hot))
static inline uint32_t fast_hash(const void* data, size_t len) {
    // Optimized hash function for frequent use
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t hash = 5381;
    
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + bytes[i];
    }
    
    return hash;
}

// Mark cold paths to optimize for size
__attribute__((cold))
void handle_rare_error(error_code_t error) {
    // Error handling code that rarely executes
    log_detailed_error(error);
    dump_system_state();
}
```

### Cache-Friendly Data Structures
```c
// Align structures to cache line boundaries
typedef struct cache_aligned_data {
    uint32_t frequently_accessed;
    uint32_t also_frequently_accessed;
    // Pad to cache line size (64 bytes on x86-64)
    uint8_t padding[56];
} __attribute__((aligned(64))) cache_aligned_data_t;
```

### Memory Access Patterns
```c
// Prefer sequential access patterns
void process_array_sequential(int* array, size_t size) {
    // Good: sequential access
    for (size_t i = 0; i < size; i++) {
        array[i] = process_element(array[i]);
    }
}

// Avoid when possible: random access patterns
void process_array_random(int* array, size_t size, size_t* indices) {
    // Less efficient: random access
    for (size_t i = 0; i < size; i++) {
        array[indices[i]] = process_element(array[indices[i]]);
    }
}
```

---

**Next Steps:**
- Review this document with the development team
- Set up automated code formatting and linting
- Create code review checklists based on these standards
- Establish training materials for new contributors

**Document Control:**
- This document is maintained by the RaeenOS development team
- Changes require approval from lead-os-developer or kernel-architect
- Version history is tracked in the project repository