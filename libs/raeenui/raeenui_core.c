/**
 * RaeenUI Core Implementation
 * Revolutionary GPU-Accelerated UI Framework for RaeenOS
 */

#include "raeenui_core.h"
#include "../../kernel/graphics.h"
#include "../../kernel/memory.h"
#include "../../kernel/string.h"
#include <stdatomic.h>

// ============================================================================
// INTERNAL STRUCTURES
// ============================================================================

typedef struct {
    RaeenUINode** nodes;
    int count;
    int capacity;
} RaeenUINodePool;

typedef struct {
    RaeenUIAnimation** animations;
    int count;
    int capacity;
} RaeenUIAnimationPool;

struct RaeenUIContext {
    // Core rendering
    uint32_t* framebuffer;
    uint32_t screen_width;
    uint32_t screen_height;
    
    // Component tree
    RaeenUINode* root;
    RaeenUINodePool node_pool;
    
    // Animation system
    RaeenUIAnimationPool animation_pool;
    uint64_t frame_time;
    float delta_time;
    
    // Event system
    RaeenUIEvent* event_queue;
    int event_count;
    int event_capacity;
    
    // Performance settings
    bool gpu_acceleration_enabled;
    bool vsync_enabled;
    int target_fps;
    
    // AI integration
    bool ai_suggestions_enabled;
    char* current_ai_context;
    
    // Theming
    RaeenUITheme* current_theme;
    
    // Frame statistics
    uint64_t frame_count;
    float average_frame_time;
};

// ============================================================================
// ATOMIC ID GENERATION
// ============================================================================

static atomic_uint_fast64_t next_id = 1;

static RaeenUIId generate_id(void) {
    return atomic_fetch_add(&next_id, 1);
}

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

static void* raeenui_alloc(size_t size) {
    return kmalloc(size);
}

static void raeenui_free(void* ptr) {
    kfree(ptr);
}

// ============================================================================
// CORE FRAMEWORK IMPLEMENTATION
// ============================================================================

RaeenUIContext* raeenui_init(void) {
    RaeenUIContext* ctx = (RaeenUIContext*)raeenui_alloc(sizeof(RaeenUIContext));
    if (!ctx) return NULL;
    
    // Initialize context
    ctx->screen_width = graphics_get_width();
    ctx->screen_height = graphics_get_height();
    ctx->framebuffer = NULL; // Will be set by graphics system
    
    // Initialize node pool
    ctx->node_pool.capacity = 1024;
    ctx->node_pool.count = 0;
    ctx->node_pool.nodes = (RaeenUINode**)raeenui_alloc(sizeof(RaeenUINode*) * ctx->node_pool.capacity);
    
    // Initialize animation pool
    ctx->animation_pool.capacity = 256;
    ctx->animation_pool.count = 0;
    ctx->animation_pool.animations = (RaeenUIAnimation**)raeenui_alloc(sizeof(RaeenUIAnimation*) * ctx->animation_pool.capacity);
    
    // Initialize event queue
    ctx->event_capacity = 512;
    ctx->event_count = 0;
    ctx->event_queue = (RaeenUIEvent*)raeenui_alloc(sizeof(RaeenUIEvent) * ctx->event_capacity);
    
    // Performance defaults
    ctx->gpu_acceleration_enabled = true;
    ctx->vsync_enabled = true;
    ctx->target_fps = 60;
    
    // AI defaults
    ctx->ai_suggestions_enabled = false;
    ctx->current_ai_context = NULL;
    
    // Create root node
    ctx->root = raeenui_create_component(ctx, RAEENUI_COMPONENT_VIEW);
    
    return ctx;
}

void raeenui_shutdown(RaeenUIContext* ctx) {
    if (!ctx) return;
    
    // Cleanup nodes
    for (int i = 0; i < ctx->node_pool.count; i++) {
        raeenui_destroy_component(ctx, ctx->node_pool.nodes[i]);
    }
    raeenui_free(ctx->node_pool.nodes);
    
    // Cleanup animations
    for (int i = 0; i < ctx->animation_pool.count; i++) {
        raeenui_free(ctx->animation_pool.animations[i]);
    }
    raeenui_free(ctx->animation_pool.animations);
    
    // Cleanup events
    raeenui_free(ctx->event_queue);
    
    // Cleanup context
    if (ctx->current_ai_context) {
        raeenui_free(ctx->current_ai_context);
    }
    
    raeenui_free(ctx);
}

