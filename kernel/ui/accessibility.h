#ifndef ACCESSIBILITY_H
#define ACCESSIBILITY_H

#include <stdint.h>
#include <stdbool.h>

// Initialize accessibility features
void accessibility_init(void);

// Enable/disable screen reader
void accessibility_set_screen_reader_enabled(bool enabled);

// Speak text (placeholder for screen reader output)
void accessibility_speak(const char* text);

#endif // ACCESSIBILITY_H
