#ifndef _3D_PIPELINE_H
#define _3D_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

// Vertex structure (placeholder)
typedef struct {
    float x, y, z;
    float r, g, b, a; // Color
    float u, v;       // Texture coordinates
} vertex_t;

// Texture structure (placeholder)
typedef struct {
    uint32_t id;
    uint32_t width, height;
    uint8_t* data;
} texture_t;

// Shader structure (placeholder)
typedef struct {
    uint32_t id;
    // Shader program data
} shader_t;

// Initialize the 3D graphics pipeline
void graphics_3d_init(void);

// Create and manage resources
texture_t* graphics_3d_create_texture(uint32_t width, uint32_t height, const uint8_t* data);
void graphics_3d_destroy_texture(texture_t* texture);
shader_t* graphics_3d_create_shader(const char* vertex_src, const char* fragment_src);
void graphics_3d_destroy_shader(shader_t* shader);

// Rendering commands
void graphics_3d_clear(uint32_t color);
void graphics_3d_draw_triangles(const vertex_t* vertices, uint32_t num_vertices, shader_t* shader, texture_t* texture);
void graphics_3d_present(void);

#endif // _3D_PIPELINE_H