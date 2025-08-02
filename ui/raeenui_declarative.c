/**
 * RaeenUI - Declarative UI Helpers
 * SwiftUI/React-inspired declarative interface builders
 */

#include "raeenui.h"
#include "../memory.h"
#include "../string.h"

// Builder context for chaining operations
typedef struct {
    RaeenUIView* view;
    RaeenUITheme* theme;
} RaeenUIBuilder;

// Internal helper functions
static RaeenUIBuilder raeenui_builder_create(RaeenUIView* view);
static RaeenUIView* raeenui_builder_build(RaeenUIBuilder* builder);

/**
 * Create vertical stack container
 */
RaeenUIView* raeenui_create_vstack(float spacing) {
    RaeenUIView* stack = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!stack) return NULL;
    
    stack->layout.type = RAEENUI_LAYOUT_STACK_VERTICAL;
    stack->layout.spacing = spacing;
    stack->layout.horizontal_alignment = RAEENUI_ALIGN_STRETCH;
    stack->layout.vertical_alignment = RAEENUI_ALIGN_START;
    
    string_copy(stack->identifier, "VStack", sizeof(stack->identifier));
    
    return stack;
}

/**
 * Create horizontal stack container
 */
RaeenUIView* raeenui_create_hstack(float spacing) {
    RaeenUIView* stack = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!stack) return NULL;
    
    stack->layout.type = RAEENUI_LAYOUT_STACK_HORIZONTAL;
    stack->layout.spacing = spacing;
    stack->layout.horizontal_alignment = RAEENUI_ALIGN_START;
    stack->layout.vertical_alignment = RAEENUI_ALIGN_STRETCH;
    
    string_copy(stack->identifier, "HStack", sizeof(stack->identifier));
    
    return stack;
}

/**
 * Create text view
 */
RaeenUIView* raeenui_create_text(const char* text) {
    if (!text) return NULL;
    
    RaeenUIView* text_view = raeenui_create_view(RAEENUI_VIEW_TEXT);
    if (!text_view) return NULL;
    
    // Copy text content
    size_t text_len = string_length(text);
    text_view->text_content = (char*)memory_alloc(text_len + 1);
    if (text_view->text_content) {
        string_copy(text_view->text_content, text, text_len + 1);
    }
    
    string_copy(text_view->identifier, "Text", sizeof(text_view->identifier));
    string_copy(text_view->accessibility_label, text, sizeof(text_view->accessibility_label));
    
    return text_view;
}

/**
 * Create button view
 */
RaeenUIView* raeenui_create_button(const char* title, RaeenUIEventHandler action) {
    if (!title) return NULL;
    
    RaeenUIView* button = raeenui_create_view(RAEENUI_VIEW_BUTTON);
    if (!button) return NULL;
    
    // Copy button title
    size_t title_len = string_length(title);
    button->text_content = (char*)memory_alloc(title_len + 1);
    if (button->text_content) {
        string_copy(button->text_content, title, title_len + 1);
    }
    
    // Set event handler
    if (action) {
        button->event_handlers[RAEENUI_EVENT_CLICK] = action;
    }
    
    // Default button styling
    button->style.background_color = raeenui_color_hex(0x007AFF);
    button->style.foreground_color = RAEENUI_COLOR_WHITE;
    button->style.corner_radius = 8.0f;
    button->style.padding = (RaeenUIEdgeInsets){12.0f, 24.0f, 12.0f, 24.0f};
    
    string_copy(button->identifier, "Button", sizeof(button->identifier));
    string_copy(button->accessibility_label, title, sizeof(button->accessibility_label));
    
    return button;
}

/**
 * Create image view
 */
RaeenUIView* raeenui_create_image(const char* path) {
    if (!path) return NULL;
    
    RaeenUIView* image_view = raeenui_create_view(RAEENUI_VIEW_IMAGE);
    if (!image_view) return NULL;
    
    // TODO: Load image from path
    // For now, set placeholder dimensions
    image_view->image_width = 100;
    image_view->image_height = 100;
    
    string_copy(image_view->identifier, "Image", sizeof(image_view->identifier));
    
    return image_view;
}

/**
 * Create input field
 */
