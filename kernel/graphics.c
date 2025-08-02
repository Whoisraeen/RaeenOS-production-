// RaeenOS Graphics Driver Implementation

#include "graphics.h"
#include "font.h"
#include "memory.h" // For kmalloc
#include "string.h" // For memcpy
#include "include/multiboot.h" // Include multiboot header
// Using types.h instead of stddef.h for kernel build

// Framebuffer properties - to be populated by the bootloader info
static uint32_t* framebuffer_addr = NULL;
static uint32_t* back_buffer_addr = NULL;
static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t screen_pitch = 0;
static uint8_t screen_bpp = 0;

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
int graphics_init(multiboot_info_t* mboot_info) {
    if (mboot_info->flags & (1 << 12)) { // Check if framebuffer info is available
        framebuffer_addr = (uint32_t*)mboot_info->framebuffer_addr;
        screen_width = mboot_info->framebuffer_width;
        screen_height = mboot_info->framebuffer_height;
        screen_pitch = mboot_info->framebuffer_pitch;
        screen_bpp = mboot_info->framebuffer_bpp;
    } else {
        // Fallback to hardcoded values if no framebuffer info from Multiboot
        framebuffer_addr = (uint32_t*)0xC0000000; // Example physical address, must be mapped
        screen_width = 1024;
        screen_height = 768;
        screen_pitch = 1024 * 4; // 1024 pixels * 4 bytes per pixel
        screen_bpp = 32;
    }

    // Allocate back buffer
    back_buffer_addr = (uint32_t*)kmalloc(screen_width * screen_height * (screen_bpp / 8));
    if (!back_buffer_addr) {
        return -1; // Failure
    }

    if (framebuffer_addr == NULL) {
        return -1; // Failure
    }

    // Clear the screen to black as part of initialization
    graphics_clear_screen(0x00000000);

    return 0; // Success
}

/**
 * @brief Draws a single pixel on the back buffer.
 */
void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (back_buffer_addr != NULL && x < screen_width && y < screen_height) {
        // Calculate the linear offset into the back buffer
        uint32_t offset = (y * (screen_pitch / 4)) + x;
        back_buffer_addr[offset] = color;
    }
}

/**
 * @brief Draws a rectangle on the back buffer.
 */
void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (back_buffer_addr == NULL || x >= screen_width || y >= screen_height) return;

    // Clamp dimensions to screen boundaries
    uint32_t clamped_width = (x + width > screen_width) ? (screen_width - x) : width;
    uint32_t clamped_height = (y + height > screen_height) ? (screen_height - y) : height;

    if (clamped_width == 0 || clamped_height == 0) return;

    // Optimize for solid rectangles using memcpy
    if (screen_bpp == 32) { // Only for 32-bit color depth
        uint32_t row_size_bytes = clamped_width * sizeof(uint32_t);
        for (uint32_t i = 0; i < clamped_height; i++) {
            uint32_t* row_start = back_buffer_addr + (y + i) * (screen_pitch / 4) + x;
            for (uint32_t j = 0; j < clamped_width; j++) {
                row_start[j] = color;
            }
        }
    } else {
        // Fallback to pixel-by-pixel for other color depths or if not 32-bit
        for (uint32_t i = 0; i < clamped_height; i++) {
            for (uint32_t j = 0; j < clamped_width; j++) {
                graphics_put_pixel(x + j, y + i, color);
            }
        }
    }
}

/**
 * @brief Draws a line on the back buffer using Bresenham's line algorithm.
 */
void graphics_draw_line(uint33_t x0, uint33_t y0, uint33_t x1, uint33_t y1, uint32_t color) {
    int dx = abs((int)x1 - (int)x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = abs((int)y1 - (int)y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    for (;;) {
        graphics_put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

/**
 * @brief Clears the entire screen (back buffer) with a given color.
 */
void graphics_clear_screen(uint32_t color) {
    if (back_buffer_addr == NULL) return;

    uint32_t total_pixels = screen_width * screen_height;
    for (uint32_t i = 0; i < total_pixels; i++) {
        back_buffer_addr[i] = color;
    }
}

/**
 * @brief Draws a simple mouse cursor on the back buffer.
 */
void graphics_draw_cursor(uint32_t x, uint32_t y, uint32_t color) {
    // Simple cross-shaped cursor
    graphics_draw_rect(x - 5, y, 11, 1, color);
    graphics_draw_rect(x, y - 5, 1, 11, color);
}

/**
 * @brief Gets the width of the screen in pixels.
 */
uint32_t graphics_get_width(void) {
    return screen_width;
}

/**
 * @brief Gets the height of the screen in pixels.
 */
uint32_t graphics_get_height(void) {
    return screen_height;
}

/**
 * @brief Draws a single character to the back buffer.
 */
void graphics_draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
    const uint8_t* glyph = font_get_char(c);
    if (!glyph) return;

    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            if ((glyph[i] >> (7 - j)) & 1) {
                graphics_put_pixel(x + j, y + i, color);
            }
        }
    }
}

/**
 * @brief Draws a null-terminated string to the back buffer.
 */
void graphics_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color) {
    if (!str) return;

    uint32_t current_x = x;
    while (*str) {
        graphics_draw_char(current_x, y, *str, color);
        current_x += FONT_WIDTH;
        str++;
    }
}

