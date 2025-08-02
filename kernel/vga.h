// Enhanced VGA interface for RaeenUI prototype - RaeenOS

#ifndef VGA_H
#define VGA_H

#include "include/types.h"

// VGA Color constants for enhanced UI
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color;

// Enhanced VGA functions for RaeenUI
void vga_init(void);
void vga_clear(void);
void vga_clear_with_color(vga_color bg);
void vga_puts(const char* str);
void vga_puts_colored(const char* str, vga_color fg, vga_color bg);
void vga_putc(char c);
void vga_putc_colored(char c, vga_color fg, vga_color bg);
void vga_putc_at(char c, vga_color fg, vga_color bg, size_t x, size_t y);
void vga_put_hex(uint32_t n);
void vga_put_dec(uint32_t n);
void debug_print(const char* str);

// Advanced VGA functions for UI components
void vga_set_cursor_position(size_t x, size_t y);
void vga_get_cursor_position(size_t* x, size_t y);
void vga_draw_box(size_t x, size_t y, size_t width, size_t height, vga_color fg, vga_color bg);
void vga_draw_horizontal_line(size_t x, size_t y, size_t length, char ch, vga_color fg, vga_color bg);
void vga_draw_vertical_line(size_t x, size_t y, size_t length, char ch, vga_color fg, vga_color bg);
void vga_fill_area(size_t x, size_t y, size_t width, size_t height, char ch, vga_color fg, vga_color bg);

// Animation and effects
void vga_blink_text_at(const char* str, size_t x, size_t y, vga_color fg, vga_color bg);
void vga_highlight_area(size_t x, size_t y, size_t width, size_t height);

// Window/panel drawing primitives
void vga_draw_window_frame(size_t x, size_t y, size_t width, size_t height, const char* title, vga_color fg, vga_color bg);
void vga_draw_button(size_t x, size_t y, size_t width, const char* text, bool pressed, vga_color fg, vga_color bg);
void vga_draw_progress_bar(size_t x, size_t y, size_t width, int progress, vga_color fg, vga_color bg);

#endif
