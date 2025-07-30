#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include <stdint.h>

// Initialize code generation engine
void code_generation_init(void);

// Generate code based on natural language description
int code_generation_generate_code(const char* description, char* output_buffer, uint32_t buffer_size);

#endif // CODE_GENERATION_H
