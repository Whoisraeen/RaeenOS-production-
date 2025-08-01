#include "c_wrappers.h"

// This C file is the ONLY place that should include the kernel headers.
// This isolates the C environment from the C++ environment.
#include "../../kernel/graphics.h"
#include "../../kernel/window.h"
#include "../../kernel/memory.h"
#include <string.h> // For strlen and memcpy

// Graphics wrappers
void gui_graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    graphics_put_pixel(x, y, color);
}

void gui_graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    graphics_draw_rect(x, y, width, height, color);
}

void gui_graphics_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    // This assumes a corresponding graphics_fill_rect exists in the kernel's graphics.h
    graphics_fill_rect(x, y, width, height, color);
}

void gui_graphics_clear_screen(uint32_t color) {
    graphics_clear_screen(color);
}

void gui_graphics_swap_buffers(void) {
    graphics_swap_buffers();
}

uint32_t gui_graphics_get_width(void) {
    return graphics_get_width();
}

uint32_t gui_graphics_get_height(void) {
    return graphics_get_height();
}

void gui_graphics_draw_string(int x, int y, const char* str, uint32_t color) {
    // Assuming a font.h or similar provides this function in the kernel
    // If not, this will need to be implemented or linked correctly.
    // graphics_draw_string(x, y, str, color);
}

// Window wrappers
struct window_t* gui_window_create(int x, int y, int width, int height) {
    return window_create(x, y, width, height);
}

void gui_window_bring_to_front(struct window_t* win) {
    window_bring_to_front(win);
}

void gui_window_snap_left(struct window_t* win) {
    window_snap_left(win);
}

void gui_window_snap_right(struct window_t* win) {
    window_snap_right(win);
}

void gui_window_draw_rect_in_window(struct window_t* win, int x, int y, int width, int height, uint32_t color) {
    window_draw_rect(win, x, y, width, height, color);
}

// Memory wrappers
void* gui_malloc(size_t size) {
    return kalloc(size);
}

void gui_free(void* ptr) {
    kfree(ptr);
}

// Window property accessors
int gui_window_get_x(struct window_t* win) {
    return win ? win->x : 0;
}

int gui_window_get_y(struct window_t* win) {
    return win ? win->y : 0;
}

int gui_window_get_width(struct window_t* win) {
    return win ? win->width : 0;
}

int gui_window_get_height(struct window_t* win) {
    return win ? win->height : 0;
}

const char* gui_window_get_title(struct window_t* win) {
    return win ? win->title : "";
}

void gui_window_set_title(struct window_t* win, const char* title) {
    if (!win || !title) return;

    // Free the old title and allocate a new one
    if (win->title) {
        kfree(win->title);
    }
    size_t len = strlen(title) + 1;
    win->title = (char*)kalloc(len);
    if (win->title) {
        memcpy(win->title, title, len);
    }
}
