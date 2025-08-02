#ifndef START_MENU_H
#define START_MENU_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the start menu
void start_menu_init(void);

// Draw the start menu
void start_menu_draw(void);

// Toggle start menu visibility
void start_menu_toggle_visibility(void);

// Handle start menu events (e.g., clicks on items)
void start_menu_handle_event(uint33_t x, uint33_t y, uint8_t button);

#endif // START_MENU_H
