// RaeenUI VGA Prototype Implementation - RaeenOS

#include "raeenui_vga.h"
#include "string.h"
#include "memory.h"

// Simple memory allocator for UI components
static char ui_heap[16384];  // 16KB for UI components
static size_t ui_heap_pos = 0;

static void* ui_malloc(size_t size) {
    if (ui_heap_pos + size >= sizeof(ui_heap)) {
        return NULL;
    }
    void* ptr = &ui_heap[ui_heap_pos];
    ui_heap_pos += size;
    return ptr;
}

static ui_id_t next_component_id = 1;

// ============================================================================
// CORE FRAMEWORK FUNCTIONS
// ============================================================================

ui_context_t* ui_init(void) {
    ui_context_t* ctx = (ui_context_t*)ui_malloc(sizeof(ui_context_t));
    if (!ctx) return NULL;
    
    ctx->component_capacity = 64;
    ctx->components = (ui_component_t**)ui_malloc(sizeof(ui_component_t*) * ctx->component_capacity);
    ctx->component_count = 0;
    
    ctx->root = NULL;
    ctx->focused = NULL;
    ctx->hover = NULL;
    ctx->drag_target = NULL;
    
    ctx->frame_counter = 0;
    ctx->fps_counter = 0;
    ctx->last_fps_time = 0;
    ctx->render_time_us = 0;
    
    return ctx;
}

void ui_shutdown(ui_context_t* ctx) {
    // In a real system, we'd properly free all memory
    // For this simple allocator, we just reset
    ui_heap_pos = 0;
    next_component_id = 1;
}

ui_component_t* ui_create_component(ui_context_t* ctx, ui_component_type_t type) {
    if (ctx->component_count >= ctx->component_capacity) {
        return NULL;  // Out of component slots
    }
    
    ui_component_t* component = (ui_component_t*)ui_malloc(sizeof(ui_component_t));
    if (!component) return NULL;
    
    // Initialize component
    component->id = next_component_id++;
    component->type = type;
    component->state = UI_STATE_NORMAL;
    component->bounds = (ui_rect_t){0, 0, 10, 3};
    
    // Default colors
    component->fg_color = VGA_COLOR_WHITE;
    component->bg_color = VGA_COLOR_BLACK;
    component->border_color = VGA_COLOR_LIGHT_GREY;
    component->visible = true;
    
    component->text = NULL;
    component->text_max_len = 0;
    
    component->parent = NULL;
    component->children = NULL;
    component->child_count = 0;
    component->child_capacity = 0;
    
    component->event_handler = NULL;
    component->user_data = NULL;
    
    component->needs_redraw = true;
    component->can_focus = false;
    component->draggable = false;
    
    component->anim_timer = 0;
    component->anim_duration = 0;
    
    // Add to context
    ctx->components[ctx->component_count++] = component;
    
    return component;
}

void ui_destroy_component(ui_context_t* ctx, ui_component_t* component) {
    // Remove from parent
    if (component->parent) {
        ui_remove_child(component->parent, component);
    }
    
    // Remove from context tracking
    if (ctx->focused == component) ctx->focused = NULL;
    if (ctx->hover == component) ctx->hover = NULL;
    if (ctx->drag_target == component) ctx->drag_target = NULL;
    
    // Remove from components array
    for (int i = 0; i < ctx->component_count; i++) {
        if (ctx->components[i] == component) {
            // Shift remaining components
            for (int j = i; j < ctx->component_count - 1; j++) {
                ctx->components[j] = ctx->components[j + 1];
            }
            ctx->component_count--;
            break;
        }
    }
}

void ui_add_child(ui_component_t* parent, ui_component_t* child) {
    if (!parent || !child) return;
    
    // Allocate children array if needed
    if (!parent->children) {
        parent->child_capacity = 8;
        parent->children = (ui_component_t**)ui_malloc(sizeof(ui_component_t*) * parent->child_capacity);
        parent->child_count = 0;
    }
    
    // Expand if needed
    if (parent->child_count >= parent->child_capacity) {
        return;  // Simple allocator doesn't support realloc
    }
    
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

void ui_remove_child(ui_component_t* parent, ui_component_t* child) {
    if (!parent || !child || !parent->children) return;
    
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            child->parent = NULL;
            break;
        }
    }
}

