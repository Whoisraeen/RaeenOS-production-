#ifndef NPU_H
#define NPU_H

#include "include/types.h"

// Initialize NPU driver
void npu_init(void);

// Load AI model to NPU
int npu_load_model(const void* model_data, uint32_t model_size);

// Execute AI inference on NPU
int npu_execute_inference(const void* input_data, uint32_t input_size, void* output_data, uint32_t output_size);

#endif // NPU_H
