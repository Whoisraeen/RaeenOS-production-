/**
 * RaeenOS Boot Splash System
 * High-resolution animated splash screen for boot sequence
 */

#ifndef BOOT_SPLASH_H
#define BOOT_SPLASH_H

#include <stdint.h>
#include <stdbool.h>

// Splash screen states
typedef enum {
    SPLASH_STATE_INIT = 0,
    SPLASH_STATE_KERNEL_LOAD,
    SPLASH_STATE_DRIVERS,
    SPLASH_STATE_FILESYSTEM,
    SPLASH_STATE_NETWORK,
    SPLASH_STATE_GRAPHICS,
    SPLASH_STATE_USERSPACE,
    SPLASH_STATE_DESKTOP,
    SPLASH_STATE_COMPLETE
} splash_state_t;

// Boot animation types
typedef enum {
    BOOT_ANIM_SPINNER = 0,
    BOOT_ANIM_PROGRESS_BAR,
    BOOT_ANIM_DOTS,
    BOOT_ANIM_PULSE,
    BOOT_ANIM_CUSTOM
} boot_animation_t;

// Splash configuration
typedef struct {
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t bpp;
    void* framebuffer;
    
    // Logo settings
    uint32_t logo_x;
    uint32_t logo_y;
    uint32_t logo_width;
    uint32_t logo_height;
    
    // Progress bar settings
    uint32_t progress_x;
    uint32_t progress_y;
    uint32_t progress_width;
    uint32_t progress_height;
    
    // Animation settings
    boot_animation_t animation_type;
    uint32_t animation_speed;
    
    // Colors (ARGB)
    uint32_t background_color;
    uint32_t logo_color;
    uint32_t progress_color;
    uint32_t text_color;
    
    // Text settings
    const char* boot_message;
    uint32_t text_x;
    uint32_t text_y;
    
    // Timing
    uint32_t fade_duration_ms;
    uint32_t min_display_time_ms;
} splash_config_t;

// Boot progress tracking
typedef struct {
    splash_state_t current_state;
    uint32_t progress_percent;
    const char* current_message;
    uint32_t start_time;
    uint32_t state_times[SPLASH_STATE_COMPLETE + 1];
    bool verbose_mode;
    bool safe_mode;
} boot_progress_t;

// Function prototypes

// Core splash functions
bool splash_init(splash_config_t* config);
void splash_shutdown(void);
bool splash_is_active(void);

// Progress management
void splash_set_state(splash_state_t state, const char* message);
void splash_set_progress(uint32_t percent);
void splash_update_message(const char* message);
splash_state_t splash_get_state(void);
uint32_t splash_get_progress(void);

// Animation control
void splash_start_animation(boot_animation_t type);
void splash_stop_animation(void);
void splash_update_animation(void);

// Display functions
void splash_render_frame(void);
void splash_clear_screen(void);
void splash_draw_logo(void);
void splash_draw_progress_bar(void);
void splash_draw_text(const char* text, uint32_t x, uint32_t y);
void splash_draw_animation(void);

// Framebuffer operations
void splash_set_pixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t splash_get_pixel(uint32_t x, uint32_t y);
void splash_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void splash_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

// Effects and transitions
void splash_fade_in(uint32_t duration_ms);
void splash_fade_out(uint32_t duration_ms);
void splash_pulse_effect(uint32_t duration_ms);
void splash_glow_effect(uint32_t intensity);

// Boot mode detection
bool splash_detect_safe_mode(void);
bool splash_detect_verbose_mode(void);
bool splash_detect_recovery_mode(void);

// Error handling
void splash_show_error(const char* error_message);
void splash_show_warning(const char* warning_message);
void splash_show_panic(const char* panic_message);

// Configuration
void splash_load_config(void);
void splash_save_config(splash_config_t* config);
void splash_set_theme(const char* theme_name);

// Utility functions
uint32_t splash_rgb_to_color(uint8_t r, uint8_t g, uint8_t b);
uint32_t splash_argb_to_color(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
void splash_color_to_rgb(uint32_t color, uint8_t* r, uint8_t* g, uint8_t* b);
uint32_t splash_blend_colors(uint32_t color1, uint32_t color2, uint8_t alpha);

// Timer functions
uint32_t splash_get_time_ms(void);
void splash_delay_ms(uint32_t ms);

// Default configurations
extern const splash_config_t SPLASH_CONFIG_DEFAULT;
extern const splash_config_t SPLASH_CONFIG_MINIMAL;
extern const splash_config_t SPLASH_CONFIG_VERBOSE;
extern const splash_config_t SPLASH_CONFIG_RECOVERY;

// Boot state messages
extern const char* SPLASH_MESSAGES[];

// Constants
#define SPLASH_MAX_MESSAGE_LENGTH   256
#define SPLASH_MAX_THEME_NAME       64
#define SPLASH_ANIMATION_FPS        60
#define SPLASH_MIN_BOOT_TIME_MS     2000
#define SPLASH_FADE_STEPS           32

// Color constants (ARGB format)
#define SPLASH_COLOR_BLACK          0xFF000000
#define SPLASH_COLOR_WHITE          0xFFFFFFFF
#define SPLASH_COLOR_BLUE           0xFF0078D4
#define SPLASH_COLOR_GREEN          0xFF107C10
#define SPLASH_COLOR_RED            0xFFD13438
#define SPLASH_COLOR_ORANGE         0xFFFF8C00
#define SPLASH_COLOR_PURPLE         0xFF881798
#define SPLASH_COLOR_TRANSPARENT    0x00000000

// RaeenOS brand colors
#define SPLASH_COLOR_RAEEN_PRIMARY  0xFF6B46C1  // Purple
#define SPLASH_COLOR_RAEEN_ACCENT   0xFF8B5CF6  // Light purple
#define SPLASH_COLOR_RAEEN_DARK     0xFF4C1D95  // Dark purple
#define SPLASH_COLOR_RAEEN_BG       0xFF1F2937  // Dark gray

#endif // BOOT_SPLASH_H
