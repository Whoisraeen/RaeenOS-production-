/**
 * @file apple_metal.h
 * @brief Apple Silicon Metal GPU Driver for RaeenOS
 * 
 * Advanced driver implementation for Apple M2/M3 GPUs with unified memory
 * architecture and Metal Performance Shaders integration.
 * 
 * Features:
 * - Unified Memory Architecture (UMA) with high-bandwidth access
 * - Metal Performance Shaders (MPS) acceleration
 * - Neural Engine integration for AI/ML workloads
 * - Hardware-accelerated video encode/decode (ProRes, H.264, HEVC)
 * - Tile-based deferred rendering optimization
 * - Advanced power efficiency and thermal design
 * - Custom GPU architecture with programmable shaders
 * - macOS Metal API compatibility layer
 * 
 * Supported SoCs:
 * - Apple M1 (8-core GPU)
 * - Apple M1 Pro (14/16-core GPU)
 * - Apple M1 Max (24/32-core GPU)
 * - Apple M1 Ultra (48/64-core GPU)
 * - Apple M2 (8/10-core GPU)
 * - Apple M2 Pro (16/19-core GPU)
 * - Apple M2 Max (30/38-core GPU)
 * - Apple M2 Ultra (60/76-core GPU)
 * - Apple M3 (8/10-core GPU)
 * - Apple M3 Pro (14/18-core GPU)
 * - Apple M3 Max (30/40-core GPU)
 * 
 * Author: RaeenOS Apple GPU Team
 * License: MIT
 * Version: 1.0.0
 */

#ifndef APPLE_METAL_H
#define APPLE_METAL_H

#include "../gpu.h"
#include "../../kernel/include/types.h"
#include "../../kernel/include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Apple SoC identification
typedef enum {
    APPLE_SOC_M1,           // Apple M1
    APPLE_SOC_M1_PRO,       // Apple M1 Pro
    APPLE_SOC_M1_MAX,       // Apple M1 Max
    APPLE_SOC_M1_ULTRA,     // Apple M1 Ultra
    APPLE_SOC_M2,           // Apple M2
    APPLE_SOC_M2_PRO,       // Apple M2 Pro
    APPLE_SOC_M2_MAX,       // Apple M2 Max
    APPLE_SOC_M2_ULTRA,     // Apple M2 Ultra
    APPLE_SOC_M3,           // Apple M3
    APPLE_SOC_M3_PRO,       // Apple M3 Pro
    APPLE_SOC_M3_MAX        // Apple M3 Max
} apple_soc_t;

// Apple GPU generations
typedef enum {
    APPLE_GPU_GEN_1,        // M1 series
    APPLE_GPU_GEN_2,        // M2 series
    APPLE_GPU_GEN_3         // M3 series
} apple_gpu_generation_t;

// Apple GPU register blocks (estimated based on reverse engineering)
#define APPLE_GPU_BASE                0x20E000000ULL
#define APPLE_GPU_CONTROL_BASE        (APPLE_GPU_BASE + 0x000000)
#define APPLE_GPU_MEMORY_BASE         (APPLE_GPU_BASE + 0x100000)
#define APPLE_GPU_SHADER_BASE         (APPLE_GPU_BASE + 0x200000)
#define APPLE_GPU_VERTEX_BASE         (APPLE_GPU_BASE + 0x300000)
#define APPLE_GPU_FRAGMENT_BASE       (APPLE_GPU_BASE + 0x400000)
#define APPLE_GPU_COMPUTE_BASE        (APPLE_GPU_BASE + 0x500000)

// Control registers
#define APPLE_GPU_CONTROL_STATUS      (APPLE_GPU_CONTROL_BASE + 0x0000)
#define APPLE_GPU_CONTROL_ENABLE      (APPLE_GPU_CONTROL_BASE + 0x0004)
#define APPLE_GPU_CONTROL_RESET       (APPLE_GPU_CONTROL_BASE + 0x0008)
#define APPLE_GPU_CONTROL_IRQ         (APPLE_GPU_CONTROL_BASE + 0x000C)

