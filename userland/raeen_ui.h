#ifndef RAEEN_UI_H
#define RAEEN_UI_H

#include <stdint.h>

// Basic UI element types
typedef enum {
    RAEEN_UI_ELEMENT_BUTTON,
    RAEEN_UI_ELEMENT_LABEL,
    RAEEN_UI_ELEMENT_TEXTBOX
} raeen_ui_element_type_t;

// Basic UI element structure
typedef struct {
    raeen_ui_element_type_t type;
    int x, y, width, height;
    char* text;
    // Add event handlers, styling, etc.
} raeen_ui_element_t;

// Initialize the RaeenOS UI framework
void raeen_ui_init(void);

// Create a UI element
raeen_ui_element_t* raeen_ui_create_element(raeen_ui_element_type_t type, int x, int y, int width, int height, const char* text);

// Draw a UI element
void raeen_ui_draw_element(raeen_ui_element_t* element);

#endif // RAEEN_UI_H
