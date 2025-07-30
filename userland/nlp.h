#ifndef NLP_H
#define NLP_H

#include <stdint.h>

// Initialize NLP engine
void nlp_init(void);

// Process natural language input
int nlp_process_input(const char* input_text, char* output_buffer, uint32_t buffer_size);

#endif // NLP_H
