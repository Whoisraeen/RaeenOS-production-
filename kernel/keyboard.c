// RaeenOS PS/2 Keyboard Driver

#include "keyboard.h"
#include "idt.h"
#include "ports.h"
#include "pic.h"
#include "include/event.h"
#include "include/types.h"
// Using types.h for kernel build

#define KBD_DATA_PORT   0x60
#define KBD_BUFFER_SIZE 256

// A circular buffer for keyboard input
static char key_buffer[KBD_BUFFER_SIZE];
static uint32_t buffer_read_pos = 0;
static uint32_t buffer_write_pos = 0;

// Basic US QWERTY scancode map. 0 indicates an unhandled key.
static const uint8_t scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', /* 0x00 - 0x0E */
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* 0x0F - 0x1C */
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  /* 0x1D - 0x29 */
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,      /* 0x2A - 0x36 */
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x37 - 0x44 */
    0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, /* 0x45 - 0x54 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x55 - 0x80 */
};

static const uint8_t scancode_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', /* 0x00 - 0x0E */
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* 0x0F - 0x1C */
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',  /* 0x1D - 0x29 */
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,      /* 0x2A - 0x36 */
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x37 - 0x44 */
    0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, /* 0x45 - 0x54 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x55 - 0x80 */
};

static bool lshift_pressed = false;
static bool rshift_pressed = false;
static bool capslock_active = false;

/**
 * @brief The main keyboard interrupt handler.
 * 
 * This function is called every time a keyboard interrupt (IRQ1) is fired.
 * It reads the scancode from the keyboard data port and, if it's a valid
 * key press, adds the corresponding ASCII character to the circular buffer.
 */
void keyboard_handler(void) {
    uint8_t scancode = inb(KBD_DATA_PORT);

    if (scancode & 0x80) { // Key release
        scancode &= 0x7F; // Clear the release bit
        if (scancode == 0x2A) { // Left Shift
            lshift_pressed = false;
        } else if (scancode == 0x36) { // Right Shift
            rshift_pressed = false;
        }
    } else { // Key press
        if (scancode == 0x2A) { // Left Shift
            lshift_pressed = true;
        } else if (scancode == 0x36) { // Right Shift
            rshift_pressed = true;
        } else if (scancode == 0x3A) { // Caps Lock
            capslock_active = !capslock_active;
        } else {
            char c;
            if (lshift_pressed || rshift_pressed || capslock_active) {
                c = scancode_map_shift[scancode];
            } else {
                c = scancode_map[scancode];
            }

            if (c != 0) {
                event_t event;
                event.type = EVENT_KEY_PRESS;
                event.data.keyboard.scancode = scancode;
                event.data.keyboard.ascii = c;
                event.data.keyboard.pressed = true;
                event_queue_push(event);
            }
        }
    }
}

/**
 * @brief Initializes the keyboard driver.
 * 
 * Registers the keyboard interrupt handler for IRQ1.
 */
void keyboard_init(void) {
    register_interrupt_handler(IRQ_TO_INT(1), keyboard_handler);
}

/**
 * @brief Reads characters from the keyboard buffer.
 * 
 * This is a non-blocking call that reads up to `count` characters into `buf`.
 * It's intended to be called from a syscall like sys_read.
 * 
 * @param buf The buffer to store the characters in.
 * @param count The maximum number of characters to read.
 * @return int The number of characters actually read.
 */
int keyboard_read(char* buf, int count) {
    int chars_read = 0;
    while (chars_read < count && buffer_read_pos != buffer_write_pos) {
        buf[chars_read] = key_buffer[buffer_read_pos];
        buffer_read_pos = (buffer_read_pos + 1) % KBD_BUFFER_SIZE;
        chars_read++;
    }
    return chars_read;
}