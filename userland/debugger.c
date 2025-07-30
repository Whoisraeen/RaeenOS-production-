#include "debugger.h"
#include "../kernel/vga.h"

void debugger_init(void) {
    vga_puts("Debugger initialized (placeholder).\n");
}

int debugger_attach(uint32_t pid) {
    (void)pid;
    vga_puts("Attaching to process (placeholder).\n");
    return -1; // Not implemented
}

void debugger_detach(void) {
    vga_puts("Detaching from process (placeholder).\n");
}

int debugger_set_breakpoint(uint32_t address) {
    (void)address;
    vga_puts("Setting breakpoint (placeholder).\n");
    return -1; // Not implemented
}

void debugger_continue(void) {
    vga_puts("Continuing execution (placeholder).\n");
}

