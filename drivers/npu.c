#include "npu.h"
#include "../kernel/vga.h"
#include "../kernel/include/driver.h"

// NPU driver structure
static driver_t npu_driver = {
    .name = "NPU Driver",
    .init = npu_init,
    .probe = NULL // NPU is not a bus driver
};

void npu_init(void) {
    vga_puts("NPU driver initialized (placeholder).\n");
}

int npu_load_model(const void* model_data, uint32_t model_size) {
    (void)model_data;
    (void)model_size;
    vga_puts("Loading AI model to NPU (placeholder).\n");
    return -1; // Not implemented
}

int npu_execute_inference(const void* input_data, uint32_t input_size, void* output_data, uint32_t output_size) {
    (void)input_data;
    (void)input_size;
    (void)output_data;
    (void)output_size;
    vga_puts("Executing AI inference on NPU (placeholder).\n");
    return -1; // Not implemented
}

