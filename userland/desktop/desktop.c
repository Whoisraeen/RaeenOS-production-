#include "desktop.h"
#include "window_manager.h"
#include "../../kernel/vga.h"
#include "../../kernel/graphics.h"
#include "../../kernel/memory.h"
#include "../../libs/libc/include/string.h"
#include "../../ui/raeenui.h" // Include RaeenUI

void desktop_init(void) {
    debug_print("Desktop environment initialized (placeholder).\n");
    graphics_init(NULL); // Initialize graphics for desktop
    raeenui_init(); // Initialize RaeenUI
    wm_init(); // Initialize window manager
    graphics_clear_screen(0x000000); // Black background
}

void desktop_start(void) {
    debug_print("Desktop environment started (placeholder main loop).\n");
    // Create a dummy window for demonstration using RaeenUI
    raeenui_window_t* main_window = raeenui_create_window("Hello RaeenOS", 50, 50, 300, 200);
    raeenui_window_set_background_color(main_window, 0x00FF00);
    raeenui_window_show(main_window);

    raeenui_window_t* another_window = raeenui_create_window("Another Window", 150, 150, 400, 250);
    raeenui_window_set_background_color(another_window, 0xFF0000);
    raeenui_window_show(another_window);

    // Main loop for desktop, handling events and drawing
    while (1) {
        raeenui_render_frame(); // Render RaeenUI frame
        // In a real desktop, this would be event-driven, reacting to mouse/keyboard input
        // For now, just redraw continuously
    }
}

void desktop_draw_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, const char* title) {
    // This function is now largely superseded by RaeenUI's drawing capabilities
    // It can be kept for compatibility or removed if all drawing is done via RaeenUI
    debug_print("Desktop: Legacy draw window called for ");
    debug_print(title);
    debug_print("\n");
}

void desktop_handle_mouse_event(uint32_t x, uint32_t y, uint8_t buttons) {
    debug_print("Desktop: Mouse event at (");
    vga_put_dec(x);
    debug_print(", ");
    vga_put_dec(y);
    debug_print(") buttons: ");
    vga_put_hex(buttons);
    debug_print("\n");
    raeenui_handle_mouse_event(x, y, buttons); // Pass to RaeenUI
}

void desktop_handle_keyboard_event(uint8_t scancode, bool pressed) {
    debug_print("Desktop: Keyboard event scancode ");
    vga_put_hex(scancode);
    debug_print(" pressed: ");
    vga_put_dec(pressed);
    debug_print("\n");
    raeenui_handle_keyboard_event(scancode, pressed); // Pass to RaeenUI
}
