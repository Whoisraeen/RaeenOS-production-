/**
 * RaeenUI - Modern GPU-Accelerated UI Framework for RaeenOS
 * 
 * Inspired by SwiftUI, Flutter, React, and modern design languages
 * Features: Declarative UI, GPU acceleration, theming, AI integration
 * 
 * @author RaeenOS UI Team
 * @version 1.0
 */

#ifndef RAEENUI_H
#define RAEENUI_H

#include <stdint.h>
#include <stdbool.h>
#include "../gpu/graphics_pipeline.h"

// Forward declarations
typedef struct RaeenUIContext RaeenUIContext;
typedef struct RaeenUIWindow RaeenUIWindow;
typedef struct RaeenUIView RaeenUIView;
typedef struct RaeenUITheme RaeenUITheme;
typedef struct RaeenUIAnimation RaeenUIAnimation;

// UI Framework Version
#define RAEENUI_VERSION_MAJOR 1
#define RAEENUI_VERSION_MINOR 0
#define RAEENUI_VERSION_PATCH 0

// Maximum limits
#define RAEENUI_MAX_WINDOWS 64
#define RAEENUI_MAX_VIEWS_PER_WINDOW 1024
#define RAEENUI_MAX_THEMES 16
#define RAEENUI_MAX_ANIMATIONS 256
#define RAEENUI_MAX_SHADER_EFFECTS 32

// Color and styling
typedef struct {
    float r, g, b, a; // RGBA values 0.0-1.0
} RaeenUIColor;

typedef struct {
    float x, y;
} RaeenUIPoint;

typedef struct {
    float width, height;
} RaeenUISize;

typedef struct {
    RaeenUIPoint origin;
    RaeenUISize size;
} RaeenUIRect;

typedef struct {
    float top, left, bottom, right;
} RaeenUIEdgeInsets;

// UI Layout and positioning
typedef enum {
    RAEENUI_LAYOUT_STACK_VERTICAL = 0,
    RAEENUI_LAYOUT_STACK_HORIZONTAL,
    RAEENUI_LAYOUT_GRID,
    RAEENUI_LAYOUT_FLOW,
    RAEENUI_LAYOUT_ABSOLUTE,
    RAEENUI_LAYOUT_FLEX
} RaeenUILayoutType;

typedef enum {
    RAEENUI_ALIGN_START = 0,
    RAEENUI_ALIGN_CENTER,
    RAEENUI_ALIGN_END,
    RAEENUI_ALIGN_STRETCH,
    RAEENUI_ALIGN_SPACE_BETWEEN,
    RAEENUI_ALIGN_SPACE_AROUND,
    RAEENUI_ALIGN_SPACE_EVENLY
} RaeenUIAlignment;

// View types and properties
typedef enum {
    RAEENUI_VIEW_CONTAINER = 0,
    RAEENUI_VIEW_TEXT,
    RAEENUI_VIEW_BUTTON,
    RAEENUI_VIEW_IMAGE,
    RAEENUI_VIEW_INPUT,
    RAEENUI_VIEW_SCROLL,
    RAEENUI_VIEW_CANVAS,
    RAEENUI_VIEW_AI_CHAT,
    RAEENUI_VIEW_CUSTOM
} RaeenUIViewType;

typedef enum {
    RAEENUI_EVENT_CLICK = 0,
    RAEENUI_EVENT_HOVER,
    RAEENUI_EVENT_FOCUS,
    RAEENUI_EVENT_BLUR,
    RAEENUI_EVENT_KEY_DOWN,
    RAEENUI_EVENT_KEY_UP,
    RAEENUI_EVENT_DRAG_START,
    RAEENUI_EVENT_DRAG_END,
    RAEENUI_EVENT_RESIZE,
    RAEENUI_EVENT_AI_RESPONSE
} RaeenUIEventType;

// Animation and effects
typedef enum {
    RAEENUI_ANIM_LINEAR = 0,
    RAEENUI_ANIM_EASE_IN,
    RAEENUI_ANIM_EASE_OUT,
    RAEENUI_ANIM_EASE_IN_OUT,
    RAEENUI_ANIM_BOUNCE,
    RAEENUI_ANIM_SPRING
} RaeenUIAnimationCurve;

