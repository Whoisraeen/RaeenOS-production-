/**
 * RaeenUI Components Implementation
 * Declarative UI Primitives with Gaming-Grade Performance
 */

#include "components.h"
#include "../../kernel/memory.h"
#include "../../kernel/string.h"

// ============================================================================
// COMPONENT DATA STRUCTURES
// ============================================================================

typedef struct {
    RaeenUIViewProps props;
} RaeenUIViewData;

typedef struct {
    RaeenUITextProps props;
    char* computed_text;  // For text processing/localization
} RaeenUITextData;

typedef struct {
    RaeenUIButtonProps props;
    enum {
        RAEENUI_BUTTON_NORMAL,
        RAEENUI_BUTTON_HOVER,
        RAEENUI_BUTTON_PRESSED
    } state;
    float animation_progress;
} RaeenUIButtonData;

typedef struct {
    RaeenUIInputProps props;
    char* current_text;
    int cursor_position;
    bool is_focused;
    float cursor_blink_time;
} RaeenUIInputData;

typedef struct {
    RaeenUIStackProps props;
} RaeenUIStackData;

typedef struct {
    RaeenUIWindowProps props;
    bool is_dragging;
    bool is_resizing;
    RaeenUIVec2 drag_offset;
    RaeenUIVec2 original_size;
} RaeenUIWindowData;

// ============================================================================
// BASIC COMPONENTS
// ============================================================================

RaeenUINode* RaeenUI_View(RaeenUIContext* ctx, RaeenUIViewProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_VIEW);
    if (!node) return NULL;
    
    // Store component data
    RaeenUIViewData* data = (RaeenUIViewData*)kmalloc(sizeof(RaeenUIViewData));
    data->props = props;
    node->component_data = data;
    
    // Apply style properties
    node->style.background_color = props.background_color;
    node->style.border_radius = props.corner_radius;
    node->style.border_width = props.border_width;
    node->style.border_color = props.border_color;
    
    return node;
}

RaeenUINode* RaeenUI_Text(RaeenUIContext* ctx, RaeenUITextProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_TEXT);
    if (!node) return NULL;
    
    // Store component data
    RaeenUITextData* data = (RaeenUITextData*)kmalloc(sizeof(RaeenUITextData));
    data->props = props;
    data->computed_text = strdup(props.text ? props.text : "");
    node->component_data = data;
    
    // Apply text styling
    node->style.text_color = props.color;
    node->style.font_size = props.font_size;
    if (props.font_family) {
        node->style.font_family = strdup(props.font_family);
    }
    
    // Auto-size based on text content (simplified)
    int text_len = strlen(data->computed_text);
    node->style.width = text_len * props.font_size * 0.6f;  // Rough estimate
    node->style.height = props.font_size * 1.2f;
    
    return node;
}

static bool button_event_handler(RaeenUIEvent* event, void* user_data) {
    RaeenUINode* node = (RaeenUINode*)user_data;
    RaeenUIButtonData* data = (RaeenUIButtonData*)node->component_data;
    
    switch (event->type) {
        case RAEENUI_EVENT_MOUSE_DOWN:
            data->state = RAEENUI_BUTTON_PRESSED;
            node->needs_repaint = true;
            return true;
            
        case RAEENUI_EVENT_MOUSE_UP:
            if (data->state == RAEENUI_BUTTON_PRESSED) {
                data->state = RAEENUI_BUTTON_NORMAL;
                node->needs_repaint = true;
                
                // Call user's click handler
                if (data->props.on_click) {
                    data->props.on_click(event, data->props.user_data);
                }
            }
            return true;
            
        case RAEENUI_EVENT_HOVER_ENTER:
            data->state = RAEENUI_BUTTON_HOVER;
            node->needs_repaint = true;
            return true;
            
        case RAEENUI_EVENT_HOVER_EXIT:
            data->state = RAEENUI_BUTTON_NORMAL;
            node->needs_repaint = true;
            return true;
            
        default:
            return false;
    }
}

