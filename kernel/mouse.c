// RaeenOS PS/2 Mouse Driver

#include "mouse.h"
#include "idt.h"
#include "ports.h"
#include "pic.h"
#include "include/event.h"
#include "graphics.h"

#define MOUSE_DATA_PORT   0x60
#define MOUSE_CMD_PORT    0x64

// Internal state of the mouse driver
static struct mouse_state_t g_mouse_state;
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];

// Helper functions to wait for mouse communication
static void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) {
            if ((port_byte_in(MOUSE_CMD_PORT) & 1) == 1) return;
        }
        return;
    } else {
        while (timeout--) {
            if ((port_byte_in(MOUSE_CMD_PORT) & 2) == 0) return;
        }
        return;
    }
}

// Helper function to write to the mouse
static void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    port_byte_out(MOUSE_CMD_PORT, 0xD4);
    mouse_wait(1);
    port_byte_out(MOUSE_DATA_PORT, a_write);
}

// Helper function to read from the mouse
static uint8_t mouse_read() {
    mouse_wait(0);
    return port_byte_in(MOUSE_DATA_PORT);
}

static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];

// Helper functions to wait for mouse communication
static void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) {
            if ((port_byte_in(MOUSE_CMD_PORT) & 1) == 1) return;
        }
        return;
    } else {
        while (timeout--) {
            if ((port_byte_in(MOUSE_CMD_PORT) & 2) == 0) return;
        }
        return;
    }
}

// Helper function to write to the mouse
static void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    port_byte_out(MOUSE_CMD_PORT, 0xD4);
    mouse_wait(1);
    port_byte_out(MOUSE_DATA_PORT, a_write);
}

// Helper function to read from the mouse
static uint8_t mouse_read() {
    mouse_wait(0);
    return port_byte_in(MOUSE_DATA_PORT);
}

/**
 * @brief The main mouse interrupt handler (IRQ12).
 */
static void mouse_handler(struct registers_t* regs) {
    (void)regs;
    uint8_t data = port_byte_in(MOUSE_DATA_PORT);
    mouse_process_packet(data);
}

/**
 * @brief Initializes the PS/2 mouse driver.
 */
void mouse_init(void) {
    uint8_t status;

    // Initial position in the center of the screen
    g_mouse_state.x = graphics_get_width() / 2;
    g_mouse_state.y = graphics_get_height() / 2;

    // Enable the auxiliary mouse device
    mouse_wait(1);
    port_byte_out(MOUSE_CMD_PORT, 0xA8);

    // Enable the interrupts
    mouse_wait(1);
    port_byte_out(MOUSE_CMD_PORT, 0x20);
    mouse_wait(0);
    status = (port_byte_in(MOUSE_DATA_PORT) | 2);
    mouse_wait(1);
    port_byte_out(MOUSE_CMD_PORT, 0x60);
    mouse_wait(1);
    port_byte_out(MOUSE_DATA_PORT, status);

    // Set mouse to use default settings
    mouse_write(0xF6);
    mouse_read(); // Acknowledge

    // Enable the mouse
    mouse_write(0xF4);
    mouse_read(); // Acknowledge

    // Register the handler for IRQ12
    register_interrupt_handler(IRQ_TO_INT(12), mouse_handler);
}

/**
 * @brief Gets the current state of the mouse.
 */
void mouse_get_state(mouse_state_t* state) {
    // This should be atomic in a real multitasking OS
    *state = g_mouse_state;
}

void mouse_process_packet(uint8_t data) {
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = data;
            // Check for valid first byte (bit 3 must be 1)
            if (mouse_byte[0] & 0x08) {
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;

            // Update button states
            g_mouse_state.left_button = mouse_byte[0] & 0x1;
            g_mouse_state.right_button = (mouse_byte[0] & 0x2) >> 1;
            g_mouse_state.middle_button = (mouse_byte[0] & 0x4) >> 2;

            // Update position
            int32_t delta_x = mouse_byte[1];
            int32_t delta_y = mouse_byte[2];

            // Handle sign extension for negative movement
            if (mouse_byte[0] & 0x10) delta_x |= 0xFFFFFF00;
            if (mouse_byte[0] & 0x20) delta_y |= 0xFFFFFF00;

            g_mouse_state.x += delta_x;
            g_mouse_state.y -= delta_y; // Y is inverted in mouse packets

            // Clamp cursor to screen bounds
            int32_t width = (int32_t)graphics_get_width();
            int32_t height = (int32_t)graphics_get_height();
            if (g_mouse_state.x < 0) g_mouse_state.x = 0;
            if (g_mouse_state.y < 0) g_mouse_state.y = 0;
            if (g_mouse_state.x >= width) g_mouse_state.x = width - 1;
            if (g_mouse_state.y >= height) g_mouse_state.y = height - 1;

            // Push mouse move event
            event_t event;
            event.type = EVENT_MOUSE_MOVE;
            event.data.mouse.x = g_mouse_state.x;
            event.data.mouse.y = g_mouse_state.y;
            event.data.mouse.buttons = (g_mouse_state.left_button ? 1 : 0) | \
                                       (g_mouse_state.right_button ? 2 : 0) | \
                                       (g_mouse_state.middle_button ? 4 : 0);
            event_queue_push(event);

            // Redraw the cursor
            graphics_draw_cursor(g_mouse_state.x, g_mouse_state.y, 0xFFFFFFFF); // White cursor

            // Basic gesture detection (example: simple swipe)
            if (delta_x > 50) {
                gesture_event_t gesture;
                gesture.type = GESTURE_SWIPE_RIGHT;
                gesture.delta_x = delta_x;
                gesture.delta_y = delta_y;
                // event_queue_push_gesture(gesture); // Assuming a separate queue for gestures
            } else if (delta_x < -50) {
                gesture_event_t gesture;
                gesture.type = GESTURE_SWIPE_LEFT;
                gesture.delta_x = delta_x;
                gesture.delta_y = delta_y;
                // event_queue_push_gesture(gesture); 
            }
            break;
    }
}
