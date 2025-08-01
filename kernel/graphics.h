// RaeenOS Graphics Driver Interface

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "include/multiboot.h"

/**
 * @brief Initializes the graphics subsystem.
 * 
 * This function should be called once at boot. It assumes the bootloader
 * has already set a VBE/VESA graphics mode and provided the necessary
 * information (e.g., via a multiboot structure).
 * 
 * @param mboot_info A pointer to the multiboot_info_t structure.
 * @return int 0 on success, -1 on failure.
 */
int graphics_init(multiboot_info_t* mboot_info);

/**
 * @brief Draws a single pixel on the screen.
 * 
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The 32-bit color of the pixel (e.g., 0x00RRGGBB).
 */
void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void graphics_clear_screen(uint32_t color);
void graphics_draw_cursor(uint32_t x, uint32_t y, uint32_t color);
void graphics_swap_buffers(void);
uint32_t graphics_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha);
uint32_t graphics_get_pixel(uint32_t x, uint32_t y);
void graphics_draw_pixel_alpha(uint32_t x, uint32_t y, uint32_t color);

/**
 * @brief Applies a simple box blur to a specified rectangular region of the back buffer.
 * 
 * @param x The x-coordinate of the top-left corner of the region.
 * @param y The y-coordinate of the top-left corner of the region.
 * @param width The width of the region.
 * @param height The height of the region.
 * @param radius The radius of the blur (e.g., 1 for a 3x3 blur).
 */
void graphics_apply_blur(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t radius);

/**
 * @brief Draws the desktop wallpaper.
 * 
 * @param wallpaper_data Pointer to the raw pixel data of the wallpaper.
 * @param width Width of the wallpaper.
 * @param height Height of the wallpaper.
 */
void graphics_draw_wallpaper(const uint32_t* wallpaper_data, uint32_t width, uint32_t height);

/**
 * @brief Gets the current screen width.
 * 
 * @return uint32_t The width of the screen in pixels.
 */
uint32_t graphics_get_width(void);

/**
 * @brief Gets the current screen height.
 * 
 * @return uint32_t The height of the screen in pixels.
 */
uint32_t graphics_get_height(void);

#endif // GRAPHICS_H
