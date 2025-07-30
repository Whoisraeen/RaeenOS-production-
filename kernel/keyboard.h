// RaeenOS PS/2 Keyboard Driver

#ifndef KEYBOARD_H
#define KEYBOARD_H

// Initializes the keyboard driver and registers the interrupt handler.
void keyboard_init(void);

// Reads up to `count` characters from the keyboard buffer into `buf`.
// Returns the number of characters read.
int keyboard_read(char* buf, int count);

#endif // KEYBOARD_H