// Memory management registers
#define APPLE_GPU_MMU_CONTROL         (APPLE_GPU_MEMORY_BASE + 0x0000)
#define APPLE_GPU_MMU_STATUS          (APPLE_GPU_MEMORY_BASE + 0x0004)
#define APPLE_GPU_MMU_FAULT_ADDR      (APPLE_GPU_MEMORY_BASE + 0x0008)
#define APPLE_GPU_MMU_FAULT_STATUS    (APPLE_GPU_MEMORY_BASE + 0x000C)

// Performance counter registers
#define APPLE_GPU_PERF_COUNTER_BASE   (APPLE_GPU_BASE + 0x600000)
#define APPLE_GPU_PERF_CTRL           (APPLE_GPU_PERF_COUNTER_BASE + 0x0000)
#define APPLE_GPU_PERF_VERTEX_CYCLES  (APPLE_GPU_PERF_COUNTER_BASE + 0x0010)
#define APPLE_GPU_PERF_FRAGMENT_CYCLES (APPLE_GPU_PERF_COUNTER_BASE + 0x0014)
#define APPLE_GPU_PERF_COMPUTE_CYCLES (APPLE_GPU_PERF_COUNTER_BASE + 0x0018)

// Metal shader types
typedef enum {
    METAL_SHADER_VERTEX,
    METAL_SHADER_FRAGMENT,
    METAL_SHADER_COMPUTE,
    METAL_SHADER_TILE
} metal_shader_type_t;

// Metal performance shader categories
typedef enum {
    MPS_CATEGORY_CONVOLUTION,       // CNN layers
    MPS_CATEGORY_MATRIX,            // Matrix operations
    MPS_CATEGORY_IMAGE,             // Image processing
    MPS_CATEGORY_NEURAL_NETWORK,    // Neural network layers
    MPS_CATEGORY_RAY_TRACING,       // Ray tracing acceleration
    MPS_CATEGORY_GEOMETRY           // Geometry processing
} mps_category_t;

// Apple GPU core configuration
typedef struct apple_gpu_core {
    uint32_t execution_units;       // ALUs per core
    uint32_t texture_units;         // Texture sampling units
    uint32_t shared_memory_size;    // Threadgroup memory (KB)
    uint32_t register_file_size;    // Register file per thread
    uint32_t max_threads_per_core;  // Maximum threads
} apple_gpu_core_t;

