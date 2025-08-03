/**
 * RaeenOS Modern Graphics Pipeline
 * Vulkan-compatible graphics API with DirectX translation and GPU acceleration
 */

#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
// #include <pthread.h>  // Commented out to avoid type conflicts in kernel build

// Define types that would normally come from pthread.h and stddef.h
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#ifndef _PTHREAD_MUTEX_T_DEFINED
#define _PTHREAD_MUTEX_T_DEFINED
typedef struct { int __dummy; } pthread_mutex_t;
#endif

// Forward declarations
typedef struct VkInstance_T* VkInstance;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;

// Graphics API types
typedef enum {
    GRAPHICS_API_VULKAN,
    GRAPHICS_API_DIRECTX11,
    GRAPHICS_API_DIRECTX12,
    GRAPHICS_API_OPENGL,
    GRAPHICS_API_SOFTWARE
} GraphicsAPI;

// GPU vendor identification
typedef enum {
    GPU_VENDOR_NVIDIA,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL,
    GPU_VENDOR_UNKNOWN
} GPUVendor;

// GPU memory types
typedef enum {
    GPU_MEMORY_DEVICE_LOCAL,
    GPU_MEMORY_HOST_VISIBLE,
    GPU_MEMORY_HOST_COHERENT,
    GPU_MEMORY_HOST_CACHED
} GPUMemoryType;

// Graphics command types
typedef enum {
    GRAPHICS_CMD_DRAW,
    GRAPHICS_CMD_COMPUTE,
    GRAPHICS_CMD_COPY,
    GRAPHICS_CMD_BARRIER,
    GRAPHICS_CMD_PRESENT
} GraphicsCommandType;

// GPU device information
typedef struct {
    uint32_t device_id;
    uint32_t vendor_id;
    GPUVendor vendor;
    char name[256];
    uint64_t vram_size;
    uint64_t shared_memory_size;
    bool supports_vulkan;
    bool supports_directx11;
    bool supports_directx12;
    bool supports_raytracing;
    bool supports_mesh_shaders;
    bool supports_variable_rate_shading;
    uint32_t max_texture_size;
    uint32_t max_render_targets;
    uint32_t max_compute_workgroup_size[3];
} GPUDeviceInfo;

// GPU memory allocation
typedef struct {
    uint64_t handle;
    void* mapped_ptr;
    uint64_t size;
    uint64_t offset;
    GPUMemoryType type;
    bool is_mapped;
    uint32_t ref_count;
} GPUMemoryAllocation;

// Graphics buffer
typedef struct {
    uint64_t handle;
    GPUMemoryAllocation* memory;
    uint64_t size;
    uint32_t usage_flags;
    bool is_staging;
} GraphicsBuffer;

// Graphics texture
typedef struct {
    uint64_t handle;
    GPUMemoryAllocation* memory;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_layers;
    uint32_t format;
    uint32_t usage_flags;
    uint32_t sample_count;
} GraphicsTexture;

// Shader module
typedef struct {
    uint64_t handle;
    uint32_t* bytecode;
    size_t bytecode_size;
    uint32_t stage_flags;
    char entry_point[64];
} ShaderModule;

// Graphics pipeline state
typedef struct {
    uint64_t handle;
    ShaderModule* vertex_shader;
    ShaderModule* fragment_shader;
    ShaderModule* geometry_shader;
    ShaderModule* compute_shader;
    uint32_t vertex_input_binding_count;
    uint32_t vertex_input_attribute_count;
    uint32_t viewport_count;
    uint32_t scissor_count;
    bool depth_test_enable;
    bool depth_write_enable;
    bool blend_enable;
    uint32_t cull_mode;
    uint32_t front_face;
} GraphicsPipelineState;

// Command buffer
typedef struct {
    uint64_t handle;
    GraphicsCommandType* commands;
    uint32_t command_count;
    uint32_t command_capacity;
    bool is_recording;
    bool is_submitted;
} GraphicsCommandBuffer;

// Render pass
typedef struct {
    uint64_t handle;
    uint32_t color_attachment_count;
    GraphicsTexture** color_attachments;
    GraphicsTexture* depth_attachment;
    uint32_t width;
    uint32_t height;
    uint32_t layers;
} RenderPass;

// Graphics queue
typedef struct {
    uint64_t handle;
    uint32_t family_index;
    uint32_t queue_index;
    bool supports_graphics;
    bool supports_compute;
    bool supports_transfer;
    bool supports_present;
    pthread_mutex_t submit_mutex;
} GraphicsQueue;

// Swapchain for presentation
typedef struct {
    uint64_t handle;
    GraphicsTexture** images;
    uint32_t image_count;
    uint32_t current_image;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    bool vsync_enabled;
    uint32_t present_mode;
} Swapchain;

