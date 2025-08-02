// RaeenOS Terminal Widget Interface
// ---------------------------------

#ifndef TERMINAL_H
#define TERMINAL_H

#include "widget.h"
#include "../kernel/ipc/pipe.h"

// Function to create a new terminal widget
widget_t* terminal_create(struct window_t* parent, int x, int y, int width, int height);

// Function to handle keyboard input for the terminal
void terminal_handle_keypress(widget_t* terminal, char c);

// Function to write a string to the terminal
void terminal_write_string(widget_t* terminal, const char* str);

// Function to process any pending input in the terminal's input pipe
void terminal_process_input(widget_t* terminal);

// Function to draw the terminal widget
void draw_terminal(widget_t* widget, struct window_t* parent);

// Functions to get the terminal's communication pipes
pipe_t* terminal_get_input_pipe(widget_t* terminal);
pipe_t* terminal_get_output_pipe(widget_t* terminal);

#endif // TERMINAL_H
