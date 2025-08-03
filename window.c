// RaeenOS Windowing System Implementation

#include "window.h"
#include "graphics.h"
#include "pmm.h"
#include "pmm_production.h"
#include "mouse.h"
#include "ui/widget.h"
#include "font.h"
#include "string.h"
#include "ui/theme.h" // Include theme header
#include <stdbool.h>
#include <stddef.h>

// Forward declaration for production PMM function
extern void* pmm_alloc_pages(unsigned int order, unsigned int flags, int node);

// Head of the window linked list
static window_t* window_list_head = NULL;

// Placeholder for wallpaper data
static const uint32_t* current_wallpaper_data = NULL;
static uint32_t current_wallpaper_width = 0;
static uint32_t current_wallpaper_height = 0;

/**
 * @brief Creates a new window.
 */
window_t* window_create(int x, int y, int width, int height, const char* title) {
    window_t* win = (window_t*)pmm_alloc_frame();
    if (!win) return NULL;

    memset(win, 0, sizeof(window_t));

    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->desktop_id = 0; // Default to desktop 0
    
    size_t buffer_size = width * height * sizeof(uint32_t);
    size_t num_pages = (buffer_size + PAGE_SIZE - 1) / PAGE_SIZE;
    win->buffer = (uint32_t*)pmm_alloc_pages(0, 0, -1); // order=0 (1 page), flags=0, node=-1 (any)
    
    if (!win->buffer) {
        pmm_free_frame((void*)win);
        return NULL;
    }

    if (title) {
        size_t title_len = strlen(title) + 1;
        win->title = (char*)pmm_alloc_frame(); // Simplified allocation
        if (win->title) {
            memcpy(win->title, title, title_len);
        }
    }

    memset(win->buffer, 0, buffer_size);

    // Add to the top of the window list (highest z-order)
    win->next = window_list_head;
    window_list_head = win;

    return win;
}

/**
 * @brief Draws a filled rectangle within a window's buffer.
 */
void window_draw_rect(window_t* win, int x, int y, int width, int height, uint32_t color) {
    if (!win || !win->buffer) return;

    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (j >= 0 && j < win->width && i >= 0 && i < win->height) {
                win->buffer[i * win->width + j] = color;
            }
        }
    }
}

/**
 * @brief Draws a single character into a window's buffer.
 */
void window_draw_char(window_t* win, int x, int y, char c, uint32_t color) {
    if (!win || !win->buffer) return;

    const uint8_t* glyph = font_get_char(c);
    if (!glyph) return;

    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            if ((glyph[i] >> (7 - j)) & 1) {
                int draw_x = x + j;
                int draw_y = y + i;
                // Basic clipping
                if (draw_x >= 0 && draw_x < win->width && draw_y >= 0 && draw_y < win->height) {
                    win->buffer[draw_y * win->width + draw_x] = color;
                }
            }
        }
    }
}

/**
 * @brief Draws a string into a window's buffer.
 */
void window_draw_string(window_t* win, int x, int y, const char* str, uint32_t color) {
    if (!win || !str) return;

    int current_x = x;
    while (*str) {
        window_draw_char(win, current_x, y, *str, color);
        current_x += FONT_WIDTH;
        str++;
    }
}

void window_set_wallpaper(const uint32_t* wallpaper_data, uint32_t width, uint32_t height) {
    current_wallpaper_data = wallpaper_data;
    current_wallpaper_width = width;
    current_wallpaper_height = height;
    window_manager_compose(); // Redraw the screen with the new wallpaper
}

static uint32_t current_desktop_id = 0;

/**
 * @brief Switches to a different virtual desktop.
 * 
 * @param desktop_id The ID of the desktop to switch to.
 */
void window_switch_desktop(uint32_t desktop_id) {
    current_desktop_id = desktop_id;
    window_manager_compose(); // Redraw the screen for the new desktop
}

/**
 * @brief Snaps a window to the left half of the screen.
 * 
 * @param win The window to snap.
 */
void window_snap_left(window_t* win) {
    if (!win) return;
    int screen_width = graphics_get_width();
    int screen_height = graphics_get_height();
    win->x = 0;
    win->y = 0;
    win->width = screen_width / 2;
    win->height = screen_height;
}

/**
 * @brief Snaps a window to the right half of the screen.
 * 
 * @param win The window to snap.
 */
