#include "input.h"
#include "../kernel/vga.h"
#include "../kernel/keyboard.h" // For keyboard_read
#include "../kernel/mouse.h"    // For mouse_get_state

void input_init(void) {
    vga_puts("Generic Input driver initialized (placeholder).\n");
}

int input_read_event(input_event_t* event) {
    // Check for keyboard events
    char key_char;
    if (keyboard_read(&key_char, 1) > 0) {
        event->type = INPUT_EVENT_KEYBOARD;
        event->data.keyboard.ascii = key_char;
        event->data.keyboard.scancode = 0; // TODO: Get actual scancode
        event->data.keyboard.pressed = 1; // Assume key press for now
        return 1; // Event read
    }

    // Check for mouse events
    struct mouse_state_t mouse_state;
    mouse_get_state(&mouse_state);
    // For simplicity, we'll just report mouse movement and button clicks
    // A more robust implementation would track previous state to report deltas
    // and separate button press/release events.
    static int32_t last_mouse_x = 0;
    static int32_t last_mouse_y = 0;
    static uint8_t last_mouse_buttons = 0;

    if (mouse_state.x != last_mouse_x || mouse_state.y != last_mouse_y || mouse_state.buttons != last_mouse_buttons) {
        event->type = INPUT_EVENT_MOUSE;
        event->data.mouse.delta_x = mouse_state.x - last_mouse_x;
        event->data.mouse.delta_y = mouse_state.y - last_mouse_y;
        event->data.mouse.delta_z = 0; // No scroll wheel for now
        event->data.mouse.buttons = mouse_state.buttons;

        last_mouse_x = mouse_state.x;
        last_mouse_y = mouse_state.y;
        last_mouse_buttons = mouse_state.buttons;
        return 1; // Event read
    }

    return 0; // No event read
}

