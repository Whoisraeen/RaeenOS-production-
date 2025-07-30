#include "accessibility.h"
#include "../../vga.h"

static bool screen_reader_enabled = false;

void accessibility_init(void) {
    vga_puts("Accessibility features initialized (placeholder).\n");
}

void accessibility_set_screen_reader_enabled(bool enabled) {
    screen_reader_enabled = enabled;
    if (enabled) {
        vga_puts("Screen reader enabled.\n");
    } else {
        vga_puts("Screen reader disabled.\n");
    }
}

void accessibility_speak(const char* text) {
    if (screen_reader_enabled) {
        vga_puts("[SPEECH] ");
        vga_puts(text);
        vga_puts("\n");
    }
}

