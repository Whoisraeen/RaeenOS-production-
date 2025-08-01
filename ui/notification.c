#include "notification.h"
#include "../graphics.h"
#include "../string.h"

void notification_init(void) {
    // Nothing specific to initialize for now
}

void notification_show(notification_t* notification) {
    // For now, just print to VGA. A real notification would draw on screen.
    graphics_draw_string(graphics_get_width() - 200, 20, notification->title, 0xFFFFFFFF);
    graphics_draw_string(graphics_get_width() - 200, 40, notification->message, 0xFFFFFFFF);
}