RaeenUINode* RaeenUI_Button(RaeenUIContext* ctx, RaeenUIButtonProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_BUTTON);
    if (!node) return NULL;
    
    // Store component data
    RaeenUIButtonData* data = (RaeenUIButtonData*)kmalloc(sizeof(RaeenUIButtonData));
    data->props = props;
    data->state = RAEENUI_BUTTON_NORMAL;
    data->animation_progress = 0.0f;
    node->component_data = data;
    
    // Apply styling based on state
    node->style.background_color = props.background_color;
    node->style.border_radius = props.corner_radius;
    node->style.text_color = props.text_color;
    
    // Add event handler
    raeenui_add_event_handler(node, RAEENUI_EVENT_MOUSE_DOWN, button_event_handler);
    raeenui_add_event_handler(node, RAEENUI_EVENT_MOUSE_UP, button_event_handler);
    raeenui_add_event_handler(node, RAEENUI_EVENT_HOVER_ENTER, button_event_handler);
    raeenui_add_event_handler(node, RAEENUI_EVENT_HOVER_EXIT, button_event_handler);
    
    // Create text child
    if (props.title) {
        RaeenUITextProps text_props = {
            .text = props.title,
            .font_size = 16.0f,
            .color = props.text_color,
            .text_align = RAEENUI_TEXT_ALIGN_CENTER,
            .bold = false,
            .italic = false
        };
        RaeenUINode* text_node = RaeenUI_Text(ctx, text_props);
        raeenui_add_child(node, text_node);
        
        // Center the text
        text_node->style.layout_type = RAEENUI_LAYOUT_ABSOLUTE;
        text_node->style.left = 10.0f;
        text_node->style.top = 10.0f;
    }
    
    return node;
}

static bool input_event_handler(RaeenUIEvent* event, void* user_data) {
    RaeenUINode* node = (RaeenUINode*)user_data;
    RaeenUIInputData* data = (RaeenUIInputData*)node->component_data;
    
    switch (event->type) {
        case RAEENUI_EVENT_MOUSE_DOWN:
            data->is_focused = true;
            data->cursor_blink_time = 0.0f;
            node->needs_repaint = true;
            return true;
            
        case RAEENUI_EVENT_KEY_DOWN:
            if (data->is_focused && event->data.keyboard.text) {
                // Simple text input handling
                int current_len = strlen(data->current_text);
                if (current_len < data->props.max_length - 1) {
                    // Insert character at cursor position
                    for (int i = current_len; i >= data->cursor_position; i--) {
                        data->current_text[i + 1] = data->current_text[i];
                    }
                    data->current_text[data->cursor_position] = event->data.keyboard.text[0];
                    data->cursor_position++;
                    
                    // Call change handler
                    if (data->props.on_change) {
                        data->props.on_change(data->current_text, data->props.user_data);
                    }
                    
                    node->needs_repaint = true;
                }
            }
            return true;
            
        default:
            return false;
    }
}

RaeenUINode* RaeenUI_Input(RaeenUIContext* ctx, RaeenUIInputProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_INPUT);
    if (!node) return NULL;
    
    // Store component data
    RaeenUIInputData* data = (RaeenUIInputData*)kmalloc(sizeof(RaeenUIInputData));
    data->props = props;
    data->current_text = (char*)kmalloc(props.max_length);
    strcpy(data->current_text, props.value ? props.value : "");
    data->cursor_position = strlen(data->current_text);
    data->is_focused = false;
    data->cursor_blink_time = 0.0f;
    node->component_data = data;
    
    // Apply styling
    node->style.background_color = props.background_color;
    node->style.border_color = props.border_color;
    node->style.border_width = 1.0f;
    node->style.border_radius = props.corner_radius;
    node->style.padding_left = 8.0f;
    node->style.padding_right = 8.0f;
    node->style.padding_top = 4.0f;
    node->style.padding_bottom = 4.0f;
    
    // Add event handlers
    raeenui_add_event_handler(node, RAEENUI_EVENT_MOUSE_DOWN, input_event_handler);
    raeenui_add_event_handler(node, RAEENUI_EVENT_KEY_DOWN, input_event_handler);
    
    return node;
}

