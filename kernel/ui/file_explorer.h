#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

#include <stdint.h>

// Initialize the file explorer
void file_explorer_init(void);

// Open the file explorer UI for a given path
void file_explorer_open(const char* path);

#endif // FILE_EXPLORER_H
