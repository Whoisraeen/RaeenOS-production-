#ifndef IME_H
#define IME_H

#include <stdint.h>

// IME context structure (placeholder)
typedef struct {
    // Input buffer, composition string, etc.
    char buffer[64];
    uint32_t cursor_pos;
} ime_context_t;

// Initialize IME framework
void ime_init(void);

// Set active IME
void ime_set_active(uint32_t ime_id);

// Process keyboard input through IME
void ime_process_keypress(uint8_t scancode, char ascii);

// Get IME composition string
const char* ime_get_composition(void);

#endif // IME_H
