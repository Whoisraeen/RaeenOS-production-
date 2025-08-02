#include "window_manager.h"
#include "desktop.h"
#include "../../kernel/vga.h"
#include "../../kernel/graphics.h"
#include "../../kernel/memory.h"
#include "../../libs/libc/include/string.h"

#define MAX_WINDOWS 32

static window_t* windows[MAX_WINDOWS];
static uint32_t num_windows = 0;
static uint32_t next_window_id = 0;
static window_t* focused_window = NULL;

// Function to sort windows by Z-order (simple bubble sort for now)
static void sort_windows_by_z_order() {
    for (uint32_t i = 0; i < num_windows; i++) {
        for (uint32_t j = i + 1; j < num_windows; j++) {
            if (windows[i]->z_order > windows[j]->z_order) {
                window_t* temp = windows[i];
                windows[i] = windows[j];
                windows[j] = temp;
            }
        }
    }
}

void wm_init(void) {
    debug_print("Window Manager initialized (placeholder).\n");
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i] = NULL;
    }
}

window_t* wm_create_window(const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (num_windows >= MAX_WINDOWS) {
        debug_print("WM: Max windows reached.\n");
        return NULL;
    }

    window_t* win = (window_t*)kmalloc(sizeof(window_t));
    if (!win) return NULL;

    memset(win, 0, sizeof(window_t));
    win->id = next_window_id++;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->background_color = color;
    win->focused = false;
    win->minimized = false;
    win->maximized = false;
    win->z_order = num_windows; // Assign initial Z-order

    windows[num_windows++] = win;
    debug_print("WM: Created window ");
    debug_print(title);
    debug_print(" (ID: ");
    vga_put_dec(win->id);
    debug_print(")\n");

    wm_focus_window(win); // Focus new window by default
    return win;
}

void wm_destroy_window(window_t* window) {
    if (!window) return;

    debug_print("WM: Destroying window ");
    debug_print(window->title);
    debug_print(" (ID: ");
    vga_put_dec(window->id);
    debug_print(")\n");

    for (uint32_t i = 0; i < num_windows; i++) {
        if (windows[i] == window) {
            kfree(windows[i]);
            windows[i] = NULL;
            // Shift remaining windows
            for (uint32_t j = i; j < num_windows - 1; j++) {
                windows[j] = windows[j + 1];
            }
            windows[--num_windows] = NULL;
            break;
        }
    }
    if (focused_window == window) {
        focused_window = NULL; // Clear focused window
    }
    wm_redraw_windows();
}

void wm_move_window(window_t* window, uint32_t new_x, uint32_t new_y) {
    if (!window) return;
    debug_print("WM: Moving window ");
    debug_print(window->title);
    debug_print(" to (");
    vga_put_dec(new_x);
    debug_print(", ");
    vga_put_dec(new_y);
    debug_print(")\n");
    window->x = new_x;
    window->y = new_y;
    wm_redraw_windows();
}

void wm_resize_window(window_t* window, uint32_t new_width, uint32_t new_height) {
    if (!window) return;
    debug_print("WM: Resizing window ");
    debug_print(window->title);
    debug_print(" to ");
    vga_put_dec(new_width);
    debug_print("x");
    vga_put_dec(new_height);
    debug_print("\n");
    window->width = new_width;
    window->height = new_height;
    wm_redraw_windows();
}

void wm_minimize_window(window_t* window) {
    if (!window) return;
    debug_print("WM: Minimizing window ");
    debug_print(window->title);
    debug_print("\n");
    window->minimized = true;
    wm_redraw_windows();
}

void wm_maximize_window(window_t* window) {
    if (!window) return;
    debug_print("WM: Maximizing window ");
    debug_print(window->title);
    debug_print("\n");
    // TODO: Set window to screen size
    wm_redraw_windows();
}

void wm_restore_window(window_t* window) {
    if (!window) return;
    debug_print("WM: Restoring window ");
    debug_print(window->title);
    debug_print("\n");
    window->minimized = false;
    window->maximized = false;
    // TODO: Restore original size/position
    wm_redraw_windows();
}

void wm_close_window(window_t* window) {
    if (!window) return;
    debug_print("WM: Closing window ");
    debug_print(window->title);
    debug_print("\n");
    wm_destroy_window(window);
}

void wm_focus_window(window_t* window) {
    if (focused_window) {
        focused_window->focused = false;
    }
    focused_window = window;
    if (focused_window) {
        focused_window->focused = true;
        // Bring to front (highest Z-order)
        for (uint32_t i = 0; i < num_windows; i++) {
            if (windows[i] == focused_window) {
                windows[i]->z_order = num_windows; // Highest Z-order
            } else {
                windows[i]->z_order--; // Decrease others' Z-order
            }
        }
        sort_windows_by_z_order();
        debug_print("WM: Focused window ");
        debug_print(focused_window->title);
        debug_print("\n");
    }
    wm_redraw_windows();
}

void wm_redraw_windows(void) {
    // Clear screen first
    graphics_clear_screen(0x000000); // Black background

    // Redraw all windows in Z-order
    for (uint32_t i = 0; i < num_windows; i++) {
        window_t* win = windows[i];
        if (win && !win->minimized) {
            desktop_draw_window(win->x, win->y, win->width, win->height, win->background_color, win->title);
            // Draw focus border if focused
            if (win->focused) {
                graphics_draw_rect(win->x - 2, win->y - 2, win->width + 4, win->height + 4, 0xFFFF00); // Yellow border
            }
        }
    }
    graphics_swap_buffers();
}

void wm_handle_mouse_click(uint32_t x, uint32_t y, uint8_t button) {
    debug_print("WM: Mouse click at (");
    vga_put_dec(x);
    debug_print(", ");
    vga_put_dec(y);
    debug_print(") button: ");
    vga_put_dec(button);
    debug_print("\n");

    // Find window clicked on (iterate in reverse Z-order)
    window_t* clicked_window = NULL;
    for (int i = num_windows - 1; i >= 0; i--) {
        window_t* win = windows[i];
        if (win && x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height) {
            clicked_window = win;
            break;
        }
    }

    if (clicked_window) {
        wm_focus_window(clicked_window);
        // TODO: Implement drag/resize logic based on click location (title bar, borders)
    }
}

void wm_handle_keyboard_event(uint8_t scancode, bool pressed) {
    debug_print("WM: Keyboard event scancode ");
    vga_put_hex(scancode);
    debug_print(" pressed: ");
    vga_put_dec(pressed);
    debug_print("\n");

    if (focused_window) {
        // TODO: Pass keyboard events to focused window for application handling
        debug_print("WM: Passing key to focused window ");
        debug_print(focused_window->title);
        debug_print("\n");
    }
}