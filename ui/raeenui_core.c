/**
 * RaeenUI - Core Implementation
 * Modern GPU-Accelerated UI Framework for RaeenOS
 */

#include "raeenui.h"
#include "../gpu/graphics_pipeline.h"
#include "../gpu/compositor.h"
#include "../memory.h"
#include "../string.h"
#include "../time.h"

// Global UI context
static RaeenUIContext* g_ui_context = NULL;

// Internal helper functions
static void raeenui_init_default_themes(RaeenUIContext* context);
static void raeenui_setup_gpu_resources(RaeenUIContext* context);
static void raeenui_cleanup_gpu_resources(RaeenUIContext* context);
static uint32_t raeenui_generate_view_id(void);
static uint32_t raeenui_generate_window_id(void);
static void raeenui_calculate_layout_recursive(RaeenUIView* view, RaeenUISize available_size);
static void raeenui_render_view_recursive(RaeenUIView* view, GraphicsContext* gfx);
static bool raeenui_point_in_view(RaeenUIView* view, RaeenUIPoint point);
static RaeenUIView* raeenui_hit_test(RaeenUIView* root, RaeenUIPoint point);

/**
 * Create and initialize UI context
 */
RaeenUIContext* raeenui_create_context(GraphicsPipeline* graphics) {
    if (!graphics) return NULL;
    
    RaeenUIContext* context = (RaeenUIContext*)memory_alloc(sizeof(RaeenUIContext));
    if (!context) return NULL;
    
    memory_set(context, 0, sizeof(RaeenUIContext));
    
    context->graphics_pipeline = graphics;
    context->gpu_acceleration = true;
    context->vsync_enabled = true;
    context->high_dpi_support = true;
    context->ui_scale_factor = 1.0f;
    context->ai_enabled = false;
    
    // Initialize default themes
    raeenui_init_default_themes(context);
    
    // Setup GPU resources
    raeenui_setup_gpu_resources(context);
    
    printf("RaeenUI: Context created successfully\n");
    return context;
}

/**
 * Destroy UI context
 */
void raeenui_destroy_context(RaeenUIContext* context) {
    if (!context) return;
    
    // Destroy all windows
    RaeenUIWindow* window = context->windows;
    while (window) {
        RaeenUIWindow* next = window->next;
        raeenui_destroy_window(window);
        window = next;
    }
    
    // Cleanup GPU resources
    raeenui_cleanup_gpu_resources(context);
    
    // Free themes
    for (uint32_t i = 0; i < context->theme_count; i++) {
        if (context->themes[i]) {
            raeenui_destroy_theme(context->themes[i]);
        }
    }
    
    memory_free(context);
    printf("RaeenUI: Context destroyed\n");
}

/**
 * Initialize UI framework
 */
bool raeenui_initialize(RaeenUIContext* context) {
    if (!context) return false;
    
    g_ui_context = context;
    
    // Initialize graphics pipeline if needed
    if (!graphics_pipeline_is_initialized(context->graphics_pipeline)) {
        if (!graphics_pipeline_initialize(context->graphics_pipeline)) {
            printf("RaeenUI: Failed to initialize graphics pipeline\n");
            return false;
        }
    }
    
    printf("RaeenUI: Framework initialized successfully\n");
    return true;
}

/**
 * Shutdown UI framework
 */
void raeenui_shutdown(RaeenUIContext* context) {
    if (!context) return;
    
    // Stop all animations
    RaeenUIAnimation* anim = context->active_animations;
    while (anim) {
        RaeenUIAnimation* next = anim->next;
        raeenui_stop_animation(anim);
        anim = next;
    }
    
    g_ui_context = NULL;
    printf("RaeenUI: Framework shutdown complete\n");
}

/**
 * Create a new window
 */
RaeenUIWindow* raeenui_create_window(RaeenUIContext* context, const char* title, RaeenUIRect frame) {
    if (!context || !title) return NULL;
    
    RaeenUIWindow* window = (RaeenUIWindow*)memory_alloc(sizeof(RaeenUIWindow));
    if (!window) return NULL;
    
    memory_set(window, 0, sizeof(RaeenUIWindow));
    
    window->window_id = raeenui_generate_window_id();
    string_copy(window->title, title, sizeof(window->title));
    window->frame = frame;
    window->state = RAEENUI_WINDOW_NORMAL;
    window->resizable = true;
    window->closable = true;
    window->minimizable = true;
    window->maximizable = true;
    window->opacity = 1.0f;
    window->theme = context->current_theme;
    
    // Create root view
    window->root_view = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (window->root_view) {
        window->root_view->frame = raeenui_rect_make(0, 0, frame.size.width, frame.size.height);
        window->root_view->style.background_color = window->theme->background;
    }
    
    // Create graphics context for window
    window->graphics_context = graphics_create_context(context->graphics_pipeline);
    
    // Add to window list
    window->next = context->windows;
    context->windows = window;
    context->window_count++;
    
    printf("RaeenUI: Window '%s' created (ID: %u)\n", title, window->window_id);
    return window;
}