// ============================================================================
// COMPONENT MANAGEMENT
// ============================================================================

RaeenUINode* raeenui_create_component(RaeenUIContext* ctx, RaeenUIComponentType type) {
    if (!ctx) return NULL;
    
    RaeenUINode* node = (RaeenUINode*)raeenui_alloc(sizeof(RaeenUINode));
    if (!node) return NULL;
    
    // Initialize node
    node->id = generate_id();
    node->type = type;
    node->parent = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->component_data = NULL;
    node->needs_layout = true;
    node->needs_repaint = true;
    node->is_visible = true;
    node->computed_opacity = 1.0f;
    node->ai_context = NULL;
    node->ai_confidence = 0.0f;
    node->event_handlers = NULL;
    node->handler_count = 0;
    
    // Initialize default style
    RaeenUIStyle default_style = {0};
    default_style.layout_type = RAEENUI_LAYOUT_ABSOLUTE;
    default_style.width = 100.0f;
    default_style.height = 100.0f;
    default_style.background_color = (RaeenUIColor){0.9f, 0.9f, 0.9f, 1.0f};
    default_style.opacity = 1.0f;
    node->style = default_style;
    
    // Add to node pool
    if (ctx->node_pool.count >= ctx->node_pool.capacity) {
        ctx->node_pool.capacity *= 2;
        ctx->node_pool.nodes = (RaeenUINode**)realloc(ctx->node_pool.nodes, 
            sizeof(RaeenUINode*) * ctx->node_pool.capacity);
    }
    ctx->node_pool.nodes[ctx->node_pool.count++] = node;
    
    return node;
}

void raeenui_destroy_component(RaeenUIContext* ctx, RaeenUINode* node) {
    if (!ctx || !node) return;
    
    // Remove from parent
    if (node->parent) {
        raeenui_remove_child(node->parent, node);
    }
    
    // Destroy children
    for (int i = 0; i < node->child_count; i++) {
        raeenui_destroy_component(ctx, node->children[i]);
    }
    
    // Cleanup memory
    if (node->children) raeenui_free(node->children);
    if (node->component_data) raeenui_free(node->component_data);
    if (node->ai_context) raeenui_free(node->ai_context);
    if (node->event_handlers) raeenui_free(node->event_handlers);
    
    // Remove from node pool
    for (int i = 0; i < ctx->node_pool.count; i++) {
        if (ctx->node_pool.nodes[i] == node) {
            // Swap with last element
            ctx->node_pool.nodes[i] = ctx->node_pool.nodes[ctx->node_pool.count - 1];
            ctx->node_pool.count--;
            break;
        }
    }
    
    raeenui_free(node);
}

// ============================================================================
// TREE MANIPULATION
// ============================================================================

void raeenui_add_child(RaeenUINode* parent, RaeenUINode* child) {
    if (!parent || !child) return;
    
    // Remove from previous parent
    if (child->parent) {
        raeenui_remove_child(child->parent, child);
    }
    
    // Expand children array if needed
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        parent->children = (RaeenUINode**)realloc(parent->children, 
            sizeof(RaeenUINode*) * parent->child_capacity);
    }
    
    // Add child
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    
    // Invalidate layout
    raeenui_invalidate_layout(parent);
}

void raeenui_remove_child(RaeenUINode* parent, RaeenUINode* child) {
    if (!parent || !child) return;
    
    // Find and remove child
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            child->parent = NULL;
            
            // Invalidate layout
            raeenui_invalidate_layout(parent);
            break;
        }
    }
}

// ============================================================================
// STYLING SYSTEM
// ============================================================================

void raeenui_set_style(RaeenUINode* node, const RaeenUIStyle* style) {
    if (!node || !style) return;
    
    node->style = *style;
    node->needs_layout = true;
    node->needs_repaint = true;
}

