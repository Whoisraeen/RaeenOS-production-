#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the AI engine
void ai_engine_init(void);

// Load an AI model
int ai_engine_load_model(const char* model_path);

// Perform inference with an AI model
int ai_engine_perform_inference(const uint8_t* input_data, uint32_t input_size, uint8_t* output_data, uint32_t output_size);

// Process natural language input
char* ai_engine_process_nlp(const char* text);

#endif // AI_ENGINE_H
