#include "include/stdlib.h"
#include "../kernel/memory.h"
#include "../kernel/process/process.h"

// Basic malloc implementation (maps to kmalloc)
void* malloc(size_t size) {
    return kmalloc(size);
}

// Basic free implementation (maps to kfree)
void free(void* ptr) {
    kfree(ptr);
}

// Basic exit implementation
void exit(int status) {
    process_exit(status);
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
