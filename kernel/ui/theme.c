#include "theme.h"
#include "graphics.h" // For color definitions if needed, or define them here
#include "../string.h" // For strcmp

static theme_colors_t current_theme_colors;

// Define light theme colors
static const theme_colors_t light_theme = {
    .window_bg = 0xFFCCCCCC,       // Light gray
    .window_border = 0xFF888888,   // Darker gray
    .title_bar_bg = 0xFF0078D7,    // Windows blue
    .title_bar_text = 0xFFFFFFFF,  // White
    .button_bg = 0xFFDDDDDD,       // Lighter gray
    .button_text = 0xFF000000,     // Black
    .textbox_bg = 0xFFFFFFFF,      // White
    .textbox_border = 0xFF888888,  // Darker gray
    .textbox_text = 0xFF000000,    // Black
    .glass_effect_strength = 128
};

// Define dark theme colors
static const theme_colors_t dark_theme = {
    .window_bg = 0xFF333333,       // Dark gray
    .window_border = 0xFF555555,   // Lighter dark gray
    .title_bar_bg = 0xFF1A1A1A,    // Very dark gray
    .title_bar_text = 0xFFFFFFFF,  // White
    .button_bg = 0xFF555555,       // Medium dark gray
    .button_text = 0xFFFFFFFF,     // White
    .textbox_bg = 0xFF222222,      // Even darker gray
    .textbox_border = 0xFF555555,  // Medium dark gray
    .textbox_text = 0xFFFFFFFF,    // White
    .glass_effect_strength = 180   // Stronger glass effect for dark theme
};

void theme_init(void) {
    // Default to light theme on initialization
    current_theme_colors = light_theme;
}

// Placeholder for applying glassmorphism effect to a region
void theme_apply_glass_effect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (current_theme_colors.glass_effect_strength > 0) {
        // Apply a blur to the background region
        graphics_apply_blur(x, y, width, height, 5); // Using a fixed radius for now

        // Then, draw a translucent overlay to give the glass effect
        // The color of the overlay could be based on the window_bg or a specific glass color
        uint32_t glass_color = (current_theme_colors.window_bg & 0x00FFFFFF) | 
                               ((255 - current_theme_colors.glass_effect_strength) << 24); // Invert strength for alpha
        graphics_draw_rect(x, y, width, height, glass_color);
    }
}

int theme_load(const char* theme_name) {
    if (strcmp(theme_name, "dark") == 0) {
        current_theme_colors = dark_theme;
        return 0;
    } else if (strcmp(theme_name, "light") == 0) {
        current_theme_colors = light_theme;
        return 0;
    } else {
        // For now, if an unknown theme name is provided, default to light theme
        current_theme_colors = light_theme;
        return -1; // Indicate failure to load specific theme
    }
}

const theme_colors_t* theme_get_colors(void) {
    return &current_theme_colors;
}

void theme_set_mode(const char* mode) {
    if (strcmp(mode, "dark") == 0) {
        current_theme_colors = dark_theme;
    } else if (strcmp(mode, "light") == 0) {
        current_theme_colors = light_theme;
    } else {
        // Invalid mode, do nothing or log an error
    }
}