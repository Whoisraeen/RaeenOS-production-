#include "python_runtime.h"
#include "../kernel/vga.h"

void python_runtime_init(void) {
    vga_puts("Python runtime initialized (placeholder).\n");
}

int python_runtime_exec_script(const char* script_path) {
    vga_puts("Executing Python script: ");
    vga_puts(script_path);
    vga_puts(" (placeholder)\n");
    return -1; // Not implemented
}

