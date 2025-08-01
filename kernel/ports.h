// RaeenOS Hardware Port I/O

#ifndef PORTS_H
#define PORTS_H

#include "include/types.h"

// Read a byte from a port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a byte to a port
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

#endif // PORTS_H
