// VGA text mode interface for RaeenOS

#ifndef VGA_H
#define VGA_H

#include "include/types.h"

void vga_init(void);
void vga_puts(const char* str);
void vga_putc(char c);
void vga_put_hex(uint32_t n);
void vga_put_dec(uint32_t n);
void debug_print(const char* str);

#endif