// Apple specific GPU device structure
typedef struct apple_gpu_device {
    gpu_device_t base;              // Base GPU device
    
    // SoC identification
    apple_soc_t soc;
    apple_gpu_generation_t generation;
    uint32_t chip_revision;
    uint32_t gpu_variant;           // Base, Pro, Max, Ultra
    
    // GPU architecture
    struct {
        uint32_t gpu_cores;         // Total GPU cores
        uint32_t execution_units;   // Total execution units
        uint32_t texture_units;     // Texture mapping units
        uint32_t render_backends;   // Render output units
        apple_gpu_core_t core_spec; // Per-core specifications
    } architecture;
    
    // Unified Memory Architecture
    struct {
        uint64_t unified_memory_size; // Total system memory
        uint64_t gpu_accessible_size; // GPU accessible portion
        uint64_t memory_bandwidth;    // Memory bandwidth GB/s
        bool coherent_memory;         // CPU-GPU coherency
        uint32_t page_size;           // Memory page size
        void* shared_memory_pool;     // Shared memory pool
    } uma;
    
    // Clock domains
    struct {
        uint32_t gpu_base_freq;     // Base GPU frequency (MHz)
        uint32_t gpu_max_freq;      // Maximum GPU frequency
        uint32_t memory_freq;       // Memory frequency
        uint32_t neural_engine_freq; // Neural Engine frequency
        uint32_t current_freq;      // Current operating frequency
    } clocks;
    
    // Power management (Apple Silicon efficiency)
    struct {
        uint32_t max_power;         // Maximum power consumption (W)
        uint32_t idle_power;        // Idle power consumption
        uint32_t current_power;     // Current power usage
        bool power_gating;          // Fine-grained power gating
        bool clock_gating;          // Dynamic clock gating
        uint32_t performance_state; // P-state (0-7)
    } power;
    
    // Thermal management
    struct {
        uint32_t max_temp;          // Maximum junction temp (°C)
        uint32_t throttle_temp;     // Thermal throttle point
        uint32_t current_temp;      // Current temperature
        bool thermal_throttling;    // Active throttling
        uint32_t thermal_zones;     // Number of thermal zones
    } thermal;
    
    // Metal Performance Shaders
    struct {
        bool initialized;
        uint32_t supported_categories;
        void* neural_network_graph; // MPS neural network
        void* cnn_convolution;      // CNN operations
        void* matrix_multiplication; // Matrix ops
        void* image_filters;        // Image processing
        bool ray_tracing_support;   // MPS ray tracing
    } mps;
    
    // Neural Engine integration
    struct {
        bool available;
        uint32_t core_count;        // Neural Engine cores
        uint32_t ops_per_second;    // Operations per second
        uint64_t peak_performance;  // Peak TOPS
        bool shared_memory;         // Shared with GPU
        void* ml_compute_device;    // ML compute device
    } neural_engine;
    
    // Video capabilities
    struct {
        bool prores_encode;         // Apple ProRes encoding
        bool prores_decode;         // Apple ProRes decoding
        bool h264_encode_decode;    // H.264 codec
        bool h265_encode_decode;    // H.265/HEVC codec
        bool av1_decode;            // AV1 decode support
        uint32_t max_4k_streams;    // 4K video streams
        uint32_t max_8k_streams;    // 8K video streams
    } video;
    
    // Display capabilities
    struct {
        uint32_t display_controllers;
        bool thunderbolt_display;   // Thunderbolt display support
        bool hdmi_support;          // HDMI output
        bool airplay_support;       // AirPlay mirroring
        uint32_t max_external_displays;
        uint32_t max_resolution_width;
        uint32_t max_resolution_height;
    } display;
    
    // Tile-based rendering
    struct {
        bool enabled;
        uint32_t tile_width;        // Tile width in pixels
        uint32_t tile_height;       // Tile height in pixels
        uint32_t on_chip_memory;    // On-chip tile memory (KB)
        bool deferred_shading;      // Deferred shading support
        bool programmable_blending; // Programmable blending
    } tbdr;
    
    // Register mappings
    volatile uint64_t* mmio_regs;
    size_t mmio_size;
    void* device_tree_node;         // Device tree information
    
    // Command submission
    void* command_queue;            // Metal command queue
    void* command_buffer_pool;      // Command buffer pool
    size_t max_command_buffers;
    
    // Memory management
    void* memory_allocator;         // Custom memory allocator
    void* resource_heap;            // Resource heap manager
    uint64_t allocated_memory;      // Currently allocated
    
    // Interrupt handling
    int irq_line;
    uint32_t irq_sources;
    void* interrupt_handler;
    
    // Performance monitoring
    struct {
        uint64_t vertex_shader_cycles;
        uint64_t fragment_shader_cycles;
        uint64_t compute_shader_cycles;
        uint64_t tile_shader_cycles;
        uint64_t memory_read_bytes;
        uint64_t memory_write_bytes;
        float gpu_utilization;
        float memory_utilization;
        float neural_engine_utilization;
    } perf_counters;
    
    // macOS compatibility
    struct {
        void* iokit_service;        // IOKit service reference
        void* metal_device;         // Metal device reference
        void* accelerator_family;   // GPU family identifier
        bool metal_3_support;       // Metal 3 feature support
        bool ray_tracing_support;   // Hardware ray tracing
    } macos_compat;
    
    // Driver state
    bool initialized;
    bool metal_initialized;
    hal_mutex_t* device_mutex;
    hal_spinlock_t* command_lock;
} apple_gpu_device_t;

// Apple GPU driver operations
extern gpu_vendor_ops_t apple_metal_ops;