void window_snap_right(window_t* win) {
    if (!win) return;
    int screen_width = graphics_get_width();
    int screen_height = graphics_get_height();
    win->x = screen_width / 2;
    win->y = 0;
    win->width = screen_width / 2;
    win->height = screen_height;
}

/**
 * @brief Main composition function. Draws all windows to the screen.
 */
void window_manager_compose(void) {
    // Draw the desktop background (wallpaper or solid color)
    if (current_wallpaper_data && current_wallpaper_width > 0 && current_wallpaper_height > 0) {
        graphics_draw_wallpaper(current_wallpaper_data, current_wallpaper_width, current_wallpaper_height);
    } else {
        graphics_clear_screen(0x00333333); // Default solid color
    }

    // Draw each window from back to front
    // We need to iterate the list in reverse order for proper blending
    window_t* current = window_list_head;
    window_t* prev = NULL;
    while (current) {
        window_t* next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    window_list_head = prev; // Now head points to the last window (back-most)

    current = window_list_head;
    while (current) {
        if (current->desktop_id == current_desktop_id) { // Only draw windows on the current desktop
            // Draw window decorations (title bar)
            window_draw_rect(current, 0, 0, current->width, 20, 0x00555555);
            if (current->title) {
                window_draw_string(current, 5, 5, current->title, 0xFFFFFFFF);
            }

            // Apply glass effect to the window background
            theme_apply_glass_effect(current->x, current->y, current->width, current->height);

            // Redraw widgets for this window
            widget_draw_all(current);
            
            // Blit the window buffer to the screen, blending with existing content
            for (int y = 0; y < current->height; y++) {
                for (int x = 0; x < current->width; x++) {
                    uint32_t window_pixel = current->buffer[y * current->width + x];
                    uint32_t screen_pixel = graphics_get_pixel(current->x + x, current->y + y);
                    // For now, simple alpha blending (assuming window_pixel has alpha 255 if opaque)
                    // A more sophisticated compositor would handle per-pixel alpha.
                    graphics_put_pixel(current->x + x, current->y + y, graphics_blend_colors(screen_pixel, window_pixel, 255));
                }
            }
        }
        current = current->next;
    }

    // Draw the mouse cursor on top
    struct mouse_state_t current_mouse;
    mouse_get_state(&current_mouse);
    graphics_draw_cursor(current_mouse.x, current_mouse.y, 0xFFFFFFFF);

    // Swap buffers to display the composed frame
    graphics_swap_buffers();
}

/**
 * @brief Finds the top-most window at a given screen coordinate.
 */
window_t* window_find_at_coords(int x, int y) {
    window_t* current = window_list_head;
    while (current) {
        if (current->desktop_id == current_desktop_id && // Only consider windows on the current desktop
            x >= current->x && x < (current->x + current->width) &&
            y >= current->y && y < (current->y + current->height)) {
            return current; // Return the first one found (top-most)
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Brings a window to the front (top of the Z-order).
 * 
 * @param win The window to bring to front.
 */
void window_bring_to_front(window_t* win) {
    if (!win || win == window_list_head) {
        return; // Already at front or invalid window
    }

    window_t* current = window_list_head;
    window_t* prev = NULL;

    while (current && current != win) {
        prev = current;
        current = current->next;
    }

    if (current == win) {
        // Remove from current position
        if (prev) {
            prev->next = current->next;
        } else {
            // This case should not happen if win != window_list_head
            window_list_head = current->next;
        }

        // Add to front
        win->next = window_list_head;
        window_list_head = win;
    }
}

/**
 * @brief Tiles all open windows in a simple grid layout.
 */
void window_tile_all(void) {
    int num_windows = 0;
    window_t* current = window_list_head;
    while (current) {
        if (current->desktop_id == current_desktop_id) { // Only tile windows on the current desktop
            num_windows++;
        }
        current = current->next;
    }

    if (num_windows == 0) return;

    int screen_width = graphics_get_width();
    int screen_height = graphics_get_height();

    int cols = 1;
    while (cols * cols < num_windows) {
        cols++;
    }
    int rows = (num_windows + cols - 1) / cols;

    int window_width = screen_width / cols;
    int window_height = screen_height / rows;

    current = window_list_head;
    int i = 0;
    while (current) {
        if (current->desktop_id == current_desktop_id) { // Only tile windows on the current desktop
            int col = i % cols;
            int row = i / cols;

            current->x = col * window_width;
            current->y = row * window_height;
            current->width = window_width;
            current->height = window_height;
            i++;
        }
        current = current->next;
    }
}