// ============================================================================
// COMPONENT PROPERTIES
// ============================================================================

void ui_set_bounds(ui_component_t* component, int x, int y, int width, int height) {
    if (!component) return;
    
    component->bounds.x = x;
    component->bounds.y = y;
    component->bounds.width = width;
    component->bounds.height = height;
    component->needs_redraw = true;
}

void ui_set_text(ui_component_t* component, const char* text) {
    if (!component || !text) return;
    
    // Allocate text buffer if needed
    int text_len = strlen(text);
    if (!component->text || component->text_max_len < text_len + 1) {
        component->text_max_len = text_len + 32;  // Extra space for growth
        component->text = (char*)ui_malloc(component->text_max_len);
    }
    
    if (component->text) {
        strcpy(component->text, text);
        component->needs_redraw = true;
    }
}

void ui_set_colors(ui_component_t* component, vga_color fg, vga_color bg, vga_color border) {
    if (!component) return;
    
    component->fg_color = fg;
    component->bg_color = bg;
    component->border_color = border;
    component->needs_redraw = true;
}

void ui_set_visible(ui_component_t* component, bool visible) {
    if (!component) return;
    
    component->visible = visible;
    component->needs_redraw = true;
}

void ui_set_event_handler(ui_component_t* component, ui_event_handler_t handler) {
    if (!component) return;
    
    component->event_handler = handler;
}

// ============================================================================
// LAYOUT AND HIT TESTING
// ============================================================================

bool ui_point_in_rect(int x, int y, const ui_rect_t* rect) {
    return x >= rect->x && x < rect->x + rect->width &&
           y >= rect->y && y < rect->y + rect->height;
}

ui_component_t* ui_find_component_at(ui_context_t* ctx, int x, int y) {
    // Search from top to bottom (reverse order for proper Z-order)
    for (int i = ctx->component_count - 1; i >= 0; i--) {
        ui_component_t* component = ctx->components[i];
        if (component->visible && ui_point_in_rect(x, y, &component->bounds)) {
            return component;
        }
    }
    return NULL;
}

void ui_invalidate(ui_component_t* component) {
    if (!component) return;
    
    component->needs_redraw = true;
    
    // Invalidate children too
    for (int i = 0; i < component->child_count; i++) {
        ui_invalidate(component->children[i]);
    }
}

// ============================================================================
// RENDERING SYSTEM
// ============================================================================

