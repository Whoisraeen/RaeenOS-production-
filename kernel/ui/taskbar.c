#include "taskbar.h"
#include "../graphics.h"

void taskbar_init(void) {
    // Nothing specific to initialize for now
}

void taskbar_draw(void) {
    uint32_t screen_width = graphics_get_width();
    uint32_t screen_height = graphics_get_height();

    // Draw a simple black rectangle at the bottom of the screen
    graphics_draw_rect(0, screen_height - 30, screen_width, 30, 0x00000000);
}