/**
 * @brief Swaps the back buffer to the front buffer (displays the content).
 */
void graphics_swap_buffers(void) {
    if (framebuffer_addr && back_buffer_addr) {
        memcpy(framebuffer_addr, back_buffer_addr, screen_width * screen_height * sizeof(uint32_t));
    }
}

uint32_t graphics_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha) {
    // Alpha is 0-255, where 255 is fully opaque (color2) and 0 is fully transparent (color1)
    uint8_t alpha_inv = 255 - alpha;

    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;

    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;

    uint8_t r = (r1 * alpha_inv + r2 * alpha) / 255;
    uint8_t g = (g1 * alpha_inv + g2 * alpha) / 255;
    uint8_t b = (b1 * alpha_inv + b2 * alpha) / 255;

    return (r << 16) | (g << 8) | b;
}

uint32_t graphics_get_pixel(uint32_t x, uint32_t y) {
    if (back_buffer_addr != NULL && x < screen_width && y < screen_height) {
        uint32_t offset = (y * (screen_pitch / 4)) + x;
        return back_buffer_addr[offset];
    }
    return 0; // Return black if out of bounds or not initialized
}

void graphics_draw_pixel_alpha(uint32_t x, uint32_t y, uint32_t color) {
    if (back_buffer_addr != NULL && x < screen_width && y < screen_height) {
        uint32_t current_pixel = graphics_get_pixel(x, y);
        uint8_t alpha = (color >> 24) & 0xFF; // Get alpha component from color
        uint32_t blended_color = graphics_blend_colors(current_pixel, color, alpha);
        uint32_t offset = (y * (screen_pitch / 4)) + x;
        back_buffer_addr[offset] = blended_color;
    }
}

void graphics_apply_blur(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t radius) {
    if (!back_buffer_addr || radius == 0) return;

    // Create a temporary buffer to store the blurred pixels
    uint32_t* temp_buffer = (uint32_t*)kmalloc(width * height * sizeof(uint32_t));
    if (!temp_buffer) return; // Out of memory

    for (uint32_t cy = 0; cy < height; cy++) {
        for (uint32_t cx = 0; cx < width; cx++) {
            uint32_t r_sum = 0, g_sum = 0, b_sum = 0;
            uint32_t pixel_count = 0;

            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int sample_x = x + cx + dx;
                    int sample_y = y + cy + dy;

                    if (sample_x >= 0 && sample_x < screen_width &&
                        sample_y >= 0 && sample_y < screen_height) {
                        uint32_t pixel = graphics_get_pixel(sample_x, sample_y);
                        r_sum += (pixel >> 16) & 0xFF;
                        g_sum += (pixel >> 8) & 0xFF;
                        b_sum += pixel & 0xFF;
                        pixel_count++;
                    }
                }
            }
            if (pixel_count > 0) {
                uint32_t avg_r = r_sum / pixel_count;
                uint32_t avg_g = g_sum / pixel_count;
                uint32_t avg_b = b_sum / pixel_count;
                temp_buffer[cy * width + cx] = (avg_r << 16) | (avg_g << 8) | avg_b;
            } else {
                temp_buffer[cy * width + cx] = 0; // Black if no pixels sampled
            }
        }
    }

    // Copy the blurred region back to the main back buffer
    for (uint32_t cy = 0; cy < height; cy++) {
        for (uint32_t cx = 0; cx < width; cx++) {
            graphics_put_pixel(x + cx, y + cy, temp_buffer[cy * width + cx]);
        }
    }

    kfree(temp_buffer);
}

void graphics_draw_wallpaper(const uint32_t* wallpaper_data, uint32_t width, uint32_t height) {
    if (!back_buffer_addr || !wallpaper_data) return;

    for (uint32_t y = 0; y < screen_height; y++) {
        for (uint32_t x = 0; x < screen_width; x++) {
            // Calculate corresponding pixel in wallpaper data, handling tiling/stretching
            uint32_t src_x = x % width;
            uint32_t src_y = y % height;
            uint32_t color = wallpaper_data[src_y * width + src_x];
            graphics_put_pixel(x, y, color);
        }
    }
}