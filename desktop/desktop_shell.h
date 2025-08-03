/**
 * RaeenOS Revolutionary Desktop Shell
 * Beautiful, fluid desktop environment exceeding Windows 11/macOS Ventura
 */

#ifndef DESKTOP_SHELL_H
#define DESKTOP_SHELL_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "../drivers/input/input.h"

// Forward declarations
typedef struct RaeenCompositor RaeenCompositor;
typedef struct GraphicsContext GraphicsContext;
typedef struct RaeenUIContext RaeenUIContext;
typedef struct RaeenUIWindow RaeenUIWindow;
typedef struct RaeenUITheme RaeenUITheme;
typedef struct RaeenUIRect RaeenUIRect;
typedef struct RaeenUIColor RaeenUIColor;
typedef struct RaeenUIPoint RaeenUIPoint;

// Desktop shell structures
typedef struct DesktopShell DesktopShell;
typedef struct DesktopWidget DesktopWidget;
typedef struct DesktopWorkspace DesktopWorkspace;
typedef struct Notification Notification;

// Input event types
typedef enum {
    INPUT_EVENT_MOUSE_CLICK,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_KEY_PRESS,
    INPUT_EVENT_KEY_RELEASE,
    INPUT_EVENT_SCROLL,
    INPUT_EVENT_GESTURE,
    INPUT_EVENT_TOUCH
} InputEventType;

// Use the comprehensive gesture types from input driver
typedef input_gesture_type_t GestureType;

// Use the comprehensive key codes from input driver
typedef input_key_code_t KeyCode;

// Convenience aliases for desktop shell specific keys
#define DESKTOP_KEY_WINDOWS KEY_LEFTMETA
#define DESKTOP_KEY_SUPER KEY_LEFTMETA
#define DESKTOP_KEY_ALT KEY_LEFTALT
#define DESKTOP_KEY_CTRL KEY_LEFTCTRL
#define DESKTOP_KEY_SHIFT KEY_LEFTSHIFT

// Animation quality levels
typedef enum {
    ANIMATION_QUALITY_LOW,
    ANIMATION_QUALITY_MEDIUM,
    ANIMATION_QUALITY_HIGH,
    ANIMATION_QUALITY_ULTRA
} AnimationQuality;

// Performance modes
typedef enum {
    PERFORMANCE_MODE_POWER_SAVE,
    PERFORMANCE_MODE_BALANCED,
    PERFORMANCE_MODE_GAMING
} PerformanceMode;

// Input event structure
typedef struct {
    InputEventType type;
    uint64_t timestamp;
    uint32_t key_code;
    RaeenUIPoint position;
    int32_t scroll_delta;
    void* data; // Additional event-specific data
} InputEvent;

// Gesture event structure
typedef struct {
    GestureType type;
    RaeenUIPoint start_position;
    RaeenUIPoint current_position;
    RaeenUIPoint velocity;
    float scale;
    float rotation;
    uint64_t duration;
} GestureEvent;

// Desktop shell statistics
typedef struct {
    float average_fps;
    uint32_t workspace_count;
    uint32_t widget_count;
    uint32_t notification_count;
    bool glassmorphism_enabled;
    bool animations_enabled;
    bool ai_suggestions_enabled;
} DesktopShellStats;

// Function declarations

/**
 * Initialize the revolutionary desktop shell
 */
DesktopShell* desktop_shell_init(RaeenCompositor* compositor, GraphicsContext* graphics);

/**
 * Shutdown desktop shell
 */
void desktop_shell_shutdown(DesktopShell* shell);

/**
 * Update desktop shell (called every frame)
 */
void desktop_shell_update(DesktopShell* shell, float delta_time);

/**
 * Render desktop shell
 */
void desktop_shell_render(DesktopShell* shell);

/**
 * Handle input events
 */
void desktop_shell_handle_input(DesktopShell* shell, InputEvent* event);

/**
 * Switch to workspace
 */
void desktop_shell_switch_workspace(DesktopShell* shell, uint32_t workspace_id);

