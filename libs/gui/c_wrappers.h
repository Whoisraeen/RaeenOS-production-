#pragma once

// This header declares the C wrapper functions that provide a stable,
// C++-compatible interface to the underlying C kernel APIs.

#include <stddef.h> // For size_t
#include <stdint.h> // For uint32_t

// Forward-declare structs to avoid including kernel headers
struct window_t;
struct multiboot_info_t;

#ifdef __cplusplus
extern "C" {
#endif

// Graphics wrappers
int gui_graphics_init(struct multiboot_info_t* mboot_info);
void gui_graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void gui_graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void gui_graphics_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void gui_graphics_clear_screen(uint32_t color);
void gui_graphics_swap_buffers(void);
uint32_t gui_graphics_get_width(void);
uint32_t gui_graphics_get_height(void);
void gui_graphics_draw_string(int x, int y, const char* str, uint32_t color);

// Window wrappers
struct window_t* gui_window_create(int x, int y, int width, int height);
void gui_window_bring_to_front(struct window_t* win);
void gui_window_snap_left(struct window_t* win);
void gui_window_snap_right(struct window_t* win);
void gui_window_draw_rect_in_window(struct window_t* win, int x, int y, int width, int height, uint32_t color);

// Window property accessors
int gui_window_get_x(struct window_t* win);
int gui_window_get_y(struct window_t* win);
int gui_window_get_width(struct window_t* win);
int gui_window_get_height(struct window_t* win);
const char* gui_window_get_title(struct window_t* win);
void gui_window_set_title(struct window_t* win, const char* title);

// Memory wrappers
void* gui_malloc(size_t size);
void gui_free(void* ptr);

#ifdef __cplusplus
}
#endif
