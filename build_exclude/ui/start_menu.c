#include "start_menu.h"
#include "../graphics.h"
#include "../string.h"

static bool is_visible = false;

void start_menu_init(void) {
    // Nothing specific to initialize for now
}

void start_menu_draw(void) {
    if (!is_visible) return;

    uint32_t screen_width = graphics_get_width();
    uint32_t screen_height = graphics_get_height();

    // Draw a simple rectangle for the start menu
    graphics_draw_rect(0, screen_height - 30 - 200, 200, 200, 0x00555555);
    graphics_draw_string(10, screen_height - 30 - 180, "RaeenOS", 0xFFFFFFFF);
    graphics_draw_string(10, screen_height - 30 - 160, "----------", 0xFFFFFFFF);
    graphics_draw_string(10, screen_height - 30 - 140, "Programs", 0xFFFFFFFF);
    graphics_draw_string(10, screen_height - 30 - 120, "Settings", 0xFFFFFFFF);
    graphics_draw_string(10, screen_height - 30 - 100, "Shutdown", 0xFFFFFFFF);
}

void start_menu_toggle(void) {
    is_visible = !is_visible;
}
