// RaeenOS PS/2 Keyboard Driver

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "include/types.h"

// Initializes the keyboard driver and registers the interrupt handler.
void keyboard_init(void);

// Reads up to `count` characters from the keyboard buffer into `buf`.
// Returns the number of characters read.
int keyboard_read(char* buf, int count);

// Check if there's a character available in the buffer
bool keyboard_has_char(void);

// Get a single character from the buffer (blocking if none available)
char keyboard_get_char(void);

// Non-blocking version - returns 0 if no character available
char keyboard_try_get_char(void);

// Keyboard interrupt handler (called from main interrupt dispatcher)
void keyboard_handler(void);

#endif // KEYBOARD_H