RaeenUINode* RaeenUI_Image(RaeenUIContext* ctx, RaeenUIImageProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_IMAGE);
    if (!node) return NULL;
    
    // Store image data
    node->component_data = kmalloc(sizeof(RaeenUIImageProps));
    *(RaeenUIImageProps*)node->component_data = props;
    
    // Set size based on image dimensions
    node->style.width = (float)props.width;
    node->style.height = (float)props.height;
    node->style.opacity = props.opacity;
    node->style.border_radius = props.corner_radius;
    
    return node;
}

// ============================================================================
// LAYOUT COMPONENTS
// ============================================================================

RaeenUINode* RaeenUI_Stack(RaeenUIContext* ctx, RaeenUIStackProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_STACK);
    if (!node) return NULL;
    
    // Store stack data
    RaeenUIStackData* data = (RaeenUIStackData*)kmalloc(sizeof(RaeenUIStackData));
    data->props = props;
    node->component_data = data;
    
    // Configure layout
    node->style.layout_type = RAEENUI_LAYOUT_FLEX;
    node->style.flex_direction = (props.direction == RAEENUI_STACK_VERTICAL) ? 
        RAEENUI_FLEX_COLUMN : RAEENUI_FLEX_ROW;
    node->style.align_items = props.alignment;
    
    // Apply padding
    float padding = props.padding;
    node->style.padding_top = padding;
    node->style.padding_right = padding;
    node->style.padding_bottom = padding;
    node->style.padding_left = padding;
    
    return node;
}

RaeenUINode* RaeenUI_Grid(RaeenUIContext* ctx, RaeenUIGridProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_GRID);
    if (!node) return NULL;
    
    // Store grid data
    node->component_data = kmalloc(sizeof(RaeenUIGridProps));
    *(RaeenUIGridProps*)node->component_data = props;
    
    // Configure grid layout
    node->style.layout_type = RAEENUI_LAYOUT_GRID;
    
    return node;
}

RaeenUINode* RaeenUI_Flex(RaeenUIContext* ctx, RaeenUIFlexProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_FLEX);
    if (!node) return NULL;
    
    // Store flex data
    node->component_data = kmalloc(sizeof(RaeenUIFlexProps));
    *(RaeenUIFlexProps*)node->component_data = props;
    
    // Configure flex layout
    node->style.layout_type = RAEENUI_LAYOUT_FLEX;
    node->style.flex_direction = props.direction;
    node->style.justify_content = props.justify_content;
    node->style.align_items = props.align_items;
    
    return node;
}

// ============================================================================
// ADVANCED COMPONENTS
// ============================================================================

static bool window_event_handler(RaeenUIEvent* event, void* user_data) {
    RaeenUINode* node = (RaeenUINode*)user_data;
    RaeenUIWindowData* data = (RaeenUIWindowData*)node->component_data;
    
    switch (event->type) {
        case RAEENUI_EVENT_MOUSE_DOWN:
            // Check if clicking on title bar for dragging
            if (event->position.y < 30.0f) {  // Title bar height
                data->is_dragging = true;
                data->drag_offset.x = event->position.x - node->computed_bounds.position.x;
                data->drag_offset.y = event->position.y - node->computed_bounds.position.y;
            }
            return true;
            
        case RAEENUI_EVENT_MOUSE_MOVE:
            if (data->is_dragging) {
                node->style.left = event->position.x - data->drag_offset.x;
                node->style.top = event->position.y - data->drag_offset.y;
                node->needs_layout = true;
            }
            return true;
            
        case RAEENUI_EVENT_MOUSE_UP:
            data->is_dragging = false;
            data->is_resizing = false;
            return true;
            
        default:
            return false;
    }
}

