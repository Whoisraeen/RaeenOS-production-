#ifndef EVENT_H
#define EVENT_H

#include "types.h"

// Event types
typedef enum {
    EVENT_NONE = 0,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_CLICK,
    EVENT_KEY_PRESS,
    EVENT_KEY_RELEASE,
    // Add more event types as needed
} event_type_t;

// Mouse event data
typedef struct {
    int32_t x;
    int32_t y;
    uint8_t buttons; // Bitmask for left, right, middle buttons
} mouse_event_data_t;

// Keyboard event data
typedef struct {
    uint8_t scancode;
    char ascii;
    bool pressed; // True for key press, false for key release
} keyboard_event_data_t;

// Generic event structure
typedef struct {
    event_type_t type;
    union {
        mouse_event_data_t mouse;
        keyboard_event_data_t keyboard;
    } data;
} event_t;

// Event queue functions
void event_queue_init(void);
_Bool event_queue_push(event_t event);
_Bool event_queue_pop(event_t* event);

#endif // EVENT_H