RaeenUIView* raeenui_create_input(const char* placeholder) {
    RaeenUIView* input = raeenui_create_view(RAEENUI_VIEW_INPUT);
    if (!input) return NULL;
    
    // Set placeholder text
    if (placeholder) {
        size_t placeholder_len = string_length(placeholder);
        input->text_content = (char*)memory_alloc(placeholder_len + 1);
        if (input->text_content) {
            string_copy(input->text_content, placeholder, placeholder_len + 1);
        }
    }
    
    // Default input styling
    input->style.background_color = RAEENUI_COLOR_WHITE;
    input->style.foreground_color = RAEENUI_COLOR_BLACK;
    input->style.border_color = raeenui_color_hex(0xCCCCCC);
    input->style.border_width = 1.0f;
    input->style.corner_radius = 4.0f;
    input->style.padding = (RaeenUIEdgeInsets){8.0f, 12.0f, 8.0f, 12.0f};
    
    string_copy(input->identifier, "Input", sizeof(input->identifier));
    
    return input;
}

/**
 * Create scroll view
 */
RaeenUIView* raeenui_create_scroll_view(void) {
    RaeenUIView* scroll = raeenui_create_view(RAEENUI_VIEW_SCROLL);
    if (!scroll) return NULL;
    
    scroll->style.background_color = RAEENUI_COLOR_CLEAR;
    
    string_copy(scroll->identifier, "ScrollView", sizeof(scroll->identifier));
    
    return scroll;
}

/**
 * Create AI chat view
 */
RaeenUIView* raeenui_create_ai_chat(void) {
    RaeenUIView* chat = raeenui_create_view(RAEENUI_VIEW_AI_CHAT);
    if (!chat) return NULL;
    
    chat->ai_enabled = true;
    chat->style.background_color = raeenui_color_hex(0xF8F9FA);
    chat->style.corner_radius = 12.0f;
    chat->style.padding = (RaeenUIEdgeInsets){16.0f, 16.0f, 16.0f, 16.0f};
    
    string_copy(chat->identifier, "AIChat", sizeof(chat->identifier));
    string_copy(chat->ai_context, "general", sizeof(chat->ai_context));
    
    return chat;
}

/**
 * Create spacer view
 */
RaeenUIView* raeenui_create_spacer(void) {
    RaeenUIView* spacer = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!spacer) return NULL;
    
    spacer->style.background_color = RAEENUI_COLOR_CLEAR;
    spacer->layout.flex_grow = 1.0f;
    
    string_copy(spacer->identifier, "Spacer", sizeof(spacer->identifier));
    
    return spacer;
}

/**
 * Create divider view
 */
RaeenUIView* raeenui_create_divider(void) {
    RaeenUIView* divider = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!divider) return NULL;
    
    divider->style.background_color = raeenui_color_hex(0xE0E0E0);
    divider->layout.preferred_size = raeenui_size_make(0, 1); // 1px height
    
    string_copy(divider->identifier, "Divider", sizeof(divider->identifier));
    
    return divider;
}

// Modifier functions (SwiftUI-style)

/**
 * Set view frame
 */
RaeenUIView* raeenui_frame(RaeenUIView* view, float width, float height) {
    if (!view) return NULL;
    
    view->layout.preferred_size = raeenui_size_make(width, height);
    view->needs_layout = true;
    
    return view;
}

/**
 * Set view background color
 */
RaeenUIView* raeenui_background(RaeenUIView* view, RaeenUIColor color) {
    if (!view) return NULL;
    
    view->style.background_color = color;
    view->needs_render = true;
    
    return view;
}

/**
 * Set view foreground color
 */
RaeenUIView* raeenui_foreground(RaeenUIView* view, RaeenUIColor color) {
    if (!view) return NULL;
    
    view->style.foreground_color = color;
    view->needs_render = true;
    
    return view;
}

/**
 * Set view corner radius
 */
RaeenUIView* raeenui_corner_radius(RaeenUIView* view, float radius) {
    if (!view) return NULL;
    
    view->style.corner_radius = radius;
    view->needs_render = true;
    
    return view;
}

/**
 * Set view padding
 */
RaeenUIView* raeenui_padding(RaeenUIView* view, float padding) {
    if (!view) return NULL;
    
    view->style.padding = (RaeenUIEdgeInsets){padding, padding, padding, padding};
    view->needs_layout = true;
    
    return view;
}