RaeenUINode* RaeenUI_Window(RaeenUIContext* ctx, RaeenUIWindowProps props) {
    RaeenUINode* node = raeenui_create_component(ctx, RAEENUI_COMPONENT_WINDOW);
    if (!node) return NULL;
    
    // Store window data
    RaeenUIWindowData* data = (RaeenUIWindowData*)kmalloc(sizeof(RaeenUIWindowData));
    data->props = props;
    data->is_dragging = false;
    data->is_resizing = false;
    node->component_data = data;
    
    // Window styling
    node->style.background_color = RAEENUI_COLOR_RGB(240, 240, 240);
    node->style.border_width = 1.0f;
    node->style.border_color = RAEENUI_COLOR_RGB(180, 180, 180);
    node->style.border_radius = 8.0f;
    
    // Add event handlers
    raeenui_add_event_handler(node, RAEENUI_EVENT_MOUSE_DOWN, window_event_handler);
    raeenui_add_event_handler(node, RAEENUI_EVENT_MOUSE_MOVE, window_event_handler);
    raeenui_add_event_handler(node, RAEENUI_EVENT_MOUSE_UP, window_event_handler);
    
    // Create title bar
    if (props.title) {
        RaeenUINode* title_bar = RaeenUI_View(ctx, (RaeenUIViewProps){
            .background_color = props.title_bar_color,
            .corner_radius = 8.0f,
            .border_width = 0.0f
        });
        title_bar->style.height = 30.0f;
        title_bar->style.width = node->style.width;
        title_bar->style.top = 0.0f;
        title_bar->style.left = 0.0f;
        
        // Title text
        RaeenUINode* title_text = RaeenUI_Text(ctx, (RaeenUITextProps){
            .text = props.title,
            .font_size = 14.0f,
            .color = props.title_text_color,
            .text_align = RAEENUI_TEXT_ALIGN_CENTER,
            .bold = true
        });
        title_text->style.left = 10.0f;
        title_text->style.top = 8.0f;
        
        raeenui_add_child(title_bar, title_text);
        raeenui_add_child(node, title_bar);
    }
    
    return node;
}

// ============================================================================
// STYLE BUILDER IMPLEMENTATION
// ============================================================================

