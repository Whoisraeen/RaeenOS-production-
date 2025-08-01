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

// Compares up to n characters of two strings.
int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Copies a string.
char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

// Concatenates two strings.
char* strcat(char* dest, const char* src) {
    char* ret = dest;
    while (*dest) {
        dest++;
    }
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

// Copies up to n characters from src to dest.
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    // Pad with null bytes if necessary
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// Compares n bytes of memory.
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i]) {
            return -1;
        } else if (p1[i] > p2[i]) {
            return 1;
        }
    }
    return 0;
}
