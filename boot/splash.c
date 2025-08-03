/**
 * RaeenOS Boot Splash System Implementation
 * High-resolution animated splash screen for boot sequence
 */

#include "splash.h"
#include "../kernel/memory.h"
#include "string.h"
#include "../kernel/timer.h"
#include "../gpu/graphics_pipeline.h"

// Global splash state
static splash_config_t g_splash_config = {0};
static boot_progress_t g_boot_progress = {0};
static bool g_splash_active = false;
static uint32_t g_animation_frame = 0;
static uint32_t g_last_frame_time = 0;

// Default boot state messages
const char* SPLASH_MESSAGES[] = {
    "Initializing system...",
    "Loading kernel...",
    "Starting drivers...",
    "Mounting filesystem...",
    "Configuring network...",
    "Initializing graphics...",
    "Starting user services...",
    "Loading desktop...",
    "System ready"
};

// Default splash configurations
const splash_config_t SPLASH_CONFIG_DEFAULT = {
    .screen_width = 1920,
    .screen_height = 1080,
    .bpp = 32,
    .framebuffer = NULL,
    
    .logo_x = 860,
    .logo_y = 400,
    .logo_width = 200,
    .logo_height = 80,
    
    .progress_x = 760,
    .progress_y = 600,
    .progress_width = 400,
    .progress_height = 8,
    
    .animation_type = BOOT_ANIM_PROGRESS_BAR,
    .animation_speed = 60,
    
    .background_color = SPLASH_COLOR_RAEEN_BG,
    .logo_color = SPLASH_COLOR_RAEEN_PRIMARY,
    .progress_color = SPLASH_COLOR_RAEEN_ACCENT,
    .text_color = SPLASH_COLOR_WHITE,
    
    .boot_message = "RaeenOS",
    .text_x = 860,
    .text_y = 520,
    
    .fade_duration_ms = 500,
    .min_display_time_ms = SPLASH_MIN_BOOT_TIME_MS
};

const splash_config_t SPLASH_CONFIG_MINIMAL = {
    .screen_width = 800,
    .screen_height = 600,
    .bpp = 32,
    .framebuffer = NULL,
    .background_color = SPLASH_COLOR_BLACK,
    .logo_color = SPLASH_COLOR_WHITE,
    .progress_color = SPLASH_COLOR_LIGHT_GREY,
    .text_color = SPLASH_COLOR_WHITE,
    .animation_type = BOOT_ANIM_DOTS,
    .boot_message = "Loading...",
    .fade_duration_ms = 100,
    .min_display_time_ms = 1000
};

const splash_config_t SPLASH_CONFIG_VERBOSE = {
    .screen_width = 1024,
    .screen_height = 768,
    .bpp = 32,
    .framebuffer = NULL,
    .background_color = SPLASH_COLOR_BLACK,
    .logo_color = SPLASH_COLOR_GREEN,
    .progress_color = SPLASH_COLOR_GREEN,
    .text_color = SPLASH_COLOR_LIGHT_GREY,
    .animation_type = BOOT_ANIM_PROGRESS_BAR,
    .boot_message = "Verbose Boot Mode",
    .fade_duration_ms = 0,
    .min_display_time_ms = 0
};

const splash_config_t SPLASH_CONFIG_RECOVERY = {
    .screen_width = 1024,
    .screen_height = 768,
    .bpp = 32,
    .framebuffer = NULL,
    .background_color = SPLASH_COLOR_DARK_GREY,
    .logo_color = SPLASH_COLOR_RED,
    .progress_color = SPLASH_COLOR_RED,
    .text_color = SPLASH_COLOR_WHITE,
    .animation_type = BOOT_ANIM_PULSE,
    .boot_message = "Recovery Mode",
    .fade_duration_ms = 200,
    .min_display_time_ms = 5000
};

/**
 * Initialize splash screen system
 */
