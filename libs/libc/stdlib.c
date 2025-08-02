#include "include/stdlib.h"
#include "../kernel/memory.h"
#include "../kernel/process/process.h"
#include "../kernel/vga.h"
#include "../libs/libc/include/string.h"

// Basic malloc implementation (maps to kmalloc)
void* malloc(size_t size) {
    return kmalloc(size);
}

// Basic calloc implementation
void* calloc(size_t num, size_t size) {
    void* ptr = kmalloc(num * size);
    if (ptr) {
        memset(ptr, 0, num * size);
    }
    return ptr;
}

// Basic realloc implementation
void* realloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) { kfree(ptr); return NULL; }

    void* new_ptr = kmalloc(size);
    if (new_ptr) {
        // TODO: Copy old data, need to know old size
        // For now, just return new block
        kfree(ptr);
    }
    return new_ptr;
}

// Basic free implementation (maps to kfree)
void free(void* ptr) {
    kfree(ptr);
}

// Basic exit implementation
void exit(int status) {
    debug_print("libc: exit() called with status ");
    vga_put_dec(status);
    debug_print("\n");
    process_exit(status);
}

// Basic abort implementation
void abort(void) {
    debug_print("libc: abort() called!\n");
    // TODO: Implement proper process termination/crash handling
    while(1); // Hang for now
}

// Basic atoi implementation
int atoi(const char* str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    for (; str[i] != '\0'; ++i) {
        res = res * 10 + str[i] - '0';
    }

    return sign * res;
}

// Basic atol implementation
long atol(const char* str) {
    long res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    for (; str[i] != '\0'; ++i) {
        res = res * 10 + (str[i] - '0');
    }

    return sign * res;
}

// Basic atoll implementation
long long atoll(const char* str) {
    long long res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    for (; str[i] != '\0'; ++i) {
        res = res * 10 + (str[i] - '0');
    }

    return sign * res;
}

// Basic atof implementation (very simplified)
double atof(const char* str) {
    // For now, just convert integer part
    return (double)atoi(str);
}

static unsigned long next_rand = 1; // Seed for rand

int rand(void) {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % 32768;
}

void srand(unsigned int seed) {
    next_rand = seed;
}

// Basic abs implementations
int abs(int n) {
    return (n < 0) ? -n : n;
}

long labs(long n) {
    return (n < 0) ? -n : n;
}

long long llabs(long long n) {
    return (n < 0) ? -n : n;
}

// Basic div implementations
div_t div(int numer, int denom) {
    div_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

ldiv_t ldiv(long numer, long denom) {
    ldiv_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

lldiv_t lldiv(long long numer, long long denom) {
    lldiv_t result;
    result.quot = numer / denom;
    result.rem = numer / denom;
    return result;
}

// Placeholder for bsearch
void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*)) {
    debug_print("libc: bsearch (placeholder).\n");
    return NULL;
}

// Placeholder for qsort
void qsort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*)) {
    debug_print("libc: qsort (placeholder).\n");
}

// Placeholder for getenv
char* getenv(const char* name) {
    debug_print("libc: getenv (placeholder).\n");
    return NULL;
}

// Placeholder for putenv
int putenv(char* string) {
    debug_print("libc: putenv (placeholder).\n");
    return -1;
}