#ifndef THEME_H
#define THEME_H

#include <stdint.h>

// Theme color definitions
typedef struct {
    uint32_t window_bg;
    uint32_t window_border;
    uint32_t title_bar_bg;
    uint32_t title_bar_text;
    uint32_t button_bg;
    uint32_t button_text;
    uint32_t textbox_bg;
    uint32_t textbox_border;
    uint32_t textbox_text;
    uint8_t glass_effect_strength; // 0-255, 0 for no glass, 255 for strong glass
} theme_colors_t;

// Initialize the theming engine
void theme_init(void);

// Load a theme from a file
int theme_load(const char* theme_name);

// Get current theme colors
const theme_colors_t* theme_get_colors(void);

// Set the theme mode (e.g., "light" or "dark")
void theme_set_mode(const char* mode);

// Apply glass effect to a window area (stub for now)
void theme_apply_glass_effect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

#endif // THEME_H