static void ui_render_component(ui_component_t* component) {
    if (!component || !component->visible) return;
    
    ui_rect_t* bounds = &component->bounds;
    
    switch (component->type) {
        case UI_COMPONENT_PANEL:
            vga_fill_area(bounds->x, bounds->y, bounds->width, bounds->height, 
                         ' ', component->fg_color, component->bg_color);
            vga_draw_box(bounds->x, bounds->y, bounds->width, bounds->height,
                        component->border_color, component->bg_color);
            break;
            
        case UI_COMPONENT_BUTTON: {
            bool pressed = (component->state == UI_STATE_PRESSED);
            vga_draw_button(bounds->x, bounds->y, bounds->width, 
                           component->text ? component->text : "Button",
                           pressed, component->fg_color, component->bg_color);
            break;
        }
        
        case UI_COMPONENT_LABEL:
            if (component->text) {
                size_t orig_x, orig_y;
                vga_get_cursor_position(&orig_x, &orig_y);
                vga_set_cursor_position(bounds->x, bounds->y);
                vga_puts_colored(component->text, component->fg_color, component->bg_color);
                vga_set_cursor_position(orig_x, orig_y);
            }
            break;
            
        case UI_COMPONENT_TEXTBOX:
            vga_fill_area(bounds->x, bounds->y, bounds->width, bounds->height,
                         ' ', component->fg_color, component->bg_color);
            vga_draw_box(bounds->x, bounds->y, bounds->width, bounds->height,
                        component->border_color, component->bg_color);
            if (component->text) {
                size_t orig_x, orig_y;
                vga_get_cursor_position(&orig_x, &orig_y);
                vga_set_cursor_position(bounds->x + 1, bounds->y + 1);
                vga_puts_colored(component->text, component->fg_color, component->bg_color);
                vga_set_cursor_position(orig_x, orig_y);
            }
            break;
            
        case UI_COMPONENT_WINDOW:
            vga_draw_window_frame(bounds->x, bounds->y, bounds->width, bounds->height,
                                 component->text ? component->text : "Window",
                                 component->border_color, component->bg_color);
            break;
            
        case UI_COMPONENT_PROGRESSBAR: {
            int progress = component->user_data ? *((int*)component->user_data) : 0;
            vga_draw_progress_bar(bounds->x, bounds->y, bounds->width, progress,
                                 component->fg_color, component->bg_color);
            break;
        }
            
        default:
            vga_fill_area(bounds->x, bounds->y, bounds->width, bounds->height,
                         '?', VGA_COLOR_RED, VGA_COLOR_YELLOW);
            break;
    }
    
    component->needs_redraw = false;
}

void ui_render(ui_context_t* ctx) {
    if (!ctx) return;
    
    uint32_t render_start = ctx->frame_counter;  // Simple timing
    
    // Render all visible components
    for (int i = 0; i < ctx->component_count; i++) {
        ui_component_t* component = ctx->components[i];
        if (component->needs_redraw) {
            ui_render_component(component);
        }
    }
    
    ctx->render_time_us = ctx->frame_counter - render_start;
    ctx->frame_counter++;
}

// ============================================================================
// EVENT SYSTEM
// ============================================================================

void ui_dispatch_event(ui_context_t* ctx, ui_event_t* event) {
    if (!ctx || !event) return;
    
    // Find target component
    ui_component_t* target = NULL;
    if (event->type == UI_EVENT_MOUSE_MOVE || event->type == UI_EVENT_MOUSE_CLICK) {
        target = ui_find_component_at(ctx, event->x, event->y);
        event->target_id = target ? target->id : 0;
    }
    
    // Handle hover state changes
    if (event->type == UI_EVENT_MOUSE_MOVE) {
        if (ctx->hover != target) {
            // Leave previous hover
            if (ctx->hover) {
                ctx->hover->state = UI_STATE_NORMAL;
                ctx->hover->needs_redraw = true;
            }
            
            // Enter new hover
            ctx->hover = target;
            if (ctx->hover && ctx->hover->type == UI_COMPONENT_BUTTON) {
                ctx->hover->state = UI_STATE_HOVER;
                ctx->hover->needs_redraw = true;
            }
        }
    }
    
    // Handle button press states
    if (event->type == UI_EVENT_MOUSE_CLICK && target) {
        if (target->type == UI_COMPONENT_BUTTON) {
            target->state = event->mouse_left ? UI_STATE_PRESSED : UI_STATE_NORMAL;
            target->needs_redraw = true;
            
            // Start button animation
            ui_start_animation(target, 10);  // 10 frame animation
        }
        
        // Set focus
        if (target->can_focus) {
            if (ctx->focused) {
                ctx->focused->state = UI_STATE_NORMAL;
                ctx->focused->needs_redraw = true;
            }
            ctx->focused = target;
            target->state = UI_STATE_FOCUSED;
            target->needs_redraw = true;
        }
    }
    
    // Call component event handler
    if (target && target->event_handler) {
        target->event_handler(target, event);
    }
}

