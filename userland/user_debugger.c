#include "user_debugger.h"
#include "../kernel/vga.h"

void user_debugger_init(void) {
    vga_puts("User-mode debugger initialized (placeholder).\n");
}

int user_debugger_attach(uint32_t pid) {
    (void)pid;
    vga_puts("Attaching to user process (placeholder).\n");
    return -1; // Not implemented
}

void user_debugger_detach(void) {
    vga_puts("Detaching from user process (placeholder).\n");
}

int user_debugger_set_breakpoint(uint32_t address) {
    (void)address;
    vga_puts("Setting user breakpoint (placeholder).\n");
    return -1; // Not implemented
}

void user_debugger_continue(void) {
    vga_puts("Continuing user process (placeholder).\n");
}