RaeenUIStyleBuilder* RaeenUI_Style(void) {
    RaeenUIStyleBuilder* builder = (RaeenUIStyleBuilder*)kmalloc(sizeof(RaeenUIStyleBuilder));
    // Initialize with defaults
    builder->style = (RaeenUIStyle){0};
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_Width(RaeenUIStyleBuilder* builder, float width) {
    if (builder) builder->style.width = width;
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_Height(RaeenUIStyleBuilder* builder, float height) {
    if (builder) builder->style.height = height;
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_Padding(RaeenUIStyleBuilder* builder, float padding) {
    if (builder) {
        builder->style.padding_top = padding;
        builder->style.padding_right = padding;
        builder->style.padding_bottom = padding;
        builder->style.padding_left = padding;
    }
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_Margin(RaeenUIStyleBuilder* builder, float margin) {
    if (builder) {
        builder->style.margin_top = margin;
        builder->style.margin_right = margin;
        builder->style.margin_bottom = margin;
        builder->style.margin_left = margin;
    }
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_BackgroundColor(RaeenUIStyleBuilder* builder, RaeenUIColor color) {
    if (builder) builder->style.background_color = color;
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_BorderRadius(RaeenUIStyleBuilder* builder, float radius) {
    if (builder) builder->style.border_radius = radius;
    return builder;
}

RaeenUIStyleBuilder* RaeenUI_Opacity(RaeenUIStyleBuilder* builder, float opacity) {
    if (builder) builder->style.opacity = opacity;
    return builder;
}

RaeenUIStyle RaeenUI_BuildStyle(RaeenUIStyleBuilder* builder) {
    if (!builder) return (RaeenUIStyle){0};
    
    RaeenUIStyle style = builder->style;
    kfree(builder);
    return style;
}

// ============================================================================
// HIGH-LEVEL SCENE BUILDERS
// ============================================================================

RaeenUINode* RaeenUI_Desktop(RaeenUIContext* ctx) {
    RaeenUINode* desktop = RaeenUI_View(ctx, (RaeenUIViewProps){
        .background_color = RAEENUI_COLOR_RGB(25, 25, 35),  // Dark theme
        .corner_radius = 0.0f,
        .border_width = 0.0f
    });
    
    // Fill entire screen
    desktop->style.width = (float)ctx->screen_width;
    desktop->style.height = (float)ctx->screen_height;
    desktop->style.left = 0.0f;
    desktop->style.top = 0.0f;
    
    return desktop;
}

RaeenUINode* RaeenUI_Taskbar(RaeenUIContext* ctx) {
    RaeenUINode* taskbar = RaeenUI_View(ctx, (RaeenUIViewProps){
        .background_color = RAEENUI_COLOR_RGBA(40, 40, 50, 220),  // Semi-transparent
        .corner_radius = 0.0f,
        .border_width = 0.0f
    });
    
    // Position at bottom
    taskbar->style.width = (float)ctx->screen_width;
    taskbar->style.height = 48.0f;
    taskbar->style.left = 0.0f;
    taskbar->style.top = (float)ctx->screen_height - 48.0f;
    
    return taskbar;
}

RaeenUINode* RaeenUI_GameOverlay(RaeenUIContext* ctx) {
    RaeenUINode* overlay = RaeenUI_View(ctx, (RaeenUIViewProps){
        .background_color = RAEENUI_COLOR_TRANSPARENT,
        .corner_radius = 0.0f,
        .border_width = 0.0f
    });
    
    // Full screen overlay
    overlay->style.width = (float)ctx->screen_width;
    overlay->style.height = (float)ctx->screen_height;
    overlay->style.left = 0.0f;
    overlay->style.top = 0.0f;
    
    return overlay;
}

RaeenUINode* RaeenUI_PerformanceMonitor(RaeenUIContext* ctx) {
    RaeenUINode* monitor = RaeenUI_View(ctx, (RaeenUIViewProps){
        .background_color = RAEENUI_COLOR_RGBA(0, 0, 0, 180),
        .corner_radius = 8.0f,
        .border_width = 1.0f,
        .border_color = RAEENUI_COLOR_RGB(60, 60, 60)
    });
    
    // Position in top-right corner
    monitor->style.width = 200.0f;
    monitor->style.height = 120.0f;
    monitor->style.left = (float)ctx->screen_width - 220.0f;
    monitor->style.top = 20.0f;
    
    // Add FPS text
    RaeenUINode* fps_text = RaeenUI_Text(ctx, (RaeenUITextProps){
        .text = "FPS: 60",
        .font_size = 14.0f,
        .color = RAEENUI_COLOR_RGB(0, 255, 0),
        .text_align = RAEENUI_TEXT_ALIGN_LEFT,
        .bold = true
    });
    fps_text->style.left = 10.0f;
    fps_text->style.top = 10.0f;
    raeenui_add_child(monitor, fps_text);
    
    // Add CPU text
    RaeenUINode* cpu_text = RaeenUI_Text(ctx, (RaeenUITextProps){
        .text = "CPU: 45%",
        .font_size = 14.0f,
        .color = RAEENUI_COLOR_RGB(255, 255, 0),
        .text_align = RAEENUI_TEXT_ALIGN_LEFT,
        .bold = false
    });
    cpu_text->style.left = 10.0f;
    cpu_text->style.top = 30.0f;
    raeenui_add_child(monitor, cpu_text);
    
    // Add GPU text
    RaeenUINode* gpu_text = RaeenUI_Text(ctx, (RaeenUITextProps){
        .text = "GPU: 67%",
        .font_size = 14.0f,
        .color = RAEENUI_COLOR_RGB(255, 100, 100),
        .text_align = RAEENUI_TEXT_ALIGN_LEFT,
        .bold = false
    });
    gpu_text->style.left = 10.0f;
    gpu_text->style.top = 50.0f;
    raeenui_add_child(monitor, gpu_text);
    
    return monitor;
}

RaeenUINode* RaeenUI_GameLauncher(RaeenUIContext* ctx) {
    RaeenUINode* launcher = RaeenUI_Window(ctx, (RaeenUIWindowProps){
        .title = "RaeenOS Game Launcher",
        .resizable = true,
        .closable = true,
        .minimizable = true,
        .maximizable = true,
        .title_bar_color = RAEENUI_COLOR_RGB(30, 30, 40),
        .title_text_color = RAEENUI_COLOR_WHITE
    });
    
    launcher->style.width = 800.0f;
    launcher->style.height = 600.0f;
    launcher->style.left = 100.0f;
    launcher->style.top = 100.0f;
    
    return launcher;
}