// Enhanced VGA implementation for RaeenUI prototype - RaeenOS

#include "vga.h"
#include "string.h"
#include <stdbool.h>

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static vga_color current_fg = VGA_COLOR_LIGHT_GREY;
static vga_color current_bg = VGA_COLOR_BLACK;

// Helper function to create VGA character with color
static inline uint16_t vga_char_with_color(char c, vga_color fg, vga_color bg) {
    return (uint16_t)c | ((uint16_t)(fg | (bg << 4)) << 8);
}

// Enhanced scrolling with color preservation
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
            vga_buffer[index] = vga_char_with_color(' ', current_fg, current_bg);
        }
        
        terminal_row = VGA_HEIGHT - 1;
        terminal_column = 0;
    }
}

// ============================================================================
// BASIC VGA FUNCTIONS (Enhanced versions)
// ============================================================================

void vga_init(void) {
    current_fg = VGA_COLOR_LIGHT_GREY;
    current_bg = VGA_COLOR_BLACK;
    vga_clear_with_color(current_bg);
}

void vga_clear(void) {
    vga_clear_with_color(VGA_COLOR_BLACK);
}

void vga_clear_with_color(vga_color bg) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_char_with_color(' ', VGA_COLOR_LIGHT_GREY, bg);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    current_bg = bg;
}

void vga_puts(const char* str) {
    vga_puts_colored(str, current_fg, current_bg);
}

void vga_puts_colored(const char* str, vga_color fg, vga_color bg) {
    while (*str) {
        vga_putc_colored(*str++, fg, bg);
    }
}

void vga_putc(char c) {
    vga_putc_colored(c, current_fg, current_bg);
}

void vga_putc_colored(char c, vga_color fg, vga_color bg) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        terminal_scroll();
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            vga_buffer[index] = vga_char_with_color(' ', fg, bg);
        }
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = vga_char_with_color(c, fg, bg);
    
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        terminal_scroll();
    }
}

void vga_putc_at(char c, vga_color fg, vga_color bg, size_t x, size_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_char_with_color(c, fg, bg);
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

    char buf[10];
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
    vga_puts_colored("[DEBUG] ", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts_colored(str, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
}

// ============================================================================
// ADVANCED VGA FUNCTIONS FOR UI COMPONENTS
// ============================================================================

void vga_set_cursor_position(size_t x, size_t y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        terminal_column = x;
        terminal_row = y;
    }
}

void vga_get_cursor_position(size_t* x, size_t* y) {
    if (x) *x = terminal_column;
    if (y) *y = terminal_row;
}

void vga_draw_box(size_t x, size_t y, size_t width, size_t height, vga_color fg, vga_color bg) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT || width == 0 || height == 0) return;
    
    // Clamp dimensions to screen bounds
    if (x + width > VGA_WIDTH) width = VGA_WIDTH - x;
    if (y + height > VGA_HEIGHT) height = VGA_HEIGHT - y;
    
    // Draw corners and edges
    vga_putc_at('+', fg, bg, x, y);                              // Top-left
    vga_putc_at('+', fg, bg, x + width - 1, y);                 // Top-right
    vga_putc_at('+', fg, bg, x, y + height - 1);                // Bottom-left
    vga_putc_at('+', fg, bg, x + width - 1, y + height - 1);    // Bottom-right
    
    // Draw horizontal lines
    for (size_t i = 1; i < width - 1; i++) {
        vga_putc_at('-', fg, bg, x + i, y);                      // Top
        vga_putc_at('-', fg, bg, x + i, y + height - 1);         // Bottom
    }
    
    // Draw vertical lines
    for (size_t i = 1; i < height - 1; i++) {
        vga_putc_at('|', fg, bg, x, y + i);                      // Left
        vga_putc_at('|', fg, bg, x + width - 1, y + i);          // Right
    }
}

void vga_draw_horizontal_line(size_t x, size_t y, size_t length, char ch, vga_color fg, vga_color bg) {
    if (y >= VGA_HEIGHT) return;
    
    for (size_t i = 0; i < length && (x + i) < VGA_WIDTH; i++) {
        vga_putc_at(ch, fg, bg, x + i, y);
    }
}

void vga_draw_vertical_line(size_t x, size_t y, size_t length, char ch, vga_color fg, vga_color bg) {
    if (x >= VGA_WIDTH) return;
    
    for (size_t i = 0; i < length && (y + i) < VGA_HEIGHT; i++) {
        vga_putc_at(ch, fg, bg, x, y + i);
    }
}

void vga_fill_area(size_t x, size_t y, size_t width, size_t height, char ch, vga_color fg, vga_color bg) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    // Clamp dimensions
    if (x + width > VGA_WIDTH) width = VGA_WIDTH - x;
    if (y + height > VGA_HEIGHT) height = VGA_HEIGHT - y;
    
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            vga_putc_at(ch, fg, bg, x + col, y + row);
        }
    }
}

// ============================================================================
// ANIMATION AND EFFECTS
// ============================================================================

