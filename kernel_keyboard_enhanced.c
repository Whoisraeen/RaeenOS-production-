/**
 * @file kernel_keyboard_enhanced.c
 * @brief Enhanced PS/2 Keyboard Driver for Interactive RaeenOS
 * 
 * This enhanced keyboard driver provides direct character access
 * for the interactive shell without complex event system dependencies.
 * 
 * @version 1.0
 * @date 2025-08-02
 */

#include "keyboard.h"
#include "idt.h"
#include "ports.h"
#include "pic.h"
#include "include/types.h"

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64
#define KBD_BUFFER_SIZE 256

// Circular buffer for keyboard input
static char key_buffer[KBD_BUFFER_SIZE];
static volatile uint32_t buffer_read_pos = 0;
static volatile uint32_t buffer_write_pos = 0;
static volatile uint32_t buffer_count = 0;

// Scancode to ASCII mapping for US QWERTY
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

// Modifier key states
static bool lshift_pressed = false;
static bool rshift_pressed = false;
static bool capslock_active = false;

/**
 * Add character to the keyboard buffer
 */
static void buffer_add_char(char c) {
    if (buffer_count < KBD_BUFFER_SIZE) {
        key_buffer[buffer_write_pos] = c;
        buffer_write_pos = (buffer_write_pos + 1) % KBD_BUFFER_SIZE;
        buffer_count++;
    }
}

/**
 * Get character from the keyboard buffer
 */
static char buffer_get_char(void) {
    if (buffer_count > 0) {
        char c = key_buffer[buffer_read_pos];
        buffer_read_pos = (buffer_read_pos + 1) % KBD_BUFFER_SIZE;
        buffer_count--;
        return c;
    }
    return 0;
}

/**
 * Keyboard interrupt handler (IRQ1)
 */
void keyboard_handler(void) {
    uint8_t scancode = inb(KBD_DATA_PORT);
    
    if (scancode & 0x80) {
        // Key release
        scancode &= 0x7F;
        if (scancode == 0x2A) {
            lshift_pressed = false;
        } else if (scancode == 0x36) {
            rshift_pressed = false;
        }
    } else {
        // Key press
        if (scancode == 0x2A) {
            lshift_pressed = true;
        } else if (scancode == 0x36) {
            rshift_pressed = true;
        } else if (scancode == 0x3A) {
            capslock_active = !capslock_active;
        } else {
            char c;
            bool use_shift = lshift_pressed || rshift_pressed;
            
            // Handle letters with caps lock
            if (scancode >= 0x10 && scancode <= 0x19) { // QWERTYUIOP
                if (capslock_active) use_shift = !use_shift;
            } else if (scancode >= 0x1E && scancode <= 0x26) { // ASDFGHJKL
                if (capslock_active) use_shift = !use_shift;
            } else if (scancode >= 0x2C && scancode <= 0x32) { // ZXCVBNM
                if (capslock_active) use_shift = !use_shift;
            }
            
            if (use_shift) {
                c = scancode_map_shift[scancode];
            } else {
                c = scancode_map[scancode];
            }
            
            if (c != 0) {
                buffer_add_char(c);
            }
        }
    }
    
    // Send EOI to PIC
    outb(0x20, 0x20);
}

/**
 * Initialize the keyboard driver
 */
void keyboard_init(void) {
    // Clear buffer
    buffer_read_pos = 0;
    buffer_write_pos = 0;
    buffer_count = 0;
    
    // Reset modifier states
    lshift_pressed = false;
    rshift_pressed = false;
    capslock_active = false;
    
    // Note: For now, interrupt handler registration will be done in the main kernel
    // The keyboard_handler function will be called from the main interrupt dispatcher
    
    // Enable keyboard IRQ (unmask IRQ1)
    uint8_t mask = inb(0x21);
    mask &= ~(1 << 1);  // Clear bit 1 for IRQ1
    outb(0x21, mask);
}

/**
 * Check if there's a character available
 */
bool keyboard_has_char(void) {
    return buffer_count > 0;
}

/**
 * Get a character (blocking)
 */
char keyboard_get_char(void) {
    while (!keyboard_has_char()) {
        __asm__ volatile("hlt");  // Wait for interrupt
    }
    return buffer_get_char();
}

/**
 * Try to get a character (non-blocking)
 */
char keyboard_try_get_char(void) {
    if (keyboard_has_char()) {
        return buffer_get_char();
    }
    return 0;
}

/**
 * Read multiple characters from buffer
 */
int keyboard_read(char* buf, int count) {
    int chars_read = 0;
    while (chars_read < count && keyboard_has_char()) {
        buf[chars_read] = buffer_get_char();
        chars_read++;
    }
    return chars_read;
}