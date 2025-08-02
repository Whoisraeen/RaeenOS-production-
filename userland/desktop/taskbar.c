#include "taskbar.h"
#include "desktop.h"
#include "../../kernel/graphics.h"
#include "../../kernel/vga.h"
#include "../../libs/libc/include/string.h"
#include "../../libs/libc/include/string.h"

void taskbar_init(void) {
    debug_print("Taskbar initialized (placeholder).\n");
}

void taskbar_draw(void) {
    // Draw a simple black rectangle at the bottom of the screen
    uint32_t screen_width = graphics_get_width();
    uint32_t screen_height = graphics_get_height();
    uint32_t taskbar_height = 30; // Example height
    uint32_t taskbar_y = screen_height - taskbar_height;

    graphics_draw_rect(0, taskbar_y, screen_width, taskbar_height, 0x222222); // Dark grey taskbar
    graphics_draw_string(5, taskbar_y + 8, "RaeenOS Taskbar", 0xFFFFFF); // White text
    debug_print("Taskbar: Drawn.\n");
}

void taskbar_handle_event(uint32_t x, uint32_t y, uint8_t button) {
    debug_print("Taskbar: Event at (");
    vga_put_dec(x);
    debug_print(", ");
    vga_put_dec(y);
    debug_print(") button: ");
    vga_put_dec(button);
    debug_print(" (simulated).\n");
    // In a real taskbar, this would handle clicks on start button, open windows, etc.
}
