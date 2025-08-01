#include "ime.h"
#include "../vga.h"

static ime_context_t current_ime_context;

void ime_init(void) {
    vga_puts("IME framework initialized (placeholder).\n");
    current_ime_context.buffer[0] = '\0';
    current_ime_context.cursor_pos = 0;
}

void ime_set_active(uint32_t ime_id) {
    vga_puts("Setting active IME (placeholder): ");
    vga_put_dec(ime_id);
    vga_puts("\n");
}

void ime_process_keypress(uint8_t scancode, char ascii) {
    // For now, just append to buffer if it's a printable ASCII character
    if (ascii >= ' ' && ascii <= '~' && current_ime_context.cursor_pos < sizeof(current_ime_context.buffer) - 1) {
        current_ime_context.buffer[current_ime_context.cursor_pos++] = ascii;
        current_ime_context.buffer[current_ime_context.cursor_pos] = '\0';
    } else if (scancode == 0x0E) { // Backspace
        if (current_ime_context.cursor_pos > 0) {
            current_ime_context.cursor_pos--;
            current_ime_context.buffer[current_ime_context.cursor_pos] = '\0';
        }
    }
}

const char* ime_get_composition(void) {
    return current_ime_context.buffer;
}
