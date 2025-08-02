#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the desktop environment
void desktop_init(void);

// Start the desktop environment main loop
void desktop_start(void);

// Draw a window (placeholder)
void desktop_draw_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, const char* title);

// Handle mouse event (placeholder)
void desktop_handle_mouse_event(uint32_t x, uint32_t y, uint8_t buttons);

// Handle keyboard event (placeholder)
void desktop_handle_keyboard_event(uint8_t scancode, bool pressed);

#endif // DESKTOP_H
