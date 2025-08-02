#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

// Forward declaration for multiboot info
struct multiboot_info_t;

// Graphics initialization
int graphics_init(struct multiboot_info_t* mboot_info);

// Basic drawing functions
void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void graphics_draw_pixel_alpha(uint32_t x, uint32_t y, uint32_t color);
void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void graphics_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void graphics_clear_screen(uint32_t color);
void graphics_draw_cursor(uint32_t x, uint32_t y, uint32_t color);
void graphics_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);

// Buffer and color management
void graphics_swap_buffers(void);
uint32_t graphics_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha);
uint32_t graphics_get_pixel(uint32_t x, uint32_t y);

// Advanced graphics functions
void graphics_apply_blur(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t radius);
void graphics_draw_wallpaper(const uint32_t* wallpaper_data, uint32_t width, uint32_t height);

// Screen information
uint32_t graphics_get_width(void);
uint32_t graphics_get_height(void);

#endif // GRAPHICS_H
