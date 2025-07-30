// RaeenOS UI Toolkit - Widget Implementation

#include "widget.h"
#include "../window.h"
#include "../memory/pmm.h"
#include "../string.h"
#include <stddef.h>
#include "terminal.h" // For draw_terminal

// Forward declarations for internal drawing functions
static void draw_button(widget_t* widget, window_t* parent);
static void draw_label(widget_t* widget, window_t* parent);
static void draw_textbox(widget_t* widget, window_t* parent);
static void draw_layout(layout_t* layout, window_t* parent);
static void draw_slider(widget_t* widget, window_t* parent);

/**
 * @brief Creates a new widget and adds it to a parent window.
 */
widget_t* widget_create(window_t* parent, widget_type_t type, int x, int y, int width, int height, const char* text) {
    if (!parent) return NULL;

    widget_t* widget = (widget_t*)pmm_alloc_frame();
    if (!widget) return NULL;
    memset(widget, 0, sizeof(widget_t));

    widget->type = type;
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;

    if (text) {
        size_t text_len = strlen(text) + 1;
        widget->text = (char*)kmalloc(text_len);
        if (widget->text) {
            memcpy(widget->text, text, text_len);
        }
    }

    // Assign the correct draw function based on type
    switch (type) {
        case WIDGET_TYPE_BUTTON:
            widget->draw = draw_button;
            break;
        case WIDGET_TYPE_LABEL:
            widget->draw = draw_label;
            break;
        case WIDGET_TYPE_TEXTBOX:
            widget->draw = draw_textbox;
            break;
        case WIDGET_TYPE_TERMINAL:
            widget->draw = draw_terminal;
            break;
        case WIDGET_TYPE_SLIDER:
            widget->draw = draw_slider;
            break;
        default:
            widget->draw = NULL;
            break;
    }

    // Add to the parent's widget list
    widget->next = parent->widgets;
    parent->widgets = widget;

    return widget;
}

/**
 * @brief Draws a single button widget.
 */
static void draw_button(widget_t* widget, window_t* parent) {
    // Draw button background
    window_draw_rect(parent, widget->x, widget->y, widget->width, widget->height, 0x00AAAAAA);

    // Draw button text, centered
    if (widget->text) {
        int text_len = strlen(widget->text);
        int text_x = widget->x + (widget->width - (text_len * FONT_WIDTH)) / 2;
        int text_y = widget->y + (widget->height - FONT_HEIGHT) / 2;
        window_draw_string(parent, text_x, text_y, widget->text, 0x00000000);
    }
}

/**
 * @brief Draws a label widget.
 */
static void draw_label(widget_t* widget, window_t* parent) {
    if (widget->text) {
        window_draw_string(parent, widget->x, widget->y, widget->text, 0xFFFFFFFF);
    }
}

/**
 * @brief Draws a textbox widget.
 */
static void draw_textbox(widget_t* widget, window_t* parent) {
    // Draw textbox background
    window_draw_rect(parent, widget->x, widget->y, widget->width, widget->height, 0xFFFFFFFF);

    // Draw textbox border
    window_draw_rect(parent, widget->x, widget->y, widget->width, 1, 0x00000000);
    window_draw_rect(parent, widget->x, widget->y + widget->height - 1, widget->width, 1, 0x00000000);
    window_draw_rect(parent, widget->x, widget->y, 1, widget->height, 0x00000000);
    window_draw_rect(parent, widget->x + widget->width - 1, widget->y, 1, widget->height, 0x00000000);

    // Draw textbox text
    if (widget->text) {
        window_draw_string(parent, widget->x + 2, widget->y + 2, widget->text, 0x00000000);
    }
}

/**
 * @brief Draws a slider widget.
 */
static void draw_slider(widget_t* widget, window_t* parent) {
    // Draw slider track
    window_draw_rect(parent, widget->x, widget->y + widget->height / 2 - 1, widget->width, 2, 0x00888888);

    // Draw slider thumb (placeholder)
    window_draw_rect(parent, widget->x + widget->width / 2 - 5, widget->y, 10, widget->height, 0x00CCCCCC);
}

/**
 * @brief Draws a single widget.
 */
void widget_draw(widget_t* widget, window_t* parent) {
    if (!widget || !parent || !widget->draw) return;

    // Call the widget's specific draw function
    widget->draw(widget, parent);
}

/**
 * @brief Draws a layout and its contained widgets.
 */
static void draw_layout(layout_t* layout, window_t* parent) {
    if (!layout || !parent) return;

    int current_y = layout->padding;
    widget_t* current_widget = layout->widgets;

    while (current_widget) {
        // For now, only vertical box layout
        current_widget->y = current_y;
        current_widget->x = layout->padding;

        widget_draw(current_widget, parent);
        current_y += current_widget->height + layout->spacing;
        current_widget = current_widget->next;
    }
}

/**
 * @brief Draws all widgets associated with a window.
 */
void widget_draw_all(window_t* parent) {
    if (!parent) return;

    // If the window has a layout, draw it
    if (parent->layout) {
        draw_layout(parent->layout, parent);
    } else {
        // Otherwise, draw individual widgets
        widget_t* current = parent->widgets;
        while (current) {
            widget_draw(current, parent);
            current = current->next;
        }
    }
}

/**
 * @brief Finds a widget within a window at the given window-local coordinates.
 */
widget_t* widget_find_at_coords(window_t* parent, int x, int y) {
    if (!parent) return NULL;

    widget_t* current = parent->widgets;
    while (current) {
        if (x >= current->x && x < (current->x + current->width) &&
            y >= current->y && y < (current->y + current->height)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}