// Apple GPU management functions
int apple_metal_probe(device_t* device, const device_id_t* id);
int apple_metal_remove(device_t* device);
int apple_metal_init_device(gpu_device_t* gpu);
void apple_metal_cleanup_device(gpu_device_t* gpu);
int apple_metal_reset_device(gpu_device_t* gpu);

// Hardware detection
apple_soc_t apple_detect_soc(void);
int apple_detect_gpu_config(apple_gpu_device_t* agpu);
int apple_detect_memory_config(apple_gpu_device_t* agpu);
int apple_detect_neural_engine(apple_gpu_device_t* agpu);

// Memory management (Unified Memory Architecture)
int apple_alloc_memory(gpu_device_t* gpu, size_t size, uint32_t flags, gpu_buffer_t** buffer);
void apple_free_memory(gpu_device_t* gpu, gpu_buffer_t* buffer);
int apple_map_memory(gpu_device_t* gpu, gpu_buffer_t* buffer, void** cpu_addr);
void apple_unmap_memory(gpu_device_t* gpu, gpu_buffer_t* buffer);
int apple_alloc_shared_memory(apple_gpu_device_t* agpu, size_t size, void** cpu_ptr, void** gpu_ptr);

// Command submission
int apple_submit_commands(gpu_device_t* gpu, gpu_command_buffer_t* cmd_buf);
int apple_wait_idle(gpu_device_t* gpu);
int apple_create_command_buffer(apple_gpu_device_t* agpu, gpu_command_buffer_t** cmd_buf);

// Metal shader management
int apple_create_shader(gpu_device_t* gpu, const void* bytecode, size_t size, gpu_shader_t** shader);
void apple_destroy_shader(gpu_device_t* gpu, gpu_shader_t* shader);
int apple_compile_metal_shader(const char* metal_source, metal_shader_type_t type, void** bytecode, size_t* size);
int apple_create_compute_pipeline(apple_gpu_device_t* agpu, gpu_shader_t* shader, void** pipeline);

// Texture operations
int apple_create_texture(gpu_device_t* gpu, uint32_t width, uint32_t height, uint32_t format, gpu_texture_t** texture);
void apple_destroy_texture(gpu_device_t* gpu, gpu_texture_t* texture);
int apple_update_texture(gpu_device_t* gpu, gpu_texture_t* texture, const void* data, size_t size);

// Metal Performance Shaders
int apple_mps_init(apple_gpu_device_t* agpu);
int apple_mps_create_cnn_convolution(apple_gpu_device_t* agpu, uint32_t kernel_width, uint32_t kernel_height, void** convolution);
int apple_mps_create_matrix_multiplication(apple_gpu_device_t* agpu, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b, void** matmul);
int apple_mps_create_neural_network(apple_gpu_device_t* agpu, const void* model_data, size_t model_size, void** network);
int apple_mps_run_neural_network(apple_gpu_device_t* agpu, void* network, const void* input, void* output);
int apple_mps_image_convolution(apple_gpu_device_t* agpu, gpu_texture_t* input, gpu_texture_t* output, const float* kernel);
void apple_mps_cleanup(apple_gpu_device_t* agpu);

// Neural Engine integration
int apple_neural_engine_init(apple_gpu_device_t* agpu);
int apple_neural_engine_load_model(apple_gpu_device_t* agpu, const void* model, size_t model_size, void** compiled_model);
int apple_neural_engine_run_inference(apple_gpu_device_t* agpu, void* model, const void* input, void* output);
int apple_neural_engine_get_utilization(apple_gpu_device_t* agpu, float* utilization);
void apple_neural_engine_cleanup(apple_gpu_device_t* agpu);

// Tile-based deferred rendering
int apple_tbdr_enable(apple_gpu_device_t* agpu, bool enable);
int apple_tbdr_configure_tiles(apple_gpu_device_t* agpu, uint32_t tile_width, uint32_t tile_height);
int apple_tbdr_begin_render_pass(apple_gpu_device_t* agpu, gpu_texture_t* render_target);
int apple_tbdr_end_render_pass(apple_gpu_device_t* agpu);