bool splash_init(splash_config_t* config) {
    if (g_splash_active) {
        return true;
    }
    
    // Use provided config or default
    if (config) {
        memcpy(&g_splash_config, config, sizeof(splash_config_t));
    } else {
        memcpy(&g_splash_config, &SPLASH_CONFIG_DEFAULT, sizeof(splash_config_t));
    }
    
    // Initialize framebuffer if not provided
    if (!g_splash_config.framebuffer) {
        // Try to get framebuffer from graphics system
        graphics_device_t* gfx = graphics_get_device();
        if (gfx && gfx->framebuffer) {
            g_splash_config.framebuffer = gfx->framebuffer;
            g_splash_config.screen_width = gfx->width;
            g_splash_config.screen_height = gfx->height;
        } else {
            // Fallback to basic VGA framebuffer
            g_splash_config.framebuffer = (void*)0xA0000;
            g_splash_config.screen_width = 640;
            g_splash_config.screen_height = 480;
            g_splash_config.bpp = 8;
        }
    }
    
    // Initialize boot progress
    memset(&g_boot_progress, 0, sizeof(boot_progress_t));
    g_boot_progress.current_state = SPLASH_STATE_INIT;
    g_boot_progress.start_time = splash_get_time_ms();
    g_boot_progress.verbose_mode = splash_detect_verbose_mode();
    g_boot_progress.safe_mode = splash_detect_safe_mode();
    
    g_splash_active = true;
    g_animation_frame = 0;
    g_last_frame_time = splash_get_time_ms();
    
    // Clear screen and show initial splash
    splash_clear_screen();
    splash_set_state(SPLASH_STATE_INIT, SPLASH_MESSAGES[SPLASH_STATE_INIT]);
    
    return true;
}

/**
 * Shutdown splash screen
 */
void splash_shutdown(void) {
    if (!g_splash_active) return;
    
    // Fade out effect
    splash_fade_out(g_splash_config.fade_duration_ms);
    
    g_splash_active = false;
}

/**
 * Check if splash is active
 */
bool splash_is_active(void) {
    return g_splash_active;
}

/**
 * Set boot state and update display
 */
void splash_set_state(splash_state_t state, const char* message) {
    if (!g_splash_active) return;
    
    g_boot_progress.current_state = state;
    g_boot_progress.state_times[state] = splash_get_time_ms();
    
    if (message) {
        g_boot_progress.current_message = message;
    } else if (state <= SPLASH_STATE_COMPLETE) {
        g_boot_progress.current_message = SPLASH_MESSAGES[state];
    }
    
    // Calculate progress percentage
    uint32_t total_states = SPLASH_STATE_COMPLETE;
    g_boot_progress.progress_percent = (state * 100) / total_states;
    
    // Update display if not in verbose mode
    if (!g_boot_progress.verbose_mode) {
        splash_render_frame();
    }
}

/**
 * Set progress percentage
 */
void splash_set_progress(uint32_t percent) {
    if (!g_splash_active) return;
    
    if (percent > 100) percent = 100;
    g_boot_progress.progress_percent = percent;
    
    if (!g_boot_progress.verbose_mode) {
        splash_render_frame();
    }
}

/**
 * Update boot message
 */
void splash_update_message(const char* message) {
    if (!g_splash_active || !message) return;
    
    g_boot_progress.current_message = message;
    
    if (!g_boot_progress.verbose_mode) {
        splash_render_frame();
    }
}

/**
 * Render complete splash frame
 */
void splash_render_frame(void) {
    if (!g_splash_active) return;
    
    // Clear screen
    splash_clear_screen();
    
    // Draw logo
    splash_draw_logo();
    
    // Draw progress bar
    splash_draw_progress_bar();
    
    // Draw current message
    if (g_boot_progress.current_message) {
        splash_draw_text(g_boot_progress.current_message, 
                        g_splash_config.text_x, 
                        g_splash_config.text_y);
    }
    
    // Draw animation
    splash_draw_animation();
    
    // Update animation frame
    splash_update_animation();
}

/**
 * Clear entire screen
 */
void splash_clear_screen(void) {
    if (!g_splash_active || !g_splash_config.framebuffer) return;
    
    splash_fill_rect(0, 0, 
                    g_splash_config.screen_width, 
                    g_splash_config.screen_height, 
                    g_splash_config.background_color);
}

/**
 * Draw RaeenOS logo
 */
void splash_draw_logo(void) {
    if (!g_splash_active) return;
    
    // Draw simple logo rectangle for now
    // In production, this would render the actual RaeenOS logo
    uint32_t x = g_splash_config.logo_x;
    uint32_t y = g_splash_config.logo_y;
    uint32_t w = g_splash_config.logo_width;
    uint32_t h = g_splash_config.logo_height;
    
    // Logo background
    splash_fill_rect(x, y, w, h, g_splash_config.logo_color);
    
    // Logo text "RaeenOS"
    splash_draw_text("RaeenOS", x + 20, y + 25);
}

/**
 * Draw progress bar
 */
