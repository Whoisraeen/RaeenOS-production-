#pragma once

/**
 * RaeenUI - Revolutionary GPU-Accelerated UI Framework for RaeenOS
 * 
 * A declarative, component-based UI framework inspired by React, SwiftUI, and Flutter
 * with gaming-grade performance and AI-native integration.
 * 
 * Key Features:
 * - GPU-accelerated rendering pipeline with Vulkan-like performance
 * - Declarative component tree (React Fiber-inspired)
 * - Real-time theming and animation system
 * - AI-aware adaptive components
 * - Sub-millisecond input latency
 * - Universal input support (mouse, touch, gesture, voice, AI)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CORE TYPES AND STRUCTURES
// ============================================================================

// Forward declarations
typedef struct RaeenUIContext RaeenUIContext;
typedef struct RaeenUINode RaeenUINode;
typedef struct RaeenUIComponent RaeenUIComponent;
typedef struct RaeenUIEvent RaeenUIEvent;
typedef struct RaeenUIStyle RaeenUIStyle;
typedef struct RaeenUIAnimation RaeenUIAnimation;

// Unique identifier for UI elements
typedef uint64_t RaeenUIId;

// Color with full alpha support
typedef struct {
    float r, g, b, a;  // 0.0-1.0 range for GPU efficiency
} RaeenUIColor;

// 2D Vector for positions, sizes, etc.
typedef struct {
    float x, y;
} RaeenUIVec2;

// Rectangle for bounds, layouts
typedef struct {
    RaeenUIVec2 position;
    RaeenUIVec2 size;
} RaeenUIRect;

// Transform matrix for GPU acceleration
typedef struct {
    float matrix[16];  // 4x4 matrix for full 3D transforms
} RaeenUITransform;

// ============================================================================
// COMPONENT SYSTEM (React-like)
// ============================================================================

// Component types
typedef enum {
    RAEENUI_COMPONENT_VIEW,
    RAEENUI_COMPONENT_TEXT,
    RAEENUI_COMPONENT_BUTTON,
    RAEENUI_COMPONENT_INPUT,
    RAEENUI_COMPONENT_IMAGE,
    RAEENUI_COMPONENT_SCROLL_VIEW,
    RAEENUI_COMPONENT_STACK,
    RAEENUI_COMPONENT_GRID,
    RAEENUI_COMPONENT_FLEX,
    RAEENUI_COMPONENT_WINDOW,
    RAEENUI_COMPONENT_CUSTOM
} RaeenUIComponentType;

// Layout types (CSS Flexbox + CSS Grid inspired)
typedef enum {
    RAEENUI_LAYOUT_ABSOLUTE,
    RAEENUI_LAYOUT_FLEX,
    RAEENUI_LAYOUT_GRID,
    RAEENUI_LAYOUT_STACK
} RaeenUILayoutType;

// Flex direction
typedef enum {
    RAEENUI_FLEX_ROW,
    RAEENUI_FLEX_COLUMN,
    RAEENUI_FLEX_ROW_REVERSE,
    RAEENUI_FLEX_COLUMN_REVERSE
} RaeenUIFlexDirection;

// Alignment options
typedef enum {
    RAEENUI_ALIGN_START,
    RAEENUI_ALIGN_CENTER,
    RAEENUI_ALIGN_END,
    RAEENUI_ALIGN_STRETCH,
    RAEENUI_ALIGN_SPACE_BETWEEN,
    RAEENUI_ALIGN_SPACE_AROUND,
    RAEENUI_ALIGN_SPACE_EVENLY
} RaeenUIAlignment;

// ============================================================================
// STYLING SYSTEM (CSS-like)
// ============================================================================

typedef struct {
    // Layout properties
    RaeenUILayoutType layout_type;
    RaeenUIFlexDirection flex_direction;
    RaeenUIAlignment justify_content;
    RaeenUIAlignment align_items;
    
    // Size and position
    float width, height;
    float min_width, min_height;
    float max_width, max_height;
    float left, top, right, bottom;
    
    // Padding and margin
    float padding_top, padding_right, padding_bottom, padding_left;
    float margin_top, margin_right, margin_bottom, margin_left;
    
    // Visual properties
    RaeenUIColor background_color;
    RaeenUIColor border_color;
    float border_width;
    float border_radius;
    float opacity;
    
    // Advanced visual effects
    float blur_radius;
    float shadow_offset_x, shadow_offset_y;
    float shadow_blur_radius;
    RaeenUIColor shadow_color;
    
    // Transform
    RaeenUITransform transform;
    
    // Typography (for text components)
    char* font_family;
    float font_size;
    RaeenUIColor text_color;
    
    // Flags for which properties are set
    uint64_t property_flags;
} RaeenUIStyle;

// ============================================================================
// ANIMATION SYSTEM (GPU-Accelerated)
// ============================================================================

typedef enum {
    RAEENUI_ANIM_LINEAR,
    RAEENUI_ANIM_EASE_IN,
    RAEENUI_ANIM_EASE_OUT,
    RAEENUI_ANIM_EASE_IN_OUT,
    RAEENUI_ANIM_SPRING,
    RAEENUI_ANIM_BOUNCE,
    RAEENUI_ANIM_CUSTOM_CURVE
} RaeenUIAnimationCurve;

typedef struct {
    RaeenUIId target_id;
    char* property_name;
    float from_value;
    float to_value;
    float duration;  // in seconds
    float delay;
    RaeenUIAnimationCurve curve;
    bool repeat;
    bool reverse;
    void (*on_complete)(RaeenUIId target_id);
} RaeenUIAnimation;

// ============================================================================
// EVENT SYSTEM (Multi-Input Support)
// ============================================================================

typedef enum {
    RAEENUI_EVENT_MOUSE_DOWN,
    RAEENUI_EVENT_MOUSE_UP,
    RAEENUI_EVENT_MOUSE_MOVE,
    RAEENUI_EVENT_MOUSE_WHEEL,
    RAEENUI_EVENT_KEY_DOWN,
    RAEENUI_EVENT_KEY_UP,
    RAEENUI_EVENT_TOUCH_START,
    RAEENUI_EVENT_TOUCH_MOVE,
    RAEENUI_EVENT_TOUCH_END,
    RAEENUI_EVENT_GESTURE,
    RAEENUI_EVENT_VOICE_COMMAND,
    RAEENUI_EVENT_AI_SUGGESTION,
    RAEENUI_EVENT_FOCUS,
    RAEENUI_EVENT_BLUR,
    RAEENUI_EVENT_HOVER_ENTER,
    RAEENUI_EVENT_HOVER_EXIT
} RaeenUIEventType;

typedef struct {
    RaeenUIEventType type;
    RaeenUIId target_id;
    RaeenUIVec2 position;
    uint64_t timestamp;
    
    union {
        struct { int button; } mouse;
        struct { int key_code; char* text; } keyboard;
        struct { int touch_id; } touch;
        struct { char* command; float confidence; } voice;
        struct { char* suggestion; int priority; } ai;
    } data;
} RaeenUIEvent;

// Event handler function pointer
typedef bool (*RaeenUIEventHandler)(RaeenUIEvent* event, void* user_data);

// ============================================================================
// COMPONENT NODE (Virtual DOM-like)
// ============================================================================

typedef struct RaeenUINode {
    RaeenUIId id;
    RaeenUIComponentType type;
    RaeenUIStyle style;
    
    // Tree structure
    struct RaeenUINode* parent;
    struct RaeenUINode** children;
    int child_count;
    int child_capacity;
    
    // Component-specific data
    void* component_data;
    
    // Computed layout (after layout pass)
    RaeenUIRect computed_bounds;
    bool needs_layout;
    bool needs_repaint;
    
    // Event handlers
    RaeenUIEventHandler* event_handlers;
    int handler_count;
    
    // AI context
    char* ai_context;
    float ai_confidence;
    
    // Rendering state
    bool is_visible;
    float computed_opacity;
    RaeenUITransform computed_transform;
} RaeenUINode;

// ============================================================================
// CORE FRAMEWORK FUNCTIONS
// ============================================================================

// Framework initialization
RaeenUIContext* raeenui_init(void);
void raeenui_shutdown(RaeenUIContext* ctx);

// Component creation (declarative API)
RaeenUINode* raeenui_create_component(RaeenUIContext* ctx, RaeenUIComponentType type);
void raeenui_destroy_component(RaeenUIContext* ctx, RaeenUINode* node);

// Tree manipulation
void raeenui_add_child(RaeenUINode* parent, RaeenUINode* child);
void raeenui_remove_child(RaeenUINode* parent, RaeenUINode* child);

// Styling
void raeenui_set_style(RaeenUINode* node, const RaeenUIStyle* style);
void raeenui_set_style_property(RaeenUINode* node, const char* property, float value);
void raeenui_set_style_color(RaeenUINode* node, const char* property, RaeenUIColor color);

// Layout system
void raeenui_layout(RaeenUIContext* ctx, RaeenUINode* root);
void raeenui_invalidate_layout(RaeenUINode* node);

// Rendering pipeline
void raeenui_render(RaeenUIContext* ctx, RaeenUINode* root);
void raeenui_present(RaeenUIContext* ctx);

// Event handling
void raeenui_add_event_handler(RaeenUINode* node, RaeenUIEventType type, RaeenUIEventHandler handler);
void raeenui_dispatch_event(RaeenUIContext* ctx, RaeenUIEvent* event);

// Animation system
RaeenUIAnimation* raeenui_create_animation(RaeenUIId target_id, const char* property, 
                                          float from, float to, float duration);
void raeenui_start_animation(RaeenUIContext* ctx, RaeenUIAnimation* animation);
void raeenui_stop_animation(RaeenUIContext* ctx, RaeenUIAnimation* animation);

// ============================================================================
// AI INTEGRATION
// ============================================================================

// AI-aware component functions
void raeenui_set_ai_context(RaeenUINode* node, const char* context);
void raeenui_enable_ai_suggestions(RaeenUIContext* ctx, bool enable);
void raeenui_process_ai_feedback(RaeenUIContext* ctx, const char* feedback);

// ============================================================================
// THEMING SYSTEM
// ============================================================================

typedef struct {
    char* name;
    RaeenUIStyle* default_styles;
    int style_count;
} RaeenUITheme;

void raeenui_load_theme(RaeenUIContext* ctx, RaeenUITheme* theme);
void raeenui_set_theme_property(RaeenUIContext* ctx, const char* property, float value);
void raeenui_animate_theme_transition(RaeenUIContext* ctx, RaeenUITheme* new_theme, float duration);

// ============================================================================
// PERFORMANCE OPTIMIZATION
// ============================================================================

// GPU acceleration hints
void raeenui_enable_gpu_acceleration(RaeenUIContext* ctx, bool enable);
void raeenui_set_vsync(RaeenUIContext* ctx, bool enable);
void raeenui_set_target_fps(RaeenUIContext* ctx, int fps);

// Memory management
void raeenui_gc_collect(RaeenUIContext* ctx);  // Garbage collection for unused nodes
void raeenui_optimize_tree(RaeenUIContext* ctx, RaeenUINode* root);  // Tree optimization

#ifdef __cplusplus
}
#endif