typedef enum {
    RAEENUI_EFFECT_BLUR = 0,
    RAEENUI_EFFECT_SHADOW,
    RAEENUI_EFFECT_GLOW,
    RAEENUI_EFFECT_GRADIENT,
    RAEENUI_EFFECT_ROUNDED_CORNERS,
    RAEENUI_EFFECT_TRANSPARENCY,
    RAEENUI_EFFECT_CUSTOM_SHADER
} RaeenUIEffectType;

// Theme system
typedef enum {
    RAEENUI_THEME_LIGHT = 0,
    RAEENUI_THEME_DARK,
    RAEENUI_THEME_AUTO,
    RAEENUI_THEME_CUSTOM
} RaeenUIThemeMode;

// Window management
typedef enum {
    RAEENUI_WINDOW_NORMAL = 0,
    RAEENUI_WINDOW_FULLSCREEN,
    RAEENUI_WINDOW_MINIMIZED,
    RAEENUI_WINDOW_MAXIMIZED,
    RAEENUI_WINDOW_FLOATING,
    RAEENUI_WINDOW_TILED
} RaeenUIWindowState;

// Event handling
typedef struct RaeenUIEvent {
    RaeenUIEventType type;
    RaeenUIView* target;
    RaeenUIPoint position;
    uint32_t timestamp;
    uint32_t key_code;
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    void* custom_data;
} RaeenUIEvent;

typedef bool (*RaeenUIEventHandler)(RaeenUIView* view, RaeenUIEvent* event);

// Style properties
typedef struct {
    RaeenUIColor background_color;
    RaeenUIColor foreground_color;
    RaeenUIColor border_color;
    float border_width;
    float corner_radius;
    RaeenUIEdgeInsets padding;
    RaeenUIEdgeInsets margin;
    float opacity;
    bool visible;
    
    // Typography
    char font_family[64];
    float font_size;
    bool font_bold;
    bool font_italic;
    
    // Effects
    float blur_radius;
    float shadow_offset_x;
    float shadow_offset_y;
    float shadow_blur;
    RaeenUIColor shadow_color;
    
    // Custom shader
    uint32_t shader_id;
    void* shader_params;
} RaeenUIStyle;

// Layout properties
typedef struct {
    RaeenUILayoutType type;
    RaeenUIAlignment horizontal_alignment;
    RaeenUIAlignment vertical_alignment;
    float spacing;
    RaeenUISize min_size;
    RaeenUISize max_size;
    RaeenUISize preferred_size;
    float flex_grow;
    float flex_shrink;
    int grid_column;
    int grid_row;
    int grid_column_span;
    int grid_row_span;
} RaeenUILayout;

// Animation properties
struct RaeenUIAnimation {
    uint32_t animation_id;
    RaeenUIView* target_view;
    RaeenUIAnimationCurve curve;
    float duration;
    float delay;
    bool repeat;
    bool auto_reverse;
    
    // Animation properties
    RaeenUIRect from_frame;
    RaeenUIRect to_frame;
    float from_opacity;
    float to_opacity;
    RaeenUIColor from_color;
    RaeenUIColor to_color;
    
    // Callbacks
    void (*on_start)(RaeenUIAnimation* anim);
    void (*on_update)(RaeenUIAnimation* anim, float progress);
    void (*on_complete)(RaeenUIAnimation* anim);
    
    // Internal state
    float current_time;
    bool is_running;
    bool is_paused;
    
    struct RaeenUIAnimation* next;
};

// Theme definition
struct RaeenUITheme {
    char name[64];
    RaeenUIThemeMode mode;
    
    // Color palette
    RaeenUIColor primary;
    RaeenUIColor secondary;
    RaeenUIColor accent;
    RaeenUIColor background;
    RaeenUIColor surface;
    RaeenUIColor error;
    RaeenUIColor warning;
    RaeenUIColor success;
    RaeenUIColor text_primary;
    RaeenUIColor text_secondary;
    
