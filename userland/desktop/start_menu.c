#include "start_menu.h"
#include "desktop.h"
#include "../../kernel/graphics.h"
#include "../../kernel/vga.h"
#include "../../libs/libc/include/string.h"
#include "../../libs/libc/include/string.h"

static bool is_visible = false;

void start_menu_init(void) {
    debug_print("Start Menu initialized (placeholder).\n");
}

void start_menu_draw(void) {
    if (!is_visible) return;

    // Draw a simple grey rectangle for the start menu
    uint32_t menu_width = 200;
    uint32_t menu_height = 300;
    uint32_t screen_height = graphics_get_height();
    uint32_t taskbar_height = 30; // Assuming taskbar is 30px high

    graphics_draw_rect(0, screen_height - taskbar_height - menu_height, menu_width, menu_height, 0x444444); // Darker grey
    graphics_draw_string(10, screen_height - taskbar_height - menu_height + 10, "Start Menu", 0xFFFFFF);
    graphics_draw_string(10, screen_height - taskbar_height - menu_height + 30, "- Apps", 0xFFFFFF);
    graphics_draw_string(10, screen_height - taskbar_height - menu_height + 50, "- Settings", 0xFFFFFF);
    debug_print("Start Menu: Drawn.\n");
}

void start_menu_toggle_visibility(void) {
    is_visible = !is_visible;
    debug_print("Start Menu: Visibility toggled to ");
    vga_put_dec(is_visible);
    debug_print("\n");
}

void start_menu_handle_event(uint32_t x, uint32_t y, uint8_t button) {
    if (!is_visible) return;

    debug_print("Start Menu: Event at (");
    vga_put_dec(x);
    debug_print(", ");
    vga_put_dec(y);
    debug_print(") button: ");
    vga_put_dec(button);
    debug_print(" (simulated).\n");
    // In a real start menu, this would handle clicks on menu items.
}
