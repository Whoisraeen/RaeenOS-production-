// Simple PS/2 Mouse Driver for RaeenUI prototype - RaeenOS

#include "mouse_simple.h"
#include "vga.h"
#include "ports.h"

#define MOUSE_PORT_DATA    0x60
#define MOUSE_PORT_STATUS  0x64
#define MOUSE_PORT_COMMAND 0x64

#define MOUSE_CMD_ENABLE_DATA_REPORTING 0xF4
#define MOUSE_CMD_SET_DEFAULTS 0xF6

static mouse_state_t mouse_state = {0};
static mouse_packet_t current_packet = {0};
static uint8_t mouse_cycle = 0;
static uint8_t mouse_bytes[3];
static int max_x = 79;  // VGA width - 1
static int max_y = 24;  // VGA height - 1
static bool cursor_visible = true;
static int cursor_prev_x = -1;
static int cursor_prev_y = -1;
static char cursor_prev_char = ' ';
static vga_color cursor_prev_fg = VGA_COLOR_WHITE;
static vga_color cursor_prev_bg = VGA_COLOR_BLACK;

// Helper functions
static void mouse_wait_input(void);
static void mouse_wait_output(void);
static void mouse_write_command(uint8_t command);
static void mouse_write_data(uint8_t data);
static uint8_t mouse_read_data(void);

static void mouse_wait_input(void) {
    uint32_t timeout = 100000;
    while (timeout-- && (inb(MOUSE_PORT_STATUS) & 0x02));
}

static void mouse_wait_output(void) {
    uint32_t timeout = 100000;
    while (timeout-- && !(inb(MOUSE_PORT_STATUS) & 0x01));
}

static void mouse_write_command(uint8_t command) {
    mouse_wait_input();
    outb(MOUSE_PORT_COMMAND, command);
}

static void mouse_write_data(uint8_t data) {
    mouse_wait_input();
    outb(MOUSE_PORT_DATA, data);
}

static uint8_t mouse_read_data(void) {
    mouse_wait_output();
    return inb(MOUSE_PORT_DATA);
}

void mouse_init(void) {
    // Initialize mouse state
    mouse_state.x = max_x / 2;
    mouse_state.y = max_y / 2;
    mouse_state.left_button = false;
    mouse_state.right_button = false;
    mouse_state.middle_button = false;
    mouse_state.has_moved = false;
    mouse_state.has_clicked = false;
    
    mouse_cycle = 0;
    
    // Enable auxiliary device (mouse)
    mouse_write_command(0xA8);
    
    // Enable mouse interface
    mouse_write_command(0x20);  // Get status
    uint8_t status = mouse_read_data();
    status |= 0x02;  // Enable IRQ 12
    status &= ~0x20; // Enable mouse clock
    mouse_write_command(0x60);  // Set status
    mouse_write_data(status);
    
    // Send commands to mouse
    mouse_write_command(0xD4);  // Send command to mouse
    mouse_write_data(MOUSE_CMD_SET_DEFAULTS);
    mouse_read_data();  // Acknowledge
    
    mouse_write_command(0xD4);  // Send command to mouse
    mouse_write_data(MOUSE_CMD_ENABLE_DATA_REPORTING);
    mouse_read_data();  // Acknowledge
    
    // Show initial cursor
    mouse_show_cursor();
}