    // Typography
    char primary_font[64];
    char secondary_font[64];
    float base_font_size;
    
    // Spacing and sizing
    float base_spacing;
    float base_corner_radius;
    float base_border_width;
    
    // Effects
    float default_blur_radius;
    float default_shadow_offset;
    float default_shadow_blur;
    RaeenUIColor default_shadow_color;
    
    // Custom properties
    void* custom_data;
    uint32_t custom_data_size;
};

// View definition
struct RaeenUIView {
    uint32_t view_id;
    RaeenUIViewType type;
    char identifier[64];
    
    // Hierarchy
    RaeenUIView* parent;
    RaeenUIView* first_child;
    RaeenUIView* next_sibling;
    uint32_t child_count;
    
    // Layout and styling
    RaeenUIRect frame;
    RaeenUILayout layout;
    RaeenUIStyle style;
    
    // State
    bool needs_layout;
    bool needs_render;
    bool is_focused;
    bool is_hovered;
    bool is_pressed;
    bool is_enabled;
    
    // Content
    char* text_content;
    void* image_data;
    uint32_t image_width;
    uint32_t image_height;
    
    // Event handling
    RaeenUIEventHandler event_handlers[16];
    
    // AI integration
    bool ai_enabled;
    char ai_context[256];
    void* ai_data;
    
    // Custom rendering
    void (*custom_render)(RaeenUIView* view, GraphicsContext* gfx);
    void* custom_data;
    
    // GPU resources
    uint32_t vertex_buffer_id;
    uint32_t texture_id;
    uint32_t shader_program_id;
    
    // Accessibility
    char accessibility_label[128];
    char accessibility_hint[256];
    bool accessibility_enabled;
};

// Window definition
struct RaeenUIWindow {
    uint32_t window_id;
    char title[128];
    RaeenUIRect frame;
    RaeenUIWindowState state;
    
    // Content
    RaeenUIView* root_view;
    RaeenUIView* focused_view;
    
    // Properties
    bool resizable;
    bool closable;
    bool minimizable;
    bool maximizable;
    bool always_on_top;
    float opacity;
    
    // Theme
    RaeenUITheme* theme;
    
    // GPU context
    GraphicsContext* graphics_context;
    uint32_t framebuffer_id;
    
    // Event handling
    RaeenUIEventHandler global_event_handler;
    
    // AI integration
    bool ai_window;
    void* ai_assistant;
    
    // Internal state
    bool needs_redraw;
    uint64_t last_render_time;
    
    struct RaeenUIWindow* next;
};

// Main UI context
struct RaeenUIContext {
    // System integration
    GraphicsPipelineState* graphics_pipeline;
    void* window_manager;
    
    // Windows and views
    RaeenUIWindow* windows;
    uint32_t window_count;
    RaeenUIWindow* active_window;
    
    // Themes
    RaeenUITheme* themes[RAEENUI_MAX_THEMES];
    uint32_t theme_count;
    RaeenUITheme* current_theme;
    
    // Animations
    RaeenUIAnimation* active_animations;
    uint32_t animation_count;
    
    // Input state
    RaeenUIPoint mouse_position;
    bool mouse_buttons[8];
    bool keys[256];
    
    // Performance
    uint64_t frame_count;
    float fps;
    uint64_t last_frame_time;
    
    // Configuration
    bool vsync_enabled;
    bool gpu_acceleration;
    bool high_dpi_support;
    float ui_scale_factor;
    
    // AI integration
    void* ai_engine;
    bool ai_enabled;
    
    // Debug and profiling
    bool debug_mode;
    bool show_fps;
    bool show_layout_bounds;
    bool wireframe_mode;
};

// Core API functions
RaeenUIContext* raeenui_create_context(GraphicsPipelineState* graphics);
void raeenui_destroy_context(RaeenUIContext* context);
bool raeenui_initialize(RaeenUIContext* context);
void raeenui_shutdown(RaeenUIContext* context);

