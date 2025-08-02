#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the file explorer
void file_explorer_init(void);

// Open a file explorer window for a given path
void file_explorer_open(const char* path);

// Handle file explorer events (e.g., clicks on files/folders)
void file_explorer_handle_event(uint33_t x, uint33_t y, uint8_t button);

#endif // FILE_EXPLORER_H