/**
 * Destroy a window
 */
void raeenui_destroy_window(RaeenUIWindow* window) {
    if (!window) return;
    
    // Destroy root view and all children
    if (window->root_view) {
        raeenui_destroy_view(window->root_view);
    }
    
    // Cleanup graphics context
    if (window->graphics_context) {
        graphics_destroy_context(window->graphics_context);
    }
    
    printf("RaeenUI: Window '%s' destroyed\n", window->title);
    memory_free(window);
}

/**
 * Create a new view
 */
RaeenUIView* raeenui_create_view(RaeenUIViewType type) {
    RaeenUIView* view = (RaeenUIView*)memory_alloc(sizeof(RaeenUIView));
    if (!view) return NULL;
    
    memory_set(view, 0, sizeof(RaeenUIView));
    
    view->view_id = raeenui_generate_view_id();
    view->type = type;
    view->is_enabled = true;
    view->needs_layout = true;
    view->needs_render = true;
    view->accessibility_enabled = true;
    
    // Set default style
    view->style.background_color = RAEENUI_COLOR_CLEAR;
    view->style.foreground_color = RAEENUI_COLOR_BLACK;
    view->style.opacity = 1.0f;
    view->style.visible = true;
    view->style.font_size = 14.0f;
    string_copy(view->style.font_family, "RaeenUI-Regular", sizeof(view->style.font_family));
    
    // Set default layout
    view->layout.type = RAEENUI_LAYOUT_ABSOLUTE;
    view->layout.horizontal_alignment = RAEENUI_ALIGN_START;
    view->layout.vertical_alignment = RAEENUI_ALIGN_START;
    
    return view;
}

/**
 * Destroy a view and all its children
 */
void raeenui_destroy_view(RaeenUIView* view) {
    if (!view) return;
    
    // Destroy all children recursively
    RaeenUIView* child = view->first_child;
    while (child) {
        RaeenUIView* next = child->next_sibling;
        raeenui_destroy_view(child);
        child = next;
    }
    
    // Free text content
    if (view->text_content) {
        memory_free(view->text_content);
    }
    
    // Free custom data
    if (view->custom_data) {
        memory_free(view->custom_data);
    }
    
    memory_free(view);
}

/**
 * Add child view to parent
 */
void raeenui_add_child_view(RaeenUIView* parent, RaeenUIView* child) {
    if (!parent || !child) return;
    
    // Remove from previous parent
    raeenui_remove_child_view(child);
    
    // Set parent
    child->parent = parent;
    
    // Add to parent's child list
    if (!parent->first_child) {
        parent->first_child = child;
    } else {
        RaeenUIView* last_child = parent->first_child;
        while (last_child->next_sibling) {
            last_child = last_child->next_sibling;
        }
        last_child->next_sibling = child;
    }
    
    parent->child_count++;
    parent->needs_layout = true;
}

/**
 * Remove child view from parent
 */
void raeenui_remove_child_view(RaeenUIView* child) {
    if (!child || !child->parent) return;
    
    RaeenUIView* parent = child->parent;
    
    // Remove from parent's child list
    if (parent->first_child == child) {
        parent->first_child = child->next_sibling;
    } else {
        RaeenUIView* sibling = parent->first_child;
        while (sibling && sibling->next_sibling != child) {
            sibling = sibling->next_sibling;
        }
        if (sibling) {
            sibling->next_sibling = child->next_sibling;
        }
    }
    
    child->parent = NULL;
    child->next_sibling = NULL;
    parent->child_count--;
    parent->needs_layout = true;
}

/**
 * Set view frame
 */
void raeenui_set_view_frame(RaeenUIView* view, RaeenUIRect frame) {
    if (!view) return;
    
    view->frame = frame;
    view->needs_layout = true;
    view->needs_render = true;
}

/**
 * Set view style
 */
void raeenui_set_view_style(RaeenUIView* view, RaeenUIStyle* style) {
    if (!view || !style) return;
    
    view->style = *style;
    view->needs_render = true;
}

/**
 * Layout a view and its children
 */
void raeenui_layout_view(RaeenUIView* view) {
    if (!view) return;
    
    RaeenUISize available_size = view->frame.size;
    raeenui_calculate_layout_recursive(view, available_size);
    view->needs_layout = false;
}

/**
 * Layout entire window
 */