void splash_draw_progress_bar(void) {
    if (!g_splash_active) return;
    
    uint32_t x = g_splash_config.progress_x;
    uint32_t y = g_splash_config.progress_y;
    uint32_t w = g_splash_config.progress_width;
    uint32_t h = g_splash_config.progress_height;
    
    // Progress bar background
    splash_draw_rect(x, y, w, h, SPLASH_COLOR_WHITE);
    
    // Progress bar fill
    uint32_t fill_width = (w * g_boot_progress.progress_percent) / 100;
    if (fill_width > 0) {
        splash_fill_rect(x + 1, y + 1, fill_width - 2, h - 2, 
                        g_splash_config.progress_color);
    }
    
    // Progress percentage text
    char progress_text[16];
    sprintf(progress_text, "%u%%", 
                 g_boot_progress.progress_percent);
    splash_draw_text(progress_text, x + w + 10, y - 5);
}

/**
 * Draw text at specified position
 */
void splash_draw_text(const char* text, uint32_t x, uint32_t y) {
    if (!g_splash_active || !text) return;
    
    // Simple bitmap font rendering
    // In production, this would use proper font rendering
    uint32_t char_width = 8;
    uint32_t char_height = 16;
    
    for (uint32_t i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        uint32_t char_x = x + (i * char_width);
        
        // Draw simple character representation
        splash_fill_rect(char_x, y, char_width - 1, char_height, 
                        g_splash_config.text_color);
    }
}

/**
 * Draw boot animation
 */
void splash_draw_animation(void) {
    if (!g_splash_active) return;
    
    switch (g_splash_config.animation_type) {
        case BOOT_ANIM_SPINNER:
            splash_draw_spinner();
            break;
        case BOOT_ANIM_DOTS:
            splash_draw_dots();
            break;
        case BOOT_ANIM_PULSE:
            splash_draw_pulse();
            break;
        default:
            break;
    }
}

/**
 * Draw spinning animation
 */
static void splash_draw_spinner(void) {
    uint32_t center_x = g_splash_config.screen_width - 100;
    uint32_t center_y = g_splash_config.screen_height - 100;
    uint32_t radius = 20;
    
    // Draw spinner based on animation frame
    uint32_t angle = (g_animation_frame * 10) % 360;
    
    for (uint32_t i = 0; i < 8; i++) {
        uint32_t spoke_angle = (angle + i * 45) % 360;
        uint32_t alpha = 255 - (i * 32);
        
        // Calculate spoke end position (simplified)
        uint32_t end_x = center_x + (radius * (spoke_angle % 180)) / 180;
        uint32_t end_y = center_y + (radius * (spoke_angle % 180)) / 180;
        
        uint32_t color = splash_argb_to_color(alpha, 255, 255, 255);
        splash_fill_rect(end_x - 2, end_y - 2, 4, 4, color);
    }
}

/**
 * Draw dots animation
 */
static void splash_draw_dots(void) {
    uint32_t base_x = g_splash_config.screen_width - 120;
    uint32_t base_y = g_splash_config.screen_height - 50;
    
    for (uint32_t i = 0; i < 3; i++) {
        uint32_t dot_x = base_x + (i * 20);
        uint32_t alpha = ((g_animation_frame + i * 20) % 60 < 30) ? 255 : 100;
        uint32_t color = splash_argb_to_color(alpha, 255, 255, 255);
        
        splash_fill_rect(dot_x, base_y, 8, 8, color);
    }
}

/**
 * Update animation frame
 */
void splash_update_animation(void) {
    if (!g_splash_active) return;
    
    uint32_t current_time = splash_get_time_ms();
    uint32_t frame_time = 1000 / SPLASH_ANIMATION_FPS;
    
    if (current_time - g_last_frame_time >= frame_time) {
        g_animation_frame++;
        g_last_frame_time = current_time;
    }
}

/**
 * Set pixel in framebuffer
 */
void splash_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!g_splash_active || !g_splash_config.framebuffer) return;
    if (x >= g_splash_config.screen_width || y >= g_splash_config.screen_height) return;
    
    if (g_splash_config.bpp == 32) {
        uint32_t* fb = (uint32_t*)g_splash_config.framebuffer;
        fb[y * g_splash_config.screen_width + x] = color;
    } else if (g_splash_config.bpp == 24) {
        uint8_t* fb = (uint8_t*)g_splash_config.framebuffer;
        uint32_t offset = (y * g_splash_config.screen_width + x) * 3;
        fb[offset] = color & 0xFF;
        fb[offset + 1] = (color >> 8) & 0xFF;
        fb[offset + 2] = (color >> 16) & 0xFF;
    }
}

