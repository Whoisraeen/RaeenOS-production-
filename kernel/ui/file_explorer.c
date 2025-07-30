#include "file_explorer.h"
#include "../../vga.h"
#include "../../string.h"

void file_explorer_init(void) {
    vga_puts("File explorer initialized (placeholder).\n");
}

void file_explorer_open(const char* path) {
    vga_puts("Opening file explorer to path: ");
    vga_puts(path);
    vga_puts(" (placeholder)\n");
}