void raeenui_layout_window(RaeenUIWindow* window) {
    if (!window || !window->root_view) return;
    
    // Update root view frame to match window
    window->root_view->frame = raeenui_rect_make(0, 0, window->frame.size.width, window->frame.size.height);
    
    raeenui_layout_view(window->root_view);
    window->needs_redraw = true;
}

/**
 * Render a single frame
 */
void raeenui_render_frame(RaeenUIContext* context) {
    if (!context) return;
    
    uint64_t frame_start = time_get_ticks();
    
    // Update animations
    float delta_time = (frame_start - context->last_frame_time) / 1000000.0f; // Convert to seconds
    raeenui_update_animations(context, delta_time);
    
    // Render all windows
    RaeenUIWindow* window = context->windows;
    while (window) {
        if (window->state != RAEENUI_WINDOW_MINIMIZED) {
            raeenui_render_window(window);
        }
        window = window->next;
    }
    
    // Update performance stats
    context->frame_count++;
    context->last_frame_time = frame_start;
    
    uint64_t frame_end = time_get_ticks();
    float frame_time = (frame_end - frame_start) / 1000000.0f;
    context->fps = 1.0f / frame_time;
}

/**
 * Render a window
 */
void raeenui_render_window(RaeenUIWindow* window) {
    if (!window || !window->graphics_context || !window->root_view) return;
    
    if (!window->needs_redraw && !window->root_view->needs_render) {
        return; // Skip rendering if nothing changed
    }
    
    // Begin rendering
    graphics_begin_frame(window->graphics_context);
    
    // Clear background
    RaeenUIColor bg = window->theme->background;
    graphics_clear_color(window->graphics_context, bg.r, bg.g, bg.b, bg.a);
    
    // Render root view and all children
    raeenui_render_view_recursive(window->root_view, window->graphics_context);
    
    // End rendering
    graphics_end_frame(window->graphics_context);
    
    window->needs_redraw = false;
    window->last_render_time = time_get_ticks();
}

/**
 * Handle UI events
 */
void raeenui_handle_event(RaeenUIContext* context, RaeenUIEvent* event) {
    if (!context || !event) return;
    
    // Update input state
    if (event->type == RAEENUI_EVENT_CLICK) {
        context->mouse_position = event->position;
    }
    
    // Find target window
    RaeenUIWindow* target_window = context->active_window;
    if (!target_window) return;
    
    // Hit test to find target view
    RaeenUIView* target_view = raeenui_hit_test(target_window->root_view, event->position);
    if (target_view) {
        event->target = target_view;
        
        // Call view's event handler
        if (target_view->event_handlers[event->type]) {
            target_view->event_handlers[event->type](target_view, event);
        }
    }
    
    // Call window's global event handler
    if (target_window->global_event_handler) {
        target_window->global_event_handler(target_view, event);
    }
}

/**
 * Create default themes
 */
static void raeenui_init_default_themes(RaeenUIContext* context) {
    // Light theme
    RaeenUITheme* light_theme = raeenui_create_theme("Light", RAEENUI_THEME_LIGHT);
    if (light_theme) {
        light_theme->primary = raeenui_color_hex(0x007AFF);
        light_theme->secondary = raeenui_color_hex(0x5856D6);
        light_theme->accent = raeenui_color_hex(0xFF3B30);
        light_theme->background = raeenui_color_hex(0xFFFFFF);
        light_theme->surface = raeenui_color_hex(0xF2F2F7);
        light_theme->text_primary = raeenui_color_hex(0x000000);
        light_theme->text_secondary = raeenui_color_hex(0x3C3C43);
        
        context->themes[context->theme_count++] = light_theme;
        context->current_theme = light_theme;
    }
    
    // Dark theme
    RaeenUITheme* dark_theme = raeenui_create_theme("Dark", RAEENUI_THEME_DARK);
    if (dark_theme) {
        dark_theme->primary = raeenui_color_hex(0x0A84FF);
        dark_theme->secondary = raeenui_color_hex(0x5E5CE6);
        dark_theme->accent = raeenui_color_hex(0xFF453A);
        dark_theme->background = raeenui_color_hex(0x000000);
        dark_theme->surface = raeenui_color_hex(0x1C1C1E);
        dark_theme->text_primary = raeenui_color_hex(0xFFFFFF);
        dark_theme->text_secondary = raeenui_color_hex(0xEBEBF5);
        
        context->themes[context->theme_count++] = dark_theme;
    }
}

/**
 * Setup GPU resources
 */
static void raeenui_setup_gpu_resources(RaeenUIContext* context) {
    if (!context || !context->graphics_pipeline) return;
    
    // Initialize compositor for window compositing
    compositor_initialize(context->graphics_pipeline);
    
    printf("RaeenUI: GPU resources initialized\n");
}

/**
 * Cleanup GPU resources
 */