// Video encoding/decoding
int apple_video_init(apple_gpu_device_t* agpu);
int apple_video_encode_prores(apple_gpu_device_t* agpu, const void* frame, void* output, size_t* output_size);
int apple_video_decode_prores(apple_gpu_device_t* agpu, const void* input, void* frame, size_t* frame_size);
int apple_video_encode_h264(apple_gpu_device_t* agpu, const void* frame, void* output, size_t* output_size);
int apple_video_encode_h265(apple_gpu_device_t* agpu, const void* frame, void* output, size_t* output_size);
int apple_video_decode_h264(apple_gpu_device_t* agpu, const void* input, void* frame, size_t* frame_size);
int apple_video_decode_h265(apple_gpu_device_t* agpu, const void* input, void* frame, size_t* frame_size);
void apple_video_cleanup(apple_gpu_device_t* agpu);

// Power management
int apple_set_power_state(gpu_device_t* gpu, uint32_t state);
int apple_get_temperature(gpu_device_t* gpu, uint32_t* temp);
int apple_set_performance_state(apple_gpu_device_t* agpu, uint32_t p_state);
int apple_enable_power_gating(apple_gpu_device_t* agpu, bool enable);
int apple_get_power_consumption(apple_gpu_device_t* agpu, uint32_t* milliwatts);

// Performance monitoring
int apple_get_metrics(gpu_device_t* gpu, gpu_performance_metrics_t* metrics);
int apple_read_performance_counters(apple_gpu_device_t* agpu);
int apple_get_gpu_utilization(apple_gpu_device_t* agpu, float* utilization);
int apple_get_memory_bandwidth_utilization(apple_gpu_device_t* agpu, float* utilization);

// macOS compatibility layer
int apple_macos_compat_init(apple_gpu_device_t* agpu);
int apple_create_metal_device(apple_gpu_device_t* agpu, void** metal_device);
int apple_metal_create_command_queue(apple_gpu_device_t* agpu, void** command_queue);
int apple_metal_create_buffer(apple_gpu_device_t* agpu, size_t size, void** buffer);
int apple_metal_create_texture_from_iosurface(apple_gpu_device_t* agpu, void* iosurface, void** texture);
void apple_macos_compat_cleanup(apple_gpu_device_t* agpu);

// Compute operations
int apple_compute_init(apple_gpu_device_t* agpu);
int apple_compute_dispatch(apple_gpu_device_t* agpu, uint32_t threads_x, uint32_t threads_y, uint32_t threads_z);
int apple_compute_set_buffer(apple_gpu_device_t* agpu, uint32_t index, gpu_buffer_t* buffer);
int apple_compute_set_texture(apple_gpu_device_t* agpu, uint32_t index, gpu_texture_t* texture);
void apple_compute_cleanup(apple_gpu_device_t* agpu);

// Device Tree integration
int apple_parse_device_tree(apple_gpu_device_t* agpu);
int apple_get_device_tree_property(apple_gpu_device_t* agpu, const char* property, void* value, size_t* size);

// Utility functions
const char* apple_soc_to_string(apple_soc_t soc);
const char* apple_gpu_generation_to_string(apple_gpu_generation_t gen);
uint64_t apple_read_reg(apple_gpu_device_t* agpu, uint64_t offset);
void apple_write_reg(apple_gpu_device_t* agpu, uint64_t offset, uint64_t value);
int apple_wait_for_idle(apple_gpu_device_t* agpu, uint32_t timeout_ms);

// Interrupt handling
void apple_irq_handler(device_t* device, int irq, void* data);
int apple_enable_interrupts(apple_gpu_device_t* agpu);
void apple_disable_interrupts(apple_gpu_device_t* agpu);

// Firmware and microcode
int apple_load_gpu_firmware(apple_gpu_device_t* agpu);
int apple_verify_secure_boot(apple_gpu_device_t* agpu);

// Device table for supported Apple Silicon GPUs
extern device_id_t apple_metal_device_table[];
extern size_t apple_metal_device_table_size;

#ifdef __cplusplus
}
#endif

#endif // APPLE_METAL_H