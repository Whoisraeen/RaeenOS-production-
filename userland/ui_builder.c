#include "ui_builder.h"
#include "../kernel/vga.h"

void ui_builder_init(void) {
    vga_puts("AI-powered UI builder initialized (placeholder).\n");
}

int ui_builder_generate_ui(const char* description, void* ui_object_buffer, uint32_t buffer_size) {
    (void)description;
    (void)ui_object_buffer;
    (void)buffer_size;
    vga_puts("Generating UI (placeholder).\n");
    return -1; // Not implemented
}

