/**
 * @file kernel_helpers.c
 * @brief Helper functions for the interactive kernel
 * 
 * @version 1.0
 * @date 2025-08-02
 */

#include "include/types.h"

/**
 * Convert uint32 to string
 */
void uint32_to_string(uint32_t value, char* buffer, size_t buffer_size) {
    if (buffer_size == 0) return;
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    char temp[12]; // Enough for 32-bit number
    int pos = 0;
    
    while (value > 0) {
        temp[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Reverse the string
    int i;
    for (i = 0; i < pos && i < (int)buffer_size - 1; i++) {
        buffer[i] = temp[pos - 1 - i];
    }
    buffer[i] = '\0';
}

/**
 * Port I/O functions
 */
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}