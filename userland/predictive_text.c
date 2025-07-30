#include "predictive_text.h"
#include "../kernel/vga.h"

void predictive_text_init(void) {
    vga_puts("Predictive text engine initialized (placeholder).\n");
}

const char* predictive_text_get_prediction(const char* current_input) {
    (void)current_input;
    vga_puts("Getting predictive text (placeholder).\n");
    return ""; // Return empty string for now
}