// Main graphics context
typedef struct {
    // API and device info
    GraphicsAPI current_api;
    GPUDeviceInfo* devices;
    uint32_t device_count;
    uint32_t active_device;
    
    // Vulkan handles
    VkInstance vk_instance;
    VkDevice vk_device;
    VkQueue vk_graphics_queue;
    VkQueue vk_compute_queue;
    VkQueue vk_transfer_queue;
    
    // DirectX handles (when using DX compatibility)
    void* dx11_device;
    void* dx11_context;
    void* dx12_device;
    void* dx12_command_queue;
    
    // Memory management
    GPUMemoryAllocation* memory_allocations;
    uint32_t allocation_count;
    uint32_t allocation_capacity;
    uint64_t total_allocated_memory;
    uint64_t peak_allocated_memory;
    pthread_mutex_t memory_mutex;
    
    // Resource pools
    GraphicsBuffer* buffer_pool;
    GraphicsTexture* texture_pool;
    ShaderModule* shader_pool;
    GraphicsPipelineState* pipeline_pool;
    uint32_t buffer_pool_size;
    uint32_t texture_pool_size;
    uint32_t shader_pool_size;
    uint32_t pipeline_pool_size;
    
    // Command submission
    GraphicsQueue* queues;
    uint32_t queue_count;
    GraphicsCommandBuffer* command_buffers;
    uint32_t command_buffer_count;
    
    // Presentation
    Swapchain* swapchains;
    uint32_t swapchain_count;
    
    // Performance monitoring
    uint64_t frames_rendered;
    uint64_t draw_calls_submitted;
    uint64_t triangles_rendered;
    uint64_t compute_dispatches;
    double average_frame_time;
    double average_gpu_time;
    
    // Configuration
    bool debug_enabled;
    bool validation_enabled;
    bool gpu_timing_enabled;
    uint32_t max_frames_in_flight;
    
    // Synchronization
    pthread_mutex_t context_mutex;
    pthread_cond_t frame_complete_cond;
    bool is_initialized;
} GraphicsContext;

// DirectX compatibility layer
typedef struct {
    void* d3d11_device;
    void* d3d11_context;
    void* d3d12_device;
    void* d3d12_command_queue;
    void* dxgi_factory;
    void* dxgi_adapter;
    bool dx11_available;
    bool dx12_available;
    uint32_t feature_level;
} DirectXCompatLayer;

// Compositor for window management
typedef struct {
    GraphicsContext* graphics;
    GraphicsTexture* framebuffer;
    GraphicsTexture* depth_buffer;
    RenderPass* main_render_pass;
    GraphicsPipelineState* composite_pipeline;
    
    // Window surfaces
    struct WindowSurface** surfaces;
    uint32_t surface_count;
    uint32_t surface_capacity;
    
    // Composition settings
    bool hardware_acceleration;
    bool vsync_enabled;
    uint32_t target_fps;
    double gamma_correction;
    
    // Performance
    uint64_t frames_composited;
    double average_composite_time;
    
    pthread_mutex_t compositor_mutex;
} Compositor;

// Window surface for rendering
typedef struct WindowSurface {
    uint64_t window_id;
    Swapchain* swapchain;
    GraphicsTexture* color_buffer;
    GraphicsTexture* depth_buffer;
    uint32_t width;
    uint32_t height;
    bool is_fullscreen;
    bool needs_resize;
    bool is_visible;
    struct WindowSurface* next;
} WindowSurface;

// Function declarations

// Core graphics system
GraphicsContext* graphics_init(void);
void graphics_shutdown(GraphicsContext* ctx);
bool graphics_enumerate_devices(GraphicsContext* ctx);
bool graphics_select_device(GraphicsContext* ctx, uint32_t device_index);
bool graphics_create_device(GraphicsContext* ctx);

// Memory management
GPUMemoryAllocation* graphics_allocate_memory(GraphicsContext* ctx, uint64_t size, GPUMemoryType type);
void graphics_free_memory(GraphicsContext* ctx, GPUMemoryAllocation* allocation);
void* graphics_map_memory(GraphicsContext* ctx, GPUMemoryAllocation* allocation);
void graphics_unmap_memory(GraphicsContext* ctx, GPUMemoryAllocation* allocation);

// Resource creation
GraphicsBuffer* graphics_create_buffer(GraphicsContext* ctx, uint64_t size, uint32_t usage_flags);
void graphics_destroy_buffer(GraphicsContext* ctx, GraphicsBuffer* buffer);
GraphicsTexture* graphics_create_texture(GraphicsContext* ctx, uint32_t width, uint32_t height, uint32_t format, uint32_t usage_flags);
void graphics_destroy_texture(GraphicsContext* ctx, GraphicsTexture* texture);

// Shader management
ShaderModule* graphics_create_shader(GraphicsContext* ctx, const uint32_t* bytecode, size_t size, uint32_t stage_flags);
void graphics_destroy_shader(GraphicsContext* ctx, ShaderModule* shader);
ShaderModule* graphics_compile_shader_from_source(GraphicsContext* ctx, const char* source, uint32_t stage_flags, const char* entry_point);

// Pipeline management
GraphicsPipelineState* graphics_create_pipeline(GraphicsContext* ctx, ShaderModule* vertex_shader, ShaderModule* fragment_shader);
void graphics_destroy_pipeline(GraphicsContext* ctx, GraphicsPipelineState* pipeline);

