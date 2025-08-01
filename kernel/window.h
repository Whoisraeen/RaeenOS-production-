// RaeenOS Windowing System Interface

#ifndef WINDOW_H
#define WINDOW_H

#include "include/types.h"
// Using types.h for kernel build

// Forward-declare the widget structure to avoid circular includes
struct widget_t;

// Represents a single window on the screen
struct window_t {
    uint32_t id;
    int x, y;           // Position of the top-left corner
    int width, height;  // Size of the window
    uint32_t* buffer;   // The window's private framebuffer
    char* title;
    struct widget_t* widgets; // Linked list of child widgets
    int z_order;        // Stacking order
    uint32_t desktop_id; // The desktop this window belongs to
    struct window_t* next;
};

typedef struct window_t window_t;

/**
 * @brief Initializes the window manager.
 */
void window_manager_init(void);

/**
 * @brief Creates a new window.
 * 
 * @param x The initial x-coordinate.
 * @param y The initial y-coordinate.
 * @param width The width of the window.
 * @param height The height of the window.
 * @return window_t* A pointer to the newly created window, or NULL on failure.
 */
window_t* window_create(int x, int y, int width, int height);

/**
 * @brief Draws the entire desktop, composing all windows onto the screen.
 */
void window_manager_compose(void);

/**
 * @brief Finds the top-most window at a given screen coordinate.
 * 
 * @param x The screen x-coordinate.
 * @param y The screen y-coordinate.
 * @return window_t* A pointer to the window, or NULL if no window is at that location.
 */
window_t* window_find_at_coords(int x, int y);

/**
 * @brief Brings a window to the front (top of the Z-order).
 * 
 * @param win The window to bring to front.
 */
void window_bring_to_front(window_t* win);
void window_tile_all(void);

/**
 * @brief Switches to a different virtual desktop.
 * 
 * @param desktop_id The ID of the desktop to switch to.
 */
void window_switch_desktop(uint32_t desktop_id);

/**
 * @brief Snaps a window to the left half of the screen.
 * 
 * @param win The window to snap.
 */
void window_snap_left(window_t* win);

/**
 * @brief Snaps a window to the right half of the screen.
 * 
 * @param win The window to snap.
 */
void window_snap_right(window_t* win);

/**
 * @brief Draws a filled rectangle within a window's buffer.
 * 
 * @param win The window to draw in.
 * @param x The rectangle's x-coordinate relative to the window.
 * @param y The rectangle's y-coordinate relative to the window.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color The color of the rectangle.
 */
void window_draw_rect(window_t* win, int x, int y, int width, int height, uint32_t color);

/**
 * @brief Draws a single character into a window's buffer.
 *
 * @param win The window to draw in.
 * @param x The local x-coordinate in the window.
 * @param y The local y-coordinate in the window.
 * @param c The character to draw.
 * @param color The color of the character.
 */
void window_draw_char(window_t* win, int x, int y, char c, uint32_t color);

/**
 * @brief Draws a string into a window's buffer.
 *
 * @param win The window to draw in.
 * @param x The local x-coordinate for the start of the string.
 * @param y The local y-coordinate for the start of the string.
 * @param str The null-terminated string to draw.
 * @param color The color of the string.
 */
void window_draw_string(window_t* win, int x, int y, const char* str, uint32_t color);

/**
 * @brief Sets the desktop wallpaper.
 * 
 * @param wallpaper_data Pointer to the raw pixel data of the wallpaper.
 * @param width Width of the wallpaper.
 * @param height Height of the wallpaper.
 */
void window_set_wallpaper(const uint32_t* wallpaper_data, uint32_t width, uint32_t height);

#endif // WINDOW_H
