// RaeenOS Kernel-Level String Utilities Implementation
// --------------------------------------------------

#include "string.h"

// Calculates the length of a string.
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

// Compares two strings.
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Copies a string.
char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

// Returns the length of the initial segment of s which consists entirely of bytes in accept.
size_t strspn(const char* s, const char* accept) {
    const char* p;
    const char* a;
    size_t count = 0;
    for (p = s; *p != '\0'; ++p) {
        for (a = accept; *a != '\0'; ++a) {
            if (*p == *a) {
                break;
            }
        }
        if (*a == '\0') {
            return count;
        }
        ++count;
    }
    return count;
}

// Returns the length of the initial segment of s which consists entirely of bytes not in reject.
size_t strcspn(const char* s, const char* reject) {
    const char* p;
    const char* r;
    size_t count = 0;
    for (p = s; *p != '\0'; ++p) {
        for (r = reject; *r != '\0'; ++r) {
            if (*p == *r) {
                return count;
            }
        }
        ++count;
    }
    return count;
}

// A re-entrant version of strtok.
char* strtok_r(char* str, const char* delim, char** saveptr) {
    char* p;
    if (str == NULL) {
        str = *saveptr;
    }
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    str += strspn(str, delim);
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    p = str + strcspn(str, delim);
    if (*p != '\0') {
        *p++ = '\0';
    }
    *saveptr = p;
    return str;
}

// Copies n bytes from src to dest.
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    return s;
}