// Command recording and submission
GraphicsCommandBuffer* graphics_create_command_buffer(GraphicsContext* ctx);
void graphics_destroy_command_buffer(GraphicsContext* ctx, GraphicsCommandBuffer* cmd_buffer);
void graphics_begin_command_buffer(GraphicsCommandBuffer* cmd_buffer);
void graphics_end_command_buffer(GraphicsCommandBuffer* cmd_buffer);
void graphics_submit_command_buffer(GraphicsContext* ctx, GraphicsCommandBuffer* cmd_buffer, GraphicsQueue* queue);

// Rendering commands
void graphics_cmd_begin_render_pass(GraphicsCommandBuffer* cmd_buffer, RenderPass* render_pass);
void graphics_cmd_end_render_pass(GraphicsCommandBuffer* cmd_buffer);
void graphics_cmd_bind_pipeline(GraphicsCommandBuffer* cmd_buffer, GraphicsPipelineState* pipeline);
void graphics_cmd_bind_vertex_buffer(GraphicsCommandBuffer* cmd_buffer, GraphicsBuffer* buffer);
void graphics_cmd_bind_index_buffer(GraphicsCommandBuffer* cmd_buffer, GraphicsBuffer* buffer);
void graphics_cmd_draw(GraphicsCommandBuffer* cmd_buffer, uint32_t vertex_count, uint32_t instance_count);
void graphics_cmd_draw_indexed(GraphicsCommandBuffer* cmd_buffer, uint32_t index_count, uint32_t instance_count);

// Presentation
Swapchain* graphics_create_swapchain(GraphicsContext* ctx, uint32_t width, uint32_t height, bool vsync);
void graphics_destroy_swapchain(GraphicsContext* ctx, Swapchain* swapchain);
uint32_t graphics_acquire_next_image(GraphicsContext* ctx, Swapchain* swapchain);
void graphics_present_image(GraphicsContext* ctx, Swapchain* swapchain, uint32_t image_index);

// DirectX compatibility
DirectXCompatLayer* directx_init_compatibility(GraphicsContext* ctx);
void directx_shutdown_compatibility(DirectXCompatLayer* dx_compat);
bool directx_translate_d3d11_call(DirectXCompatLayer* dx_compat, void* d3d_call);
bool directx_translate_d3d12_call(DirectXCompatLayer* dx_compat, void* d3d_call);

// Compositor
Compositor* compositor_init(GraphicsContext* graphics);
void compositor_shutdown(Compositor* compositor);
WindowSurface* compositor_create_surface(Compositor* compositor, uint64_t window_id, uint32_t width, uint32_t height);
void compositor_destroy_surface(Compositor* compositor, WindowSurface* surface);
void compositor_resize_surface(Compositor* compositor, WindowSurface* surface, uint32_t width, uint32_t height);
void compositor_composite_frame(Compositor* compositor);

// Advanced Compositor Features
bool compositor_enable_high_refresh_rate(Compositor* compositor, uint32_t target_fps);
bool compositor_enable_hdr(Compositor* compositor, bool hdr10_support);
bool compositor_enable_advanced_effects(Compositor* compositor, bool glassmorphism, bool neumorphism);
bool compositor_configure_multi_monitor(Compositor* compositor, uint32_t monitor_count, uint32_t* widths, uint32_t* heights, float* dpi_scales);
void compositor_enable_gaming_mode(Compositor* compositor, bool enable);
void compositor_update_adaptive_performance(Compositor* compositor);
bool compositor_enable_color_accuracy(Compositor* compositor, bool wide_gamut, bool hardware_calibration);
void compositor_get_performance_stats(Compositor* compositor, double* avg_frame_time, double* current_fps, uint64_t* frames_rendered);

// Performance monitoring
void graphics_update_performance_stats(GraphicsContext* ctx);
void graphics_print_performance_stats(GraphicsContext* ctx);
double graphics_get_average_frame_time(GraphicsContext* ctx);
double graphics_get_average_gpu_time(GraphicsContext* ctx);

// Utility functions
const char* graphics_api_to_string(GraphicsAPI api);
const char* gpu_vendor_to_string(GPUVendor vendor);
bool graphics_is_format_supported(GraphicsContext* ctx, uint32_t format);
uint32_t graphics_get_format_size(uint32_t format);

// Advanced GPU Performance Optimizations for 120FPS+ Gaming
bool graphics_enable_variable_refresh_rate(GraphicsContext* ctx, uint32_t min_fps, uint32_t max_fps);
bool graphics_enable_hdr(GraphicsContext* ctx, bool hdr10_enabled);
void graphics_optimize_input_latency(GraphicsContext* ctx);
bool graphics_enable_raytracing(GraphicsContext* ctx);
bool graphics_enable_mesh_shaders(GraphicsContext* ctx);
void graphics_optimize_memory_management(GraphicsContext* ctx);
void graphics_set_desktop_quality_mode(GraphicsContext* ctx);
void graphics_update_adaptive_quality(GraphicsContext* ctx);
bool graphics_enable_color_accuracy(GraphicsContext* ctx, bool wide_gamut);

// System integration
uint64_t get_total_system_memory(void);

#endif // GRAPHICS_PIPELINE_H
