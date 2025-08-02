#ifndef GPU_H
#define GPU_H

#include "include/types.h"

// NOTE: This is a generic, software-rendered GPU driver. 
// Hardware-specific implementations (e.g., Vulkan/DirectX) are currently missing.

// Generic GPU capabilities
typedef struct {
    uint32_t max_width;
    uint32_t max_height;
    uint32_t max_depth;
    bool has_3d_acceleration;
    bool has_hardware_overlay;
    // Add more capabilities as needed
} gpu_capabilities_t;

// Initialize generic GPU driver
void gpu_init(void);

// Get GPU capabilities
void gpu_get_capabilities(gpu_capabilities_t* caps);

// Set video mode (placeholder)
int gpu_set_mode(uint32_t width, uint32_t height, uint32_t depth);

// Basic 3D rendering (placeholder - software rasterized)
void gpu_draw_triangle(float x1, float y1, float z1, uint32_t c1,
                       float x2, float y2, float z2, uint32_t c2,
                       float x3, float y3, float z3, uint32_t c3);

// Placeholder for submitting commands to a hardware GPU
int gpu_submit_command_buffer(void* command_buffer, uint32_t size);

// Placeholder for allocating VRAM
void* gpu_alloc_vram(uint32_t size);

#endif // GPU_H
