// RaeenOS PS/2 Mouse Driver Interface

#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include "include/event.h"

// Holds the state of the mouse
struct mouse_state_t {
    int32_t x;
    int32_t y;
    uint8_t left_button;
    uint8_t right_button;
    uint8_t middle_button;
} mouse_state_t;

// Gesture types
typedef enum {
    GESTURE_NONE,
    GESTURE_SCROLL_UP,
    GESTURE_SCROLL_DOWN,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN,
    // Add more complex gestures as needed
} gesture_type_t;

// Gesture event structure
typedef struct {
    gesture_type_t type;
    int32_t delta_x;
    int32_t delta_y;
} gesture_event_t;

/**
 * @brief Initializes the PS/2 mouse driver.
 */
void mouse_init(void);

/**
 * @brief Gets the current state of the mouse.
 * 
 * @param state A pointer to a struct to be filled with the current mouse state.
 */
void mouse_get_state(mouse_state_t* state);

/**
 * @brief Processes a raw mouse data packet and updates mouse state/detects gestures.
 * 
 * @param data The raw byte data from the mouse.
 */
void mouse_process_packet(uint8_t data);

#endif // MOUSE_H
