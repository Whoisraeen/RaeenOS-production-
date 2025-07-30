#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <stdint.h>

// Initialize the text editor
void text_editor_init(void);

// Open a file in the text editor
void text_editor_open(const char* filename);

// Save the current file
void text_editor_save(void);

#endif // TEXT_EDITOR_H