bool ui_handle_mouse_event(ui_context_t* ctx) {
    mouse_state_t* mouse = mouse_get_state();
    if (!mouse) return false;
    
    ctx->mouse = mouse;
    
    bool handled = false;
    
    if (mouse->has_moved) {
        ui_event_t event = {
            .type = UI_EVENT_MOUSE_MOVE,
            .x = mouse->x,
            .y = mouse->y,
            .mouse_left = mouse->left_button,
            .mouse_right = mouse->right_button
        };
        ui_dispatch_event(ctx, &event);
        handled = true;
    }
    
    if (mouse->has_clicked) {
        ui_event_t event = {
            .type = UI_EVENT_MOUSE_CLICK,
            .x = mouse->x,
            .y = mouse->y,
            .mouse_left = mouse->left_button,
            .mouse_right = mouse->right_button
        };
        ui_dispatch_event(ctx, &event);
        handled = true;
    }
    
    return handled;
}

bool ui_handle_keyboard_event(ui_context_t* ctx, char key) {
    ui_event_t event = {
        .type = UI_EVENT_KEY_PRESS,
        .key = key,
        .target_id = ctx->focused ? ctx->focused->id : 0
    };
    
    ui_dispatch_event(ctx, &event);
    return true;
}

// ============================================================================
// ANIMATION SYSTEM
// ============================================================================

void ui_start_animation(ui_component_t* component, int duration) {
    if (!component) return;
    
    component->anim_timer = 0;
    component->anim_duration = duration;
}

void ui_update_animations(ui_context_t* ctx) {
    for (int i = 0; i < ctx->component_count; i++) {
        ui_component_t* component = ctx->components[i];
        
        if (component->anim_duration > 0) {
            component->anim_timer++;
            
            if (component->anim_timer >= component->anim_duration) {
                // Animation complete
                component->anim_duration = 0;
                component->anim_timer = 0;
                
                // Reset button state after animation
                if (component->type == UI_COMPONENT_BUTTON && 
                    component->state == UI_STATE_PRESSED) {
                    component->state = UI_STATE_HOVER;
                    component->needs_redraw = true;
                }
            } else {
                // Animation in progress - could add easing here
                component->needs_redraw = true;
            }
        }
    }
}

void ui_update(ui_context_t* ctx) {
    if (!ctx) return;
    
    // Handle input events
    ui_handle_mouse_event(ctx);
    
    // Update animations
    ui_update_animations(ctx);
    
    // Update performance metrics
    ui_update_performance_metrics(ctx);
}

// ============================================================================
// BUILT-IN COMPONENT CREATORS
// ============================================================================

ui_component_t* ui_create_window(ui_context_t* ctx, int x, int y, int width, int height, const char* title) {
    ui_component_t* window = ui_create_component(ctx, UI_COMPONENT_WINDOW);
    if (!window) return NULL;
    
    ui_set_bounds(window, x, y, width, height);
    ui_set_text(window, title);
    ui_set_colors(window, VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY);
    window->draggable = true;
    
    return window;
}

ui_component_t* ui_create_button(ui_context_t* ctx, int x, int y, int width, const char* text) {
    ui_component_t* button = ui_create_component(ctx, UI_COMPONENT_BUTTON);
    if (!button) return NULL;
    
    ui_set_bounds(button, x, y, width, 3);
    ui_set_text(button, text);
    ui_set_colors(button, VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY, VGA_COLOR_DARK_GREY);
    button->can_focus = true;
    
    return button;
}

