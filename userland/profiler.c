#include "profiler.h"
#include "../kernel/vga.h"

void profiler_init(void) {
    vga_puts("Profiler initialized (placeholder).\n");
}

void profiler_start(const char* name) {
    (void)name;
    vga_puts("Profiler started for: ");
    vga_puts(name);
    vga_puts(" (placeholder)\n");
}

void profiler_stop(void) {
    vga_puts("Profiler stopped (placeholder).\n");
}

void profiler_get_report(void) {
    vga_puts("Generating profiler report (placeholder).\n");
}

