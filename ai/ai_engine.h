/**
 * @file ai_engine.h
 * @brief RaeenOS AI Integration Engine Header
 * 
 * Neural processing unit integration with model inference
 */

#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "../kernel/include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// AI engine structures
typedef struct ai_model {
    char name[64];
    void* weights;
    size_t weights_size;
    uint32_t input_size;
    uint32_t output_size;
    bool loaded;
} ai_model_t;

typedef struct ai_context {
    ai_model_t* models;
    uint32_t model_count;
    void* inference_buffer;
    size_t buffer_size;
} ai_context_t;

// Function declarations
int ai_engine_init(void);
void ai_engine_cleanup(void);
int ai_load_model(const char* model_path, ai_model_t* model);
int ai_run_inference(ai_model_t* model, void* input, void* output);
void ai_unload_model(ai_model_t* model);

#ifdef __cplusplus
}
#endif

#endif // AI_ENGINE_H