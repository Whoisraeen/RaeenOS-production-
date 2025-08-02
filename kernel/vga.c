// VGA text mode implementation for RaeenOS

#include "vga.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;

static void terminal_scroll(void) {
    if (terminal_row >= VGA_HEIGHT) {
        for (size_t y = 1; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t index = (y - 1) * VGA_WIDTH + x;
                const size_t next_index = y * VGA_WIDTH + x;
                vga_buffer[index] = vga_buffer[next_index];
            }
        }
        
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            vga_buffer[index] = ' ' | 0x0700;
        }
        
        terminal_row = VGA_HEIGHT - 1;
        terminal_column = 0;
    }
}

void vga_init(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = ' ' | 0x0700;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = ' ' | 0x0700;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void vga_putc(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        terminal_scroll();
        return;
    }
    
    if (c == '\b') {
        // Backspace
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            vga_buffer[index] = ' ' | 0x0700;
        }
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = c | 0x0700;
    
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        terminal_scroll();
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}

void vga_put_hex(uint32_t n) {
    char buf[9];
    const char* hex_chars = "0123456789ABCDEF";
    buf[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        buf[i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    vga_puts(buf);
}

void vga_put_dec(uint32_t n) {
    if (n == 0) {
        vga_putc('0');
        return;
    }

    char buf[10]; // Max 10 digits for 32-bit unsigned int
    int i = 0;
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }

    while (i-- > 0) {
        vga_putc(buf[i]);
    }
}

void debug_print(const char* str) {
    vga_puts("[DEBUG] ");
    vga_puts(str);
    vga_puts("\n");
}