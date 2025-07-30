#ifndef UI_BUILDER_H
#define UI_BUILDER_H

#include <stdint.h>

// Initialize UI builder
void ui_builder_init(void);

// Generate UI based on natural language description
int ui_builder_generate_ui(const char* description, void* ui_object_buffer, uint32_t buffer_size);

#endif // UI_BUILDER_H
