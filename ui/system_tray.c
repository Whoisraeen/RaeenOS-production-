#include "system_tray.h"
#include "../graphics.h"
#include "../string.h"

void system_tray_init(void) {
    // Nothing specific to initialize for now
}

void system_tray_draw(void) {
    uint32_t screen_width = graphics_get_width();
    uint32_t screen_height = graphics_get_height();

    // Draw a simple rectangle for the system tray
    graphics_draw_rect(screen_width - 150, screen_height - 30, 150, 30, 0x00222222);
    graphics_draw_string(screen_width - 140, screen_height - 20, "[Time] [Icons]", 0xFFFFFFFF);
}