void vga_blink_text_at(const char* str, size_t x, size_t y, vga_color fg, vga_color bg) {
    // Simple blink effect by alternating colors
    static bool blink_state = false;
    vga_color current_fg = blink_state ? bg : fg;
    vga_color current_bg = blink_state ? fg : bg;
    
    size_t original_x, original_y;
    vga_get_cursor_position(&original_x, &original_y);
    
    vga_set_cursor_position(x, y);
    vga_puts_colored(str, current_fg, current_bg);
    
    vga_set_cursor_position(original_x, original_y);
    blink_state = !blink_state;
}

void vga_highlight_area(size_t x, size_t y, size_t width, size_t height) {
    // Highlight by inverting colors
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    if (x + width > VGA_WIDTH) width = VGA_WIDTH - x;
    if (y + height > VGA_HEIGHT) height = VGA_HEIGHT - y;
    
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            size_t index = (y + row) * VGA_WIDTH + (x + col);
            uint16_t current = vga_buffer[index];
            
            // Extract character and colors
            char ch = (char)(current & 0xFF);
            vga_color fg = (vga_color)((current >> 8) & 0x0F);
            vga_color bg = (vga_color)((current >> 12) & 0x0F);
            
            // Swap colors for highlight effect
            vga_buffer[index] = vga_char_with_color(ch, bg, fg);
        }
    }
}

// ============================================================================
// WINDOW/PANEL DRAWING PRIMITIVES
// ============================================================================

void vga_draw_window_frame(size_t x, size_t y, size_t width, size_t height, const char* title, vga_color fg, vga_color bg) {
    if (width < 3 || height < 3) return;
    
    // Draw the outer frame
    vga_draw_box(x, y, width, height, fg, bg);
    
    // Draw title bar if title provided
    if (title && height > 2) {
        // Fill title bar area
        vga_fill_area(x + 1, y + 1, width - 2, 1, ' ', VGA_COLOR_WHITE, VGA_COLOR_BLUE);
        
        // Draw title text (centered)
        size_t title_len = strlen(title);
        size_t title_x = x + 1 + (width - 2 - title_len) / 2;
        if (title_len < width - 2) {
            size_t orig_x, orig_y;
            vga_get_cursor_position(&orig_x, &orig_y);
            vga_set_cursor_position(title_x, y + 1);
            vga_puts_colored(title, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
            vga_set_cursor_position(orig_x, orig_y);
        }
        
        // Draw title bar separator
        vga_draw_horizontal_line(x + 1, y + 2, width - 2, '-', fg, bg);
    }
}

void vga_draw_button(size_t x, size_t y, size_t width, const char* text, bool pressed, vga_color fg, vga_color bg) {
    if (width < 3) return;
    
    vga_color button_bg = pressed ? VGA_COLOR_DARK_GREY : bg;
    vga_color button_fg = pressed ? VGA_COLOR_WHITE : fg;
    
    // Draw button background
    vga_fill_area(x, y, width, 3, ' ', button_fg, button_bg);
    
    // Draw button border
    if (!pressed) {
        // Normal button - raised appearance
        vga_draw_horizontal_line(x, y, width, '-', VGA_COLOR_WHITE, button_bg);
        vga_draw_vertical_line(x, y, 3, '|', VGA_COLOR_WHITE, button_bg);
        vga_draw_horizontal_line(x + 1, y + 2, width - 1, '-', VGA_COLOR_DARK_GREY, button_bg);
        vga_draw_vertical_line(x + width - 1, y + 1, 2, '|', VGA_COLOR_DARK_GREY, button_bg);
    } else {
        // Pressed button - sunken appearance
        vga_draw_horizontal_line(x, y, width, '-', VGA_COLOR_DARK_GREY, button_bg);
        vga_draw_vertical_line(x, y, 3, '|', VGA_COLOR_DARK_GREY, button_bg);
    }
    
    // Draw button text (centered)
    if (text) {
        size_t text_len = strlen(text);
        size_t text_x = x + (width - text_len) / 2;
        if (text_len < width) {
            size_t orig_x, orig_y;
            vga_get_cursor_position(&orig_x, &orig_y);
            vga_set_cursor_position(text_x, y + 1);
            vga_puts_colored(text, button_fg, button_bg);
            vga_set_cursor_position(orig_x, orig_y);
        }
    }
}

void vga_draw_progress_bar(size_t x, size_t y, size_t width, int progress, vga_color fg, vga_color bg) {
    if (width < 2) return;
    
    // Clamp progress to 0-100
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;
    
    // Calculate filled portion
    size_t filled_width = (width * progress) / 100;
    
    // Draw progress bar frame
    vga_draw_box(x, y, width, 3, fg, bg);
    
    // Fill progress area
    if (filled_width > 0) {
        vga_fill_area(x + 1, y + 1, filled_width - 1, 1, '#', VGA_COLOR_GREEN, bg);
    }
    
    // Fill empty area
    if (filled_width < width - 1) {
        vga_fill_area(x + filled_width, y + 1, width - filled_width - 1, 1, ' ', fg, bg);
    }
}