void raeenui_set_style_property(RaeenUINode* node, const char* property, float value) {
    if (!node || !property) return;
    
    // Simple property setting (could be expanded with hash table)
    if (strcmp(property, "width") == 0) {
        node->style.width = value;
    } else if (strcmp(property, "height") == 0) {
        node->style.height = value;
    } else if (strcmp(property, "opacity") == 0) {
        node->style.opacity = value;
    } else if (strcmp(property, "border-radius") == 0) {
        node->style.border_radius = value;
    }
    // ... more properties
    
    node->needs_layout = true;
    node->needs_repaint = true;
}

void raeenui_set_style_color(RaeenUINode* node, const char* property, RaeenUIColor color) {
    if (!node || !property) return;
    
    if (strcmp(property, "background-color") == 0) {
        node->style.background_color = color;
    } else if (strcmp(property, "border-color") == 0) {
        node->style.border_color = color;
    } else if (strcmp(property, "text-color") == 0) {
        node->style.text_color = color;
    }
    
    node->needs_repaint = true;
}

// ============================================================================
// LAYOUT SYSTEM (Simplified Flexbox)
// ============================================================================

static void layout_absolute(RaeenUINode* node) {
    // Absolute positioning - just use style values directly
    node->computed_bounds.position.x = node->style.left;
    node->computed_bounds.position.y = node->style.top;
    node->computed_bounds.size.x = node->style.width;
    node->computed_bounds.size.y = node->style.height;
}

static void layout_flex(RaeenUINode* node) {
    // Simplified flexbox implementation
    float available_width = node->computed_bounds.size.x;
    float available_height = node->computed_bounds.size.y;
    
    float current_x = node->computed_bounds.position.x + node->style.padding_left;
    float current_y = node->computed_bounds.position.y + node->style.padding_top;
    
    for (int i = 0; i < node->child_count; i++) {
        RaeenUINode* child = node->children[i];
        
        child->computed_bounds.size.x = child->style.width;
        child->computed_bounds.size.y = child->style.height;
        
        if (node->style.flex_direction == RAEENUI_FLEX_ROW) {
            child->computed_bounds.position.x = current_x;
            child->computed_bounds.position.y = current_y;
            current_x += child->computed_bounds.size.x + child->style.margin_right;
        } else {
            child->computed_bounds.position.x = current_x;
            child->computed_bounds.position.y = current_y;
            current_y += child->computed_bounds.size.y + child->style.margin_bottom;
        }
    }
}

void raeenui_layout(RaeenUIContext* ctx, RaeenUINode* root) {
    if (!ctx || !root) return;
    
    // Set root bounds to screen size
    if (root == ctx->root) {
        root->computed_bounds.position.x = 0;
        root->computed_bounds.position.y = 0;
        root->computed_bounds.size.x = (float)ctx->screen_width;
        root->computed_bounds.size.y = (float)ctx->screen_height;
    }
    
    // Layout current node
    switch (root->style.layout_type) {
        case RAEENUI_LAYOUT_ABSOLUTE:
            layout_absolute(root);
            break;
        case RAEENUI_LAYOUT_FLEX:
            layout_flex(root);
            break;
        // Add more layout types as needed
        default:
            layout_absolute(root);
            break;
    }
    
    // Layout children recursively
    for (int i = 0; i < root->child_count; i++) {
        raeenui_layout(ctx, root->children[i]);
    }
    
    root->needs_layout = false;
}

void raeenui_invalidate_layout(RaeenUINode* node) {
    if (!node) return;
    
    node->needs_layout = true;
    node->needs_repaint = true;
    
    // Propagate up to root
    if (node->parent) {
        raeenui_invalidate_layout(node->parent);
    }
}

// ============================================================================
// RENDERING PIPELINE (GPU-Accelerated)
// ============================================================================