/**
 * Set view padding with individual values
 */
RaeenUIView* raeenui_padding_edges(RaeenUIView* view, float top, float left, float bottom, float right) {
    if (!view) return NULL;
    
    view->style.padding = (RaeenUIEdgeInsets){top, left, bottom, right};
    view->needs_layout = true;
    
    return view;
}

/**
 * Set view opacity
 */
RaeenUIView* raeenui_opacity(RaeenUIView* view, float opacity) {
    if (!view) return NULL;
    
    view->style.opacity = opacity;
    view->needs_render = true;
    
    return view;
}

/**
 * Add shadow to view
 */
RaeenUIView* raeenui_shadow(RaeenUIView* view, RaeenUIColor color, float offset_x, float offset_y, float blur) {
    if (!view) return NULL;
    
    view->style.shadow_color = color;
    view->style.shadow_offset_x = offset_x;
    view->style.shadow_offset_y = offset_y;
    view->style.shadow_blur = blur;
    view->needs_render = true;
    
    return view;
}

/**
 * Add blur effect to view
 */
RaeenUIView* raeenui_blur(RaeenUIView* view, float radius) {
    if (!view) return NULL;
    
    view->style.blur_radius = radius;
    view->needs_render = true;
    
    return view;
}

/**
 * Set view font
 */
RaeenUIView* raeenui_font(RaeenUIView* view, const char* family, float size) {
    if (!view || !family) return view;
    
    string_copy(view->style.font_family, family, sizeof(view->style.font_family));
    view->style.font_size = size;
    view->needs_render = true;
    
    return view;
}

/**
 * Set view font weight
 */
RaeenUIView* raeenui_font_weight(RaeenUIView* view, bool bold) {
    if (!view) return NULL;
    
    view->style.font_bold = bold;
    view->needs_render = true;
    
    return view;
}

/**
 * Set view alignment
 */
RaeenUIView* raeenui_alignment(RaeenUIView* view, RaeenUIAlignment horizontal, RaeenUIAlignment vertical) {
    if (!view) return NULL;
    
    view->layout.horizontal_alignment = horizontal;
    view->layout.vertical_alignment = vertical;
    view->needs_layout = true;
    
    return view;
}

/**
 * Add event handler to view
 */
RaeenUIView* raeenui_on_click(RaeenUIView* view, RaeenUIEventHandler handler) {
    if (!view || !handler) return view;
    
    view->event_handlers[RAEENUI_EVENT_CLICK] = handler;
    
    return view;
}

/**
 * Add hover handler to view
 */
RaeenUIView* raeenui_on_hover(RaeenUIView* view, RaeenUIEventHandler handler) {
    if (!view || !handler) return view;
    
    view->event_handlers[RAEENUI_EVENT_HOVER] = handler;
    
    return view;
}

/**
 * Enable AI for view
 */
RaeenUIView* raeenui_ai_enabled(RaeenUIView* view, const char* context) {
    if (!view) return NULL;
    
    view->ai_enabled = true;
    if (context) {
        string_copy(view->ai_context, context, sizeof(view->ai_context));
    }
    
    return view;
}

/**
 * Set accessibility label
 */
RaeenUIView* raeenui_accessibility_label(RaeenUIView* view, const char* label) {
    if (!view || !label) return view;
    
    string_copy(view->accessibility_label, label, sizeof(view->accessibility_label));
    view->accessibility_enabled = true;
    
    return view;
}

/**
 * Set accessibility hint
 */
RaeenUIView* raeenui_accessibility_hint(RaeenUIView* view, const char* hint) {
    if (!view || !hint) return view;
    
    string_copy(view->accessibility_hint, hint, sizeof(view->accessibility_hint));
    view->accessibility_enabled = true;
    
    return view;
}

// Animation helpers

/**
 * Animate view to new frame
 */
RaeenUIAnimation* raeenui_animate_frame(RaeenUIView* view, RaeenUIRect to_frame, float duration) {
    if (!view) return NULL;
    
    RaeenUIAnimation* anim = raeenui_create_animation(view, duration);
    if (!anim) return NULL;
    
    anim->from_frame = view->frame;
    anim->to_frame = to_frame;
    
    return anim;
}

/**
 * Animate view opacity
 */
