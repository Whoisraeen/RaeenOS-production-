#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <stdint.h>

// Exit status constants
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Memory allocation
void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);

// Process control
void exit(int status);
void abort(void);

// String conversion
int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);
double atof(const char* str);

// Pseudo-random numbers
int rand(void);
void srand(unsigned int seed);

// Environment variables (placeholder)
char* getenv(const char* name);
int putenv(char* string);

// Searching and sorting
void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*));
void qsort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*));

// Integer absolute value
int abs(int n);
long labs(long n);
long long llabs(long long n);

// Division
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
typedef struct { long long quot; long long rem; } lldiv_t;
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

#endif // _STDLIB_H