void mouse_handler(void) {
    uint8_t data = inb(MOUSE_PORT_DATA);
    
    switch (mouse_cycle) {
        case 0:
            // First byte: buttons and overflow flags
            if (data & 0x08) {  // Check if this is a valid first byte
                mouse_bytes[0] = data;
                mouse_cycle = 1;
            }
            break;
            
        case 1:
            // Second byte: X movement
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;
            
        case 2:
            // Third byte: Y movement
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            
            // Process complete packet
            current_packet.left_button = (mouse_bytes[0] & 0x01) != 0;
            current_packet.right_button = (mouse_bytes[0] & 0x02) != 0;
            current_packet.middle_button = (mouse_bytes[0] & 0x04) != 0;
            current_packet.x_overflow = (mouse_bytes[0] & 0x40) != 0;
            current_packet.y_overflow = (mouse_bytes[0] & 0x80) != 0;
            
            // Handle X movement (convert from signed)
            if (mouse_bytes[0] & 0x10) {
                current_packet.x_movement = (int8_t)(mouse_bytes[1] | 0xFF00);
            } else {
                current_packet.x_movement = (int8_t)mouse_bytes[1];
            }
            
            // Handle Y movement (convert from signed, Y is inverted)
            if (mouse_bytes[0] & 0x20) {
                current_packet.y_movement = -((int8_t)(mouse_bytes[2] | 0xFF00));
            } else {
                current_packet.y_movement = -((int8_t)mouse_bytes[2]);
            }
            
            // Update mouse state
            bool old_left = mouse_state.left_button;
            bool old_right = mouse_state.right_button;
            
            mouse_state.left_button = current_packet.left_button;
            mouse_state.right_button = current_packet.right_button;
            mouse_state.middle_button = current_packet.middle_button;
            
            // Check for clicks
            mouse_state.has_clicked = (!old_left && current_packet.left_button) ||
                                    (!old_right && current_packet.right_button);
            
            // Update position
            if (!current_packet.x_overflow && !current_packet.y_overflow) {
                int new_x = mouse_state.x + current_packet.x_movement;
                int new_y = mouse_state.y + current_packet.y_movement;
                
                // Clamp to screen bounds
                if (new_x < 0) new_x = 0;
                if (new_x > max_x) new_x = max_x;
                if (new_y < 0) new_y = 0;
                if (new_y > max_y) new_y = max_y;
                
                mouse_state.has_moved = (new_x != mouse_state.x) || (new_y != mouse_state.y);
                mouse_state.x = new_x;
                mouse_state.y = new_y;
                
                // Update cursor display
                if (cursor_visible) {
                    mouse_update_cursor();
                }
            }
            break;
    }
}

bool mouse_has_data(void) {
    return mouse_state.has_moved || mouse_state.has_clicked;
}

mouse_state_t* mouse_get_state(void) {
    // Clear event flags after reading
    mouse_state.has_moved = false;
    mouse_state.has_clicked = false;
    return &mouse_state;
}

void mouse_set_bounds(int max_x_new, int max_y_new) {
    max_x = max_x_new;
    max_y = max_y_new;
    
    // Clamp current position to new bounds
    if (mouse_state.x > max_x) mouse_state.x = max_x;
    if (mouse_state.y > max_y) mouse_state.y = max_y;
}

void mouse_show_cursor(void) {
    cursor_visible = true;
    mouse_update_cursor();
}

void mouse_hide_cursor(void) {
    if (cursor_visible && cursor_prev_x >= 0 && cursor_prev_y >= 0) {
        // Restore previous character
        vga_putc_at(cursor_prev_char, cursor_prev_fg, cursor_prev_bg, 
                   cursor_prev_x, cursor_prev_y);
    }
    cursor_visible = false;
    cursor_prev_x = -1;
    cursor_prev_y = -1;
}

void mouse_update_cursor(void) {
    if (!cursor_visible) return;
    
    // Restore previous position if valid
    if (cursor_prev_x >= 0 && cursor_prev_y >= 0) {
        vga_putc_at(cursor_prev_char, cursor_prev_fg, cursor_prev_bg, 
                   cursor_prev_x, cursor_prev_y);
    }
    
    // Save current character at cursor position
    // For now, we'll assume it's a space - in a real system we'd read from VGA buffer
    cursor_prev_char = ' ';
    cursor_prev_fg = VGA_COLOR_WHITE;
    cursor_prev_bg = VGA_COLOR_BLACK;
    cursor_prev_x = mouse_state.x;
    cursor_prev_y = mouse_state.y;
    
    // Draw cursor
    char cursor_char = '#';  // Simple cursor character
    vga_color cursor_fg = VGA_COLOR_WHITE;
    vga_color cursor_bg = VGA_COLOR_RED;
    
    if (mouse_state.left_button) {
        cursor_fg = VGA_COLOR_YELLOW;  // Highlight when clicking
    }
    
    vga_putc_at(cursor_char, cursor_fg, cursor_bg, mouse_state.x, mouse_state.y);
}