static void raeenui_cleanup_gpu_resources(RaeenUIContext* context) {
    if (!context) return;
    
    compositor_shutdown();
    
    printf("RaeenUI: GPU resources cleaned up\n");
}

/**
 * Generate unique view ID
 */
static uint32_t raeenui_generate_view_id(void) {
    static uint32_t next_id = 1;
    return next_id++;
}

/**
 * Generate unique window ID
 */
static uint32_t raeenui_generate_window_id(void) {
    static uint32_t next_id = 1;
    return next_id++;
}

/**
 * Calculate layout recursively
 */
static void raeenui_calculate_layout_recursive(RaeenUIView* view, RaeenUISize available_size) {
    if (!view) return;
    
    // Apply padding to available size
    available_size.width -= view->style.padding.left + view->style.padding.right;
    available_size.height -= view->style.padding.top + view->style.padding.bottom;
    
    // Layout children based on layout type
    switch (view->layout.type) {
        case RAEENUI_LAYOUT_STACK_VERTICAL: {
            float y_offset = view->style.padding.top;
            RaeenUIView* child = view->first_child;
            
            while (child) {
                RaeenUISize child_size = raeenui_measure_view(child, available_size);
                child->frame = raeenui_rect_make(
                    view->style.padding.left,
                    y_offset,
                    child_size.width,
                    child_size.height
                );
                
                raeenui_calculate_layout_recursive(child, child_size);
                y_offset += child_size.height + view->layout.spacing;
                child = child->next_sibling;
            }
            break;
        }
        
        case RAEENUI_LAYOUT_STACK_HORIZONTAL: {
            float x_offset = view->style.padding.left;
            RaeenUIView* child = view->first_child;
            
            while (child) {
                RaeenUISize child_size = raeenui_measure_view(child, available_size);
                child->frame = raeenui_rect_make(
                    x_offset,
                    view->style.padding.top,
                    child_size.width,
                    child_size.height
                );
                
                raeenui_calculate_layout_recursive(child, child_size);
                x_offset += child_size.width + view->layout.spacing;
                child = child->next_sibling;
            }
            break;
        }
        
        default:
            // For other layout types, just layout children in their current frames
            RaeenUIView* child = view->first_child;
            while (child) {
                raeenui_calculate_layout_recursive(child, child->frame.size);
                child = child->next_sibling;
            }
            break;
    }
    
    view->needs_layout = false;
}

/**
 * Render view recursively
 */
static void raeenui_render_view_recursive(RaeenUIView* view, GraphicsContext* gfx) {
    if (!view || !view->style.visible || view->style.opacity <= 0.0f) return;
    
    // Render this view
    raeenui_render_view(view, gfx);
    
    // Render all children
    RaeenUIView* child = view->first_child;
    while (child) {
        raeenui_render_view_recursive(child, gfx);
        child = child->next_sibling;
    }
    
    view->needs_render = false;
}

/**
 * Check if point is inside view bounds
 */
static bool raeenui_point_in_view(RaeenUIView* view, RaeenUIPoint point) {
    if (!view) return false;
    
    return (point.x >= view->frame.origin.x &&
            point.x <= view->frame.origin.x + view->frame.size.width &&
            point.y >= view->frame.origin.y &&
            point.y <= view->frame.origin.y + view->frame.size.height);
}

/**
 * Hit test to find view at point
 */
static RaeenUIView* raeenui_hit_test(RaeenUIView* root, RaeenUIPoint point) {
    if (!root || !raeenui_point_in_view(root, point)) return NULL;
    
    // Check children first (front to back)
    RaeenUIView* child = root->first_child;
    while (child) {
        RaeenUIView* hit = raeenui_hit_test(child, point);
        if (hit) return hit;
        child = child->next_sibling;
    }
    
    // Return this view if no children were hit
    return root;
}

/**
 * Utility functions
 */
RaeenUIColor raeenui_color_rgba(float r, float g, float b, float a) {
    RaeenUIColor color = {r, g, b, a};
    return color;
}

RaeenUIColor raeenui_color_hex(uint32_t hex) {
    float r = ((hex >> 16) & 0xFF) / 255.0f;
    float g = ((hex >> 8) & 0xFF) / 255.0f;
    float b = (hex & 0xFF) / 255.0f;
    return raeenui_color_rgba(r, g, b, 1.0f);
}

RaeenUIRect raeenui_rect_make(float x, float y, float width, float height) {
    RaeenUIRect rect = {{x, y}, {width, height}};
    return rect;
}

RaeenUIPoint raeenui_point_make(float x, float y) {
    RaeenUIPoint point = {x, y};
    return point;
}

RaeenUISize raeenui_size_make(float width, float height) {
    RaeenUISize size = {width, height};
    return size;
}