RaeenUIAnimation* raeenui_animate_opacity(RaeenUIView* view, float to_opacity, float duration) {
    if (!view) return NULL;
    
    RaeenUIAnimation* anim = raeenui_create_animation(view, duration);
    if (!anim) return NULL;
    
    anim->from_opacity = view->style.opacity;
    anim->to_opacity = to_opacity;
    
    return anim;
}

/**
 * Animate view color
 */
RaeenUIAnimation* raeenui_animate_color(RaeenUIView* view, RaeenUIColor to_color, float duration) {
    if (!view) return NULL;
    
    RaeenUIAnimation* anim = raeenui_create_animation(view, duration);
    if (!anim) return NULL;
    
    anim->from_color = view->style.background_color;
    anim->to_color = to_color;
    
    return anim;
}

// Layout helpers

/**
 * Create grid layout
 */
RaeenUIView* raeenui_create_grid(int columns, int rows, float spacing) {
    RaeenUIView* grid = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!grid) return NULL;
    
    grid->layout.type = RAEENUI_LAYOUT_GRID;
    grid->layout.spacing = spacing;
    
    string_copy(grid->identifier, "Grid", sizeof(grid->identifier));
    
    return grid;
}

/**
 * Create flex layout
 */
RaeenUIView* raeenui_create_flex(void) {
    RaeenUIView* flex = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!flex) return NULL;
    
    flex->layout.type = RAEENUI_LAYOUT_FLEX;
    
    string_copy(flex->identifier, "Flex", sizeof(flex->identifier));
    
    return flex;
}

/**
 * Set flex properties
 */
RaeenUIView* raeenui_flex(RaeenUIView* view, float grow, float shrink) {
    if (!view) return NULL;
    
    view->layout.flex_grow = grow;
    view->layout.flex_shrink = shrink;
    view->needs_layout = true;
    
    return view;
}

// Convenience macros for common UI patterns

/**
 * Create a card-like container
 */
RaeenUIView* raeenui_create_card(void) {
    RaeenUIView* card = raeenui_create_view(RAEENUI_VIEW_CONTAINER);
    if (!card) return NULL;
    
    card->style.background_color = RAEENUI_COLOR_WHITE;
    card->style.corner_radius = 12.0f;
    card->style.shadow_color = raeenui_color_rgba(0, 0, 0, 0.1f);
    card->style.shadow_offset_x = 0.0f;
    card->style.shadow_offset_y = 2.0f;
    card->style.shadow_blur = 8.0f;
    card->style.padding = (RaeenUIEdgeInsets){16.0f, 16.0f, 16.0f, 16.0f};
    
    string_copy(card->identifier, "Card", sizeof(card->identifier));
    
    return card;
}

/**
 * Create a navigation bar
 */
RaeenUIView* raeenui_create_navbar(const char* title) {
    RaeenUIView* navbar = raeenui_create_hstack(16.0f);
    if (!navbar) return NULL;
    
    navbar->style.background_color = raeenui_color_hex(0xF8F9FA);
    navbar->style.padding = (RaeenUIEdgeInsets){12.0f, 16.0f, 12.0f, 16.0f};
    navbar->layout.preferred_size = raeenui_size_make(0, 64); // Full width, 64px height
    
    if (title) {
        RaeenUIView* title_text = raeenui_create_text(title);
        if (title_text) {
            raeenui_font_weight(title_text, true);
            raeenui_font(title_text, "RaeenUI-Bold", 18.0f);
            raeenui_add_child_view(navbar, title_text);
        }
    }
    
    string_copy(navbar->identifier, "NavBar", sizeof(navbar->identifier));
    
    return navbar;
}

/**
 * Create a toolbar
 */
RaeenUIView* raeenui_create_toolbar(void) {
    RaeenUIView* toolbar = raeenui_create_hstack(8.0f);
    if (!toolbar) return NULL;
    
    toolbar->style.background_color = raeenui_color_hex(0xF0F0F0);
    toolbar->style.padding = (RaeenUIEdgeInsets){8.0f, 12.0f, 8.0f, 12.0f};
    toolbar->layout.preferred_size = raeenui_size_make(0, 48); // Full width, 48px height
    
    string_copy(toolbar->identifier, "Toolbar", sizeof(toolbar->identifier));
    
    return toolbar;
}
