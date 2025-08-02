// Simple PS/2 Mouse Driver for RaeenUI prototype - RaeenOS

#ifndef MOUSE_SIMPLE_H
#define MOUSE_SIMPLE_H

#include "include/types.h"

// Mouse packet structure
typedef struct {
    int8_t x_movement;
    int8_t y_movement;
    bool left_button;
    bool right_button;
    bool middle_button;
    bool x_overflow;
    bool y_overflow;
} mouse_packet_t;

// Mouse state
typedef struct {
    int x;
    int y;
    bool left_button;
    bool right_button;
    bool middle_button;
    bool has_moved;
    bool has_clicked;
} mouse_state_t;

// Mouse functions
void mouse_init(void);
void mouse_handler(void);
bool mouse_has_data(void);
mouse_state_t* mouse_get_state(void);
void mouse_set_bounds(int max_x, int max_y);

// Cursor functions
void mouse_show_cursor(void);
void mouse_hide_cursor(void);
void mouse_update_cursor(void);

#endif