ui_component_t* ui_create_label(ui_context_t* ctx, int x, int y, const char* text) {
    ui_component_t* label = ui_create_component(ctx, UI_COMPONENT_LABEL);
    if (!label) return NULL;
    
    int text_len = text ? strlen(text) : 0;
    ui_set_bounds(label, x, y, text_len, 1);
    ui_set_text(label, text);
    ui_set_colors(label, VGA_COLOR_WHITE, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    return label;
}

ui_component_t* ui_create_textbox(ui_context_t* ctx, int x, int y, int width, const char* placeholder) {
    ui_component_t* textbox = ui_create_component(ctx, UI_COMPONENT_TEXTBOX);
    if (!textbox) return NULL;
    
    ui_set_bounds(textbox, x, y, width, 3);
    ui_set_text(textbox, placeholder ? placeholder : "");
    ui_set_colors(textbox, VGA_COLOR_BLACK, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    textbox->can_focus = true;
    
    return textbox;
}

ui_component_t* ui_create_progressbar(ui_context_t* ctx, int x, int y, int width, int progress) {
    ui_component_t* progressbar = ui_create_component(ctx, UI_COMPONENT_PROGRESSBAR);
    if (!progressbar) return NULL;
    
    ui_set_bounds(progressbar, x, y, width, 3);
    ui_set_colors(progressbar, VGA_COLOR_GREEN, VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    
    // Store progress value
    int* progress_data = (int*)ui_malloc(sizeof(int));
    if (progress_data) {
        *progress_data = progress;
        progressbar->user_data = progress_data;
    }
    
    return progressbar;
}

ui_component_t* ui_create_panel(ui_context_t* ctx, int x, int y, int width, int height) {
    ui_component_t* panel = ui_create_component(ctx, UI_COMPONENT_PANEL);
    if (!panel) return NULL;
    
    ui_set_bounds(panel, x, y, width, height);
    ui_set_colors(panel, VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_COLOR_LIGHT_GREY);
    
    return panel;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

void ui_update_performance_metrics(ui_context_t* ctx) {
    ctx->fps_counter++;
    
    // Update FPS every ~60 frames (rough timing)
    if (ctx->fps_counter >= 60) {
        ctx->last_fps_time = ctx->fps_counter;
        ctx->fps_counter = 0;
    }
}

void ui_show_performance_overlay(ui_context_t* ctx, bool enable) {
    if (!enable) return;
    
    // Simple performance display in top-right corner
    char fps_text[32];
    char components_text[32];
    char memory_text[32];
    
    // Format performance metrics
    strcpy(fps_text, "FPS: ~60");  // Simplified
    strcpy(components_text, "Components: ");
    // Simple number to string
    char comp_num[8];
    int comp_count = ctx->component_count;
    int digit_count = 0;
    if (comp_count == 0) {
        comp_num[0] = '0';
        digit_count = 1;
    } else {
        while (comp_count > 0) {
            comp_num[digit_count++] = (comp_count % 10) + '0';
            comp_count /= 10;
        }
        // Reverse digits
        for (int i = 0; i < digit_count / 2; i++) {
            char temp = comp_num[i];
            comp_num[i] = comp_num[digit_count - 1 - i];
            comp_num[digit_count - 1 - i] = temp;
        }
    }
    comp_num[digit_count] = '\0';
    strcat(components_text, comp_num);
    
    strcpy(memory_text, "Memory: ");
    // Simple memory usage
    int memory_usage = (ui_heap_pos * 100) / sizeof(ui_heap);
    char mem_num[8];
    int mem_digits = 0;
    if (memory_usage == 0) {
        mem_num[0] = '0';
        mem_digits = 1;
    } else {
        int temp_usage = memory_usage;
        while (temp_usage > 0) {
            mem_num[mem_digits++] = (temp_usage % 10) + '0';
            temp_usage /= 10;
        }
        for (int i = 0; i < mem_digits / 2; i++) {
            char temp = mem_num[i];
            mem_num[i] = mem_num[mem_digits - 1 - i];
            mem_num[mem_digits - 1 - i] = temp;
        }
    }
    mem_num[mem_digits] = '\0';
    strcat(memory_text, mem_num);
    strcat(memory_text, "%");
    
    // Display overlay
    vga_puts_colored(fps_text, VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    size_t orig_x, orig_y;
    vga_get_cursor_position(&orig_x, &orig_y);
    vga_set_cursor_position(60, 1);
    vga_puts_colored(components_text, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_set_cursor_position(60, 2);
    vga_puts_colored(memory_text, VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_set_cursor_position(orig_x, orig_y);
}