// RaeenUI VGA Prototype - Lightweight UI Framework for VGA text mode
// This is a demonstration of RaeenUI concepts adapted for VGA constraints

#ifndef RAEENUI_VGA_H
#define RAEENUI_VGA_H

#include "include/types.h"
#include "vga.h"
#include "mouse_simple.h"

// ============================================================================
// SIMPLIFIED RAEENUI CORE TYPES
// ============================================================================

typedef uint32_t ui_id_t;

// Simplified rectangle for VGA coordinates
typedef struct {
    int x, y;
    int width, height;
} ui_rect_t;

// UI event types
typedef enum {
    UI_EVENT_NONE,
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_MOUSE_CLICK,
    UI_EVENT_KEY_PRESS,
    UI_EVENT_TIMER
} ui_event_type_t;

// UI event structure
typedef struct {
    ui_event_type_t type;
    int x, y;               // Mouse coordinates
    char key;               // Key pressed
    bool mouse_left;        // Left mouse button state
    bool mouse_right;       // Right mouse button state
    ui_id_t target_id;      // Target component ID
} ui_event_t;

// Component types
typedef enum {
    UI_COMPONENT_PANEL,
    UI_COMPONENT_BUTTON,
    UI_COMPONENT_LABEL,
    UI_COMPONENT_TEXTBOX,
    UI_COMPONENT_WINDOW,
    UI_COMPONENT_MENU,
    UI_COMPONENT_PROGRESSBAR
} ui_component_type_t;

// Component states
typedef enum {
    UI_STATE_NORMAL,
    UI_STATE_HOVER,
    UI_STATE_PRESSED,
    UI_STATE_FOCUSED,
    UI_STATE_DISABLED
} ui_component_state_t;

// Forward declarations
typedef struct ui_component ui_component_t;
typedef struct ui_context ui_context_t;

// Event handler function pointer
typedef bool (*ui_event_handler_t)(ui_component_t* component, ui_event_t* event);

// Component structure
typedef struct ui_component {
    ui_id_t id;
    ui_component_type_t type;
    ui_component_state_t state;
    ui_rect_t bounds;
    
    // Visual properties
    vga_color fg_color;
    vga_color bg_color;
    vga_color border_color;
    bool visible;
    
    // Content
    char* text;
    int text_max_len;
    
    // Tree structure
    struct ui_component* parent;
    struct ui_component** children;
    int child_count;
    int child_capacity;
    
    // Event handling
    ui_event_handler_t event_handler;
    
    // Component-specific data
    void* user_data;
    
    // Flags
    bool needs_redraw;
    bool can_focus;
    bool draggable;
    
    // Animation state (simple)
    int anim_timer;
    int anim_duration;
} ui_component_t;

// UI Context - manages all components
typedef struct ui_context {
    ui_component_t** components;
    int component_count;
    int component_capacity;
    
    ui_component_t* root;
    ui_component_t* focused;
    ui_component_t* hover;
    ui_component_t* drag_target;
    
    // Input state
    mouse_state_t* mouse;
    
    // Animation timer
    uint32_t frame_counter;
    
    // Performance metrics
    uint32_t fps_counter;
    uint32_t last_fps_time;
    uint32_t render_time_us;
} ui_context_t;

// ============================================================================
// CORE FRAMEWORK FUNCTIONS
// ============================================================================

// Framework lifecycle
ui_context_t* ui_init(void);
void ui_shutdown(ui_context_t* ctx);
void ui_update(ui_context_t* ctx);
void ui_render(ui_context_t* ctx);

// Component creation and management
ui_component_t* ui_create_component(ui_context_t* ctx, ui_component_type_t type);
void ui_destroy_component(ui_context_t* ctx, ui_component_t* component);
void ui_add_child(ui_component_t* parent, ui_component_t* child);
void ui_remove_child(ui_component_t* parent, ui_component_t* child);

// Component properties
void ui_set_bounds(ui_component_t* component, int x, int y, int width, int height);
void ui_set_text(ui_component_t* component, const char* text);
void ui_set_colors(ui_component_t* component, vga_color fg, vga_color bg, vga_color border);
void ui_set_visible(ui_component_t* component, bool visible);
void ui_set_event_handler(ui_component_t* component, ui_event_handler_t handler);

// Layout helpers
bool ui_point_in_rect(int x, int y, const ui_rect_t* rect);
ui_component_t* ui_find_component_at(ui_context_t* ctx, int x, int y);
void ui_invalidate(ui_component_t* component);

// Event system
void ui_dispatch_event(ui_context_t* ctx, ui_event_t* event);
bool ui_handle_mouse_event(ui_context_t* ctx);
bool ui_handle_keyboard_event(ui_context_t* ctx, char key);

// Animation system (basic)
void ui_start_animation(ui_component_t* component, int duration);
void ui_update_animations(ui_context_t* ctx);

// ============================================================================
// BUILT-IN COMPONENT CREATORS
// ============================================================================

// Window with title bar and close button
ui_component_t* ui_create_window(ui_context_t* ctx, int x, int y, int width, int height, const char* title);

// Button with click handling
ui_component_t* ui_create_button(ui_context_t* ctx, int x, int y, int width, const char* text);

// Text label
ui_component_t* ui_create_label(ui_context_t* ctx, int x, int y, const char* text);

// Text input box
ui_component_t* ui_create_textbox(ui_context_t* ctx, int x, int y, int width, const char* placeholder);

// Progress bar
ui_component_t* ui_create_progressbar(ui_context_t* ctx, int x, int y, int width, int progress);

// Panel (container)
ui_component_t* ui_create_panel(ui_context_t* ctx, int x, int y, int width, int height);

// Menu with dropdown
ui_component_t* ui_create_menu(ui_context_t* ctx, int x, int y, const char* title);
void ui_menu_add_item(ui_component_t* menu, const char* item_text, ui_event_handler_t handler);

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

// Demo scenes showcasing RaeenUI capabilities
void ui_demo_desktop_environment(ui_context_t* ctx);
void ui_demo_gaming_overlay(ui_context_t* ctx);
void ui_demo_ai_assistance(ui_context_t* ctx);
void ui_demo_animations(ui_context_t* ctx);

// Performance monitoring
void ui_show_performance_overlay(ui_context_t* ctx, bool enable);
void ui_update_performance_metrics(ui_context_t* ctx);

#endif