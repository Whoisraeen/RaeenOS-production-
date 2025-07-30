#include "text_editor.h"
#include "../../kernel/vga.h"

void text_editor_init(void) {
    vga_puts("Text editor initialized (placeholder).\n");
}

void text_editor_open(const char* filename) {
    vga_puts("Opening file in text editor: ");
    vga_puts(filename);
    vga_puts(" (placeholder)\n");
}

void text_editor_save(void) {
    vga_puts("Saving current file (placeholder).\n");
}