/**
 * Fill rectangle with color
 */
void splash_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!g_splash_active) return;
    
    for (uint32_t py = y; py < y + height; py++) {
        for (uint32_t px = x; px < x + width; px++) {
            splash_set_pixel(px, py, color);
        }
    }
}

/**
 * Draw rectangle outline
 */
void splash_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!g_splash_active) return;
    
    // Top and bottom lines
    for (uint32_t px = x; px < x + width; px++) {
        splash_set_pixel(px, y, color);
        splash_set_pixel(px, y + height - 1, color);
    }
    
    // Left and right lines
    for (uint32_t py = y; py < y + height; py++) {
        splash_set_pixel(x, py, color);
        splash_set_pixel(x + width - 1, py, color);
    }
}

/**
 * Convert RGB to color value
 */
uint32_t splash_rgb_to_color(uint8_t r, uint8_t g, uint8_t b) {
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

/**
 * Convert ARGB to color value
 */
uint32_t splash_argb_to_color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/**
 * Detect boot modes from kernel command line or key presses
 */
bool splash_detect_verbose_mode(void) {
    // Check for Shift key held during boot
    // In production, this would check actual keyboard state
    return false;
}

bool splash_detect_safe_mode(void) {
    // Check for F9 key or safe mode flag
    return false;
}

bool splash_detect_recovery_mode(void) {
    // Check for F10 key or recovery flag
    return false;
}

/**
 * Get current time in milliseconds
 */
uint32_t splash_get_time_ms(void) {
    // Use timer system to get current time
    return timer_get_ticks() * (1000 / TIMER_FREQUENCY);
}

/**
 * Show error message on splash screen
 */
void splash_show_error(const char* error_message) {
    if (!g_splash_active || !error_message) return;
    
    // Clear screen with red background
    splash_fill_rect(0, 0, g_splash_config.screen_width, g_splash_config.screen_height, 
                    SPLASH_COLOR_RED);
    
    // Draw error message
    splash_draw_text("BOOT ERROR", g_splash_config.screen_width / 2 - 50, 
                    g_splash_config.screen_height / 2 - 50);
    splash_draw_text(error_message, g_splash_config.screen_width / 2 - 100, 
                    g_splash_config.screen_height / 2);
}

// Getter functions
splash_state_t splash_get_state(void) {
    return g_boot_progress.current_state;
}

uint32_t splash_get_progress(void) {
    return g_boot_progress.progress_percent;
}

// Placeholder implementations for missing functions
static void splash_draw_pulse(void) {
    // Implement pulse animation
    debug_print("Splash: Pulse animation (placeholder).\n");
}

static void splash_fade_in(uint32_t duration_ms) {
    // Implement fade-in effect
    debug_print("Splash: Fade-in (placeholder).\n");
}

static void splash_fade_out(uint32_t duration_ms) {
    // Implement fade-out effect
    debug_print("Splash: Fade-out (placeholder).\n");
}

void splash_pulse_effect(uint32_t intensity) {
    debug_print("Splash: Pulse effect (placeholder).\n");
}

void splash_glow_effect(uint32_t intensity) {
    debug_print("Splash: Glow effect (placeholder).\n");
}

void splash_load_config(void) {
    debug_print("Splash: Loading config (placeholder).\n");
}

void splash_save_config(splash_config_t* config) {
    debug_print("Splash: Saving config (placeholder).\n");
}

void splash_set_theme(const char* theme_name) {
    debug_print("Splash: Setting theme ");
    debug_print(theme_name);
    debug_print(" (placeholder).\n");
}

void splash_show_warning(const char* warning_message) {
    debug_print("Splash: Warning: ");
    debug_print(warning_message);
    debug_print("\n");
}

void splash_show_panic(const char* panic_message) {
    debug_print("Splash: PANIC: ");
    debug_print(panic_message);
    debug_print("\n");
}

void splash_delay_ms(uint32_t ms) {
    // Simple busy-wait delay for now
    uint32_t start_time = splash_get_time_ms();
    while (splash_get_time_ms() - start_time < ms);
}

void splash_color_to_rgb(uint32_t color, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (r) *r = (color >> 16) & 0xFF;
    if (g) *g = (color >> 8) & 0xFF;
    if (b) *b = color & 0xFF;
}

uint32_t splash_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha) {
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

    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}