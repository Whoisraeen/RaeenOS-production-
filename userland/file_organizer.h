#ifndef FILE_ORGANIZER_H
#define FILE_ORGANIZER_H

#include <stdint.h>

// Initialize file organizer
void file_organizer_init(void);

// Analyze and suggest file organization
int file_organizer_suggest(const char* path);

// Automatically organize files
int file_organizer_auto_organize(const char* path);

#endif // FILE_ORGANIZER_H
