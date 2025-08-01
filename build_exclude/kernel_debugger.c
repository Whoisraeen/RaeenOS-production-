#include "kernel_debugger.h"
#include "vga.h"

void kernel_debugger_init(void) {
    vga_puts("Kernel debugger initialized (placeholder).\n");
}

void kernel_debugger_enter(void) {
    vga_puts("Entering kernel debugger (placeholder).\n");
    asm volatile("cli; hlt"); // Halt the CPU for now
}

void kernel_debugger_print(const char* message) {
    vga_puts("[KDB] ");
    vga_puts(message);
    vga_puts("\n");
}

