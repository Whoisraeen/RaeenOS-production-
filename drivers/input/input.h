#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

// Input event types
typedef enum {
    INPUT_EVENT_NONE,
    INPUT_EVENT_KEYBOARD,
    INPUT_EVENT_MOUSE,
    INPUT_EVENT_TOUCH,
    INPUT_EVENT_TRACKPAD
} input_event_type_t;

// Keyboard event data
typedef struct {
    uint8_t scancode;
    uint8_t ascii;
    uint8_t pressed; // 1 for key down, 0 for key up
} keyboard_event_data_t;

// Mouse event data
typedef struct {
    int32_t delta_x;
    int32_t delta_y;
    int32_t delta_z; // For scroll wheel
    uint8_t buttons; // Bitmask for buttons (e.g., bit 0 for left, bit 1 for right)
} mouse_event_data_t;

// Generic input event structure
typedef struct {
    input_event_type_t type;
    union {
        keyboard_event_data_t keyboard;
        mouse_event_data_t mouse;
        // Add other event types here
    } data;
} input_event_t;

// Initialize generic input driver
void input_init(void);

// Read an input event
int input_read_event(input_event_t* event);

#endif // INPUT_H
