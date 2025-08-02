#ifndef TASKBAR_H
#define TASKBAR_H

#include <stdint.h>

// Initialize the taskbar
void taskbar_init(void);

// Draw the taskbar
void taskbar_draw(void);

// Handle taskbar events (e.g., clicks on buttons)
void taskbar_handle_event(uint32_t x, uint32_t y, uint8_t button);

#endif // TASKBAR_H
