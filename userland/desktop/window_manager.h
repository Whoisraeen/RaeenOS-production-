#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// Window structure (placeholder)
typedef struct window {
    uint32_t id;
    char title[128];
    uint32_t x, y, width, height;
    uint32_t background_color;
    bool focused;
    bool minimized;
    bool maximized;
    // Add more window properties (e.g., Z-order, parent/child relationships)
} window_t;

// Initialize the window manager
void wm_init(void);

// Create a new window
window_t* wm_create_window(const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

// Destroy a window
void wm_destroy_window(window_t* window);

// Move a window
void wm_move_window(window_t* window, uint32_t new_x, uint32_t new_y);

// Resize a window
void wm_resize_window(window_t* window, uint32_t new_width, uint32_t new_height);

// Focus a window
void wm_focus_window(window_t* window);

// Redraw all windows
void wm_redraw_windows(void);

#endif // WINDOW_MANAGER_H