// Window management
RaeenUIWindow* raeenui_create_window(RaeenUIContext* context, const char* title, RaeenUIRect frame);
void raeenui_destroy_window(RaeenUIWindow* window);
void raeenui_show_window(RaeenUIWindow* window);
void raeenui_hide_window(RaeenUIWindow* window);
void raeenui_set_window_frame(RaeenUIWindow* window, RaeenUIRect frame);
void raeenui_set_window_state(RaeenUIWindow* window, RaeenUIWindowState state);

// View management
RaeenUIView* raeenui_create_view(RaeenUIViewType type);
void raeenui_destroy_view(RaeenUIView* view);
void raeenui_add_child_view(RaeenUIView* parent, RaeenUIView* child);
void raeenui_remove_child_view(RaeenUIView* child);
void raeenui_set_view_frame(RaeenUIView* view, RaeenUIRect frame);
void raeenui_set_view_style(RaeenUIView* view, RaeenUIStyle* style);

// Layout system
void raeenui_layout_view(RaeenUIView* view);
void raeenui_layout_window(RaeenUIWindow* window);
RaeenUISize raeenui_measure_view(RaeenUIView* view, RaeenUISize available_size);

// Rendering
void raeenui_render_frame(RaeenUIContext* context);
void raeenui_render_window(RaeenUIWindow* window);
void raeenui_render_view(RaeenUIView* view, GraphicsContext* gfx);

// Event handling
void raeenui_handle_event(RaeenUIContext* context, RaeenUIEvent* event);
void raeenui_set_event_handler(RaeenUIView* view, RaeenUIEventType type, RaeenUIEventHandler handler);

// Animation system
RaeenUIAnimation* raeenui_create_animation(RaeenUIView* target, float duration);
void raeenui_start_animation(RaeenUIAnimation* animation);
void raeenui_stop_animation(RaeenUIAnimation* animation);
void raeenui_update_animations(RaeenUIContext* context, float delta_time);

// Theme system
RaeenUITheme* raeenui_create_theme(const char* name, RaeenUIThemeMode mode);
void raeenui_destroy_theme(RaeenUITheme* theme);
void raeenui_set_theme(RaeenUIContext* context, RaeenUITheme* theme);
RaeenUITheme* raeenui_get_builtin_theme(RaeenUIThemeMode mode);

// AI integration
void raeenui_enable_ai(RaeenUIContext* context, void* ai_engine);
void raeenui_set_view_ai_context(RaeenUIView* view, const char* context);
void raeenui_trigger_ai_response(RaeenUIView* view, const char* prompt);

// Utility functions
RaeenUIColor raeenui_color_rgba(float r, float g, float b, float a);
RaeenUIColor raeenui_color_hex(uint32_t hex);
RaeenUIRect raeenui_rect_make(float x, float y, float width, float height);
RaeenUIPoint raeenui_point_make(float x, float y);
RaeenUISize raeenui_size_make(float width, float height);

// Declarative UI helpers (SwiftUI-inspired)
#define RaeenUIVStack(spacing) raeenui_create_vstack(spacing)
#define RaeenUIHStack(spacing) raeenui_create_hstack(spacing)
#define RaeenUIText(text) raeenui_create_text(text)
#define RaeenUIButton(title, action) raeenui_create_button(title, action)
#define RaeenUIImage(path) raeenui_create_image(path)

// Convenience macros
#define RAEENUI_COLOR_CLEAR raeenui_color_rgba(0, 0, 0, 0)
#define RAEENUI_COLOR_WHITE raeenui_color_rgba(1, 1, 1, 1)
#define RAEENUI_COLOR_BLACK raeenui_color_rgba(0, 0, 0, 1)
#define RAEENUI_COLOR_RED raeenui_color_rgba(1, 0, 0, 1)
#define RAEENUI_COLOR_GREEN raeenui_color_rgba(0, 1, 0, 1)
#define RAEENUI_COLOR_BLUE raeenui_color_rgba(0, 0, 1, 1)

#endif // RAEENUI_H