/**
 * Add desktop widget
 */
void desktop_shell_add_widget(DesktopShell* shell, const char* name, RaeenUIRect frame);

/**
 * Show notification
 */
void desktop_shell_show_notification(DesktopShell* shell, const char* title, const char* message, const char* app_name);

/**
 * Set theme
 */
void desktop_shell_set_theme(DesktopShell* shell, RaeenUITheme* theme);

/**
 * Enable/disable glassmorphism effects
 */
void desktop_shell_set_glassmorphism(DesktopShell* shell, bool enabled);

/**
 * Enable/disable animations
 */
void desktop_shell_set_animations(DesktopShell* shell, bool enabled, float speed_multiplier);

/**
 * Set animation quality
 */
void desktop_shell_set_animation_quality(DesktopShell* shell, AnimationQuality quality);

/**
 * Set performance mode
 */
void desktop_shell_set_performance_mode(DesktopShell* shell, PerformanceMode mode);

/**
 * Enable/disable AI suggestions
 */
void desktop_shell_set_ai_suggestions(DesktopShell* shell, bool enabled);

/**
 * Set UI scale factor for HiDPI displays
 */
void desktop_shell_set_ui_scale(DesktopShell* shell, float scale_factor);

/**
 * Set wallpaper for current workspace
 */
void desktop_shell_set_wallpaper(DesktopShell* shell, const char* path);

/**
 * Enable/disable dynamic wallpaper
 */
void desktop_shell_set_dynamic_wallpaper(DesktopShell* shell, bool enabled);

/**
 * Create new workspace
 */
uint32_t desktop_shell_create_workspace(DesktopShell* shell, const char* name);

/**
 * Delete workspace
 */
void desktop_shell_delete_workspace(DesktopShell* shell, uint32_t workspace_id);

/**
 * Get current workspace ID
 */
uint32_t desktop_shell_get_current_workspace(DesktopShell* shell);

/**
 * Show/hide start menu
 */
void desktop_shell_toggle_start_menu(DesktopShell* shell);

/**
 * Show/hide notification center
 */
void desktop_shell_toggle_notification_center(DesktopShell* shell);

/**
 * Clear all notifications
 */
void desktop_shell_clear_notifications(DesktopShell* shell);

/**
 * Set adaptive brightness
 */
void desktop_shell_set_adaptive_brightness(DesktopShell* shell, float brightness);

/**
 * Set adaptive accent color
 */
void desktop_shell_set_adaptive_accent(DesktopShell* shell, RaeenUIColor color);

/**
 * Enable/disable gesture recognition
 */
void desktop_shell_set_gesture_recognition(DesktopShell* shell, bool enabled);

/**
 * Get desktop shell performance statistics
 */
void desktop_shell_get_stats(DesktopShell* shell, DesktopShellStats* stats);

/**
 * Take screenshot of current desktop
 */
bool desktop_shell_take_screenshot(DesktopShell* shell, const char* filename);

/**
 * Enter presentation mode (disable notifications, effects)
 */
void desktop_shell_enter_presentation_mode(DesktopShell* shell);

/**
 * Exit presentation mode
 */
void desktop_shell_exit_presentation_mode(DesktopShell* shell);

/**
 * Enter gaming mode (optimize for performance and low latency)
 */
void desktop_shell_enter_gaming_mode(DesktopShell* shell);

/**
 * Exit gaming mode
 */
void desktop_shell_exit_gaming_mode(DesktopShell* shell);

/**
 * Set focus mode (minimize distractions)
 */
void desktop_shell_set_focus_mode(DesktopShell* shell, bool enabled);

/**
 * Apply automatic color theme based on wallpaper
 */
void desktop_shell_auto_theme_from_wallpaper(DesktopShell* shell);

/**
 * Set time-based theme (day/night themes)
 */
void desktop_shell_set_time_based_theme(DesktopShell* shell, bool enabled);

#endif // DESKTOP_SHELL_H