static uint32_t color_to_rgba(RaeenUIColor color) {
    uint8_t r = (uint8_t)(color.r * 255.0f);
    uint8_t g = (uint8_t)(color.g * 255.0f);
    uint8_t b = (uint8_t)(color.b * 255.0f);
    uint8_t a = (uint8_t)(color.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static void render_node(RaeenUIContext* ctx, RaeenUINode* node) {
    if (!node || !node->is_visible) return;
    
    // Calculate final opacity
    float opacity = node->style.opacity;
    if (node->parent) {
        opacity *= node->parent->computed_opacity;
    }
    node->computed_opacity = opacity;
    
    // Skip if fully transparent
    if (opacity <= 0.0f) return;
    
    // Apply opacity to background color
    RaeenUIColor bg_color = node->style.background_color;
    bg_color.a *= opacity;
    
    // Render background
    if (bg_color.a > 0.0f) {
        uint32_t rgba = color_to_rgba(bg_color);
        
        if (node->style.border_radius > 0.0f) {
            // Render rounded rectangle (simplified)
            // TODO: Implement proper rounded rectangle rendering
            graphics_fill_rect(
                (uint32_t)node->computed_bounds.position.x,
                (uint32_t)node->computed_bounds.position.y,
                (uint32_t)node->computed_bounds.size.x,
                (uint32_t)node->computed_bounds.size.y,
                rgba
            );
        } else {
            graphics_fill_rect(
                (uint32_t)node->computed_bounds.position.x,
                (uint32_t)node->computed_bounds.position.y,
                (uint32_t)node->computed_bounds.size.x,
                (uint32_t)node->computed_bounds.size.y,
                rgba
            );
        }
    }
    
    // Render border
    if (node->style.border_width > 0.0f && node->style.border_color.a > 0.0f) {
        RaeenUIColor border_color = node->style.border_color;
        border_color.a *= opacity;
        uint32_t border_rgba = color_to_rgba(border_color);
        
        graphics_draw_rect(
            (uint32_t)node->computed_bounds.position.x,
            (uint32_t)node->computed_bounds.position.y,
            (uint32_t)node->computed_bounds.size.x,
            (uint32_t)node->computed_bounds.size.y,
            border_rgba
        );
    }
    
    // Component-specific rendering
    switch (node->type) {
        case RAEENUI_COMPONENT_TEXT: {
            // Render text (placeholder implementation)
            if (node->component_data) {
                char* text = (char*)node->component_data;
                RaeenUIColor text_color = node->style.text_color;
                text_color.a *= opacity;
                uint32_t text_rgba = color_to_rgba(text_color);
                
                graphics_draw_string(
                    (uint32_t)(node->computed_bounds.position.x + 5),
                    (uint32_t)(node->computed_bounds.position.y + 5),
                    text,
                    text_rgba
                );
            }
            break;
        }
        // Add more component-specific rendering
        default:
            break;
    }
    
    // Render children
    for (int i = 0; i < node->child_count; i++) {
        render_node(ctx, node->children[i]);
    }
    
    node->needs_repaint = false;
}

void raeenui_render(RaeenUIContext* ctx, RaeenUINode* root) {
    if (!ctx || !root) return;
    
    // Update layout if needed
    if (root->needs_layout) {
        raeenui_layout(ctx, root);
    }
    
    // Clear screen
    graphics_clear_screen(0x00000000);
    
    // Render tree
    render_node(ctx, root);
    
    ctx->frame_count++;
}

void raeenui_present(RaeenUIContext* ctx) {
    if (!ctx) return;
    
    // Present to screen
    graphics_swap_buffers();
}

// ============================================================================
// EVENT SYSTEM
// ============================================================================

void raeenui_add_event_handler(RaeenUINode* node, RaeenUIEventType type, RaeenUIEventHandler handler) {
    if (!node || !handler) return;
    
    // Expand handlers array if needed
    node->event_handlers = (RaeenUIEventHandler*)realloc(node->event_handlers, 
        sizeof(RaeenUIEventHandler) * (node->handler_count + 1));
    
    node->event_handlers[node->handler_count++] = handler;
}

void raeenui_dispatch_event(RaeenUIContext* ctx, RaeenUIEvent* event) {
    if (!ctx || !event) return;
    
    // Add event to queue
    if (ctx->event_count < ctx->event_capacity) {
        ctx->event_queue[ctx->event_count++] = *event;
    }
    
    // TODO: Implement proper event dispatching with hit testing
    // For now, just call handlers on all nodes
    for (int i = 0; i < ctx->node_pool.count; i++) {
        RaeenUINode* node = ctx->node_pool.nodes[i];
        for (int j = 0; j < node->handler_count; j++) {
            if (node->event_handlers[j](event, node)) {
                break; // Event consumed
            }
        }
    }
}

// ============================================================================
// ANIMATION SYSTEM (Placeholder)
// ============================================================================

RaeenUIAnimation* raeenui_create_animation(RaeenUIId target_id, const char* property, 
                                          float from, float to, float duration) {
    RaeenUIAnimation* anim = (RaeenUIAnimation*)raeenui_alloc(sizeof(RaeenUIAnimation));
    if (!anim) return NULL;
    
    anim->target_id = target_id;
    anim->property_name = strdup(property);
    anim->from_value = from;
    anim->to_value = to;
    anim->duration = duration;
    anim->delay = 0.0f;
    anim->curve = RAEENUI_ANIM_LINEAR;
    anim->repeat = false;
    anim->reverse = false;
    anim->on_complete = NULL;
    
    return anim;
}

void raeenui_start_animation(RaeenUIContext* ctx, RaeenUIAnimation* animation) {
    if (!ctx || !animation) return;
    
    // Add to animation pool
    if (ctx->animation_pool.count >= ctx->animation_pool.capacity) {
        ctx->animation_pool.capacity *= 2;
        ctx->animation_pool.animations = (RaeenUIAnimation**)realloc(ctx->animation_pool.animations,
            sizeof(RaeenUIAnimation*) * ctx->animation_pool.capacity);
    }
    
    ctx->animation_pool.animations[ctx->animation_pool.count++] = animation;
}

void raeenui_stop_animation(RaeenUIContext* ctx, RaeenUIAnimation* animation) {
    if (!ctx || !animation) return;
    
    // Remove from animation pool
    for (int i = 0; i < ctx->animation_pool.count; i++) {
        if (ctx->animation_pool.animations[i] == animation) {
            // Swap with last element
            ctx->animation_pool.animations[i] = ctx->animation_pool.animations[ctx->animation_pool.count - 1];
            ctx->animation_pool.count--;
            break;
        }
    }
}

// ============================================================================
// AI INTEGRATION (Placeholder)
// ============================================================================

void raeenui_set_ai_context(RaeenUINode* node, const char* context) {
    if (!node || !context) return;
    
    if (node->ai_context) {
        raeenui_free(node->ai_context);
    }
    
    node->ai_context = strdup(context);
}

void raeenui_enable_ai_suggestions(RaeenUIContext* ctx, bool enable) {
    if (!ctx) return;
    ctx->ai_suggestions_enabled = enable;
}

void raeenui_process_ai_feedback(RaeenUIContext* ctx, const char* feedback) {
    if (!ctx || !feedback) return;
    
    // TODO: Process AI feedback and apply suggestions
    // This would integrate with Rae AI system
}

// ============================================================================
// PERFORMANCE OPTIMIZATION
// ============================================================================

void raeenui_enable_gpu_acceleration(RaeenUIContext* ctx, bool enable) {
    if (!ctx) return;
    ctx->gpu_acceleration_enabled = enable;
}

void raeenui_set_vsync(RaeenUIContext* ctx, bool enable) {
    if (!ctx) return;
    ctx->vsync_enabled = enable;
}

void raeenui_set_target_fps(RaeenUIContext* ctx, int fps) {
    if (!ctx) return;
    ctx->target_fps = fps;
}

void raeenui_gc_collect(RaeenUIContext* ctx) {
    if (!ctx) return;
    
    // TODO: Implement garbage collection for unused nodes
    // Mark and sweep algorithm would work well here
}

void raeenui_optimize_tree(RaeenUIContext* ctx, RaeenUINode* root) {
    if (!ctx || !root) return;
    
    // TODO: Implement tree optimization
    // - Flatten unnecessary nesting
    // - Combine adjacent similar nodes
    // - Remove invisible nodes
}