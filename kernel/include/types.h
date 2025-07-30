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

#endif