// RaeenOS UI Toolkit - Widget Interface

#ifndef WIDGET_H
#define WIDGET_H

#include <stdint.h>

// Forward declaration of window_t to avoid circular dependency
struct window_t;

// Enum for different widget types
typedef enum {
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
    WIDGET_TYPE_TEXTBOX,
    WIDGET_TYPE_TERMINAL,
    WIDGET_TYPE_SLIDER
} widget_type_t;

typedef enum {
    LAYOUT_TYPE_NONE,
    LAYOUT_TYPE_VERTICAL_BOX,
} layout_type_t;

typedef struct layout {
    layout_type_t type;
    struct widget_t* widgets; // Linked list of widgets in this layout
    int padding;
    int spacing;
} layout_t;

// Generic widget structure
struct widget_t {
    widget_type_t type;
    int x, y;           // Position relative to the parent window
    int width, height;  // Size of the widget
    char* text;         // Text label for the widget
    void (*on_click)(struct widget_t* widget, int x, int y);
    void (*draw)(struct widget_t* widget, struct window_t* parent);
    struct widget_t* next;
};

typedef struct widget_t widget_t;

/**
 * @brief Creates a new widget and adds it to a window.
 *
 * @param parent The window that will contain the widget.
 * @param type The type of widget to create.
 * @param x X-position relative to the parent window.
 * @param y Y-position relative to the parent window.
 * @param width The width of the widget.
 * @param height The height of the widget.
 * @param text The text label for the widget.
 * @return widget_t* A pointer to the new widget, or NULL on failure.
 */
widget_t* widget_create(struct window_t* parent, widget_type_t type, int x, int y, int width, int height, const char* text);

/**
 * @brief Draws a single widget within its parent window.
 *
 * @param widget The widget to draw.
 * @param parent The parent window.
 */
void widget_draw(widget_t* widget, struct window_t* parent);

/**
 * @brief Draws all widgets associated with a window.
 *
 * @param parent The window whose widgets to draw.
 */
void widget_draw_all(struct window_t* parent);

/**
 * @brief Finds a widget within a window at the given window-local coordinates.
 *
 * @param parent The window to search within.
 * @param x The x-coordinate relative to the window.
 * @param y The y-coordinate relative to the window.
 * @return widget_t* A pointer to the widget, or NULL if none is found.
 */
widget_t* widget_find_at_coords(struct window_t* parent, int x, int y);

#endif // WIDGET_H
