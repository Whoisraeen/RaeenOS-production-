/**
 * @file intel_arc.h
 * @brief Intel Arc GPU Driver for RaeenOS
 * 
 * Advanced driver implementation for Intel Arc A770/A750 and Xe-HPG GPUs
 * with hardware ray tracing, XeSS AI upscaling, and AV1 encoding support.
 * 
 * Features:
 * - Xe-HPG architecture with Xe-cores and RT units
 * - Hardware ray tracing acceleration
 * - XeSS AI-powered super resolution
 * - Dual AV1 encoders for content creation
 * - Variable Rate Shading Tier 2
 * - Mesh shaders and geometry pipeline
 * - Advanced power efficiency and thermal design
 * - DirectX 12 Ultimate and Vulkan 1.3 support
 * - Intel Deep Link technology
 * 
 * Supported GPUs:
 * - Arc A770 16GB (ACM-G10)
 * - Arc A770 8GB (ACM-G10)
 * - Arc A750 (ACM-G10)
 * - Arc A580 (ACM-G12)
 * - Arc A380 (ACM-G11)
 * - Arc A310 (ACM-G11)
 * 
 * Author: RaeenOS Intel GPU Team
 * License: MIT
 * Version: 1.0.0
 */

#ifndef INTEL_ARC_H
#define INTEL_ARC_H

#include "../gpu.h"
#include "../../kernel/include/types.h"
#include "../../kernel/include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Intel GPU device IDs (Arc/Xe-HPG)
#define INTEL_ARC_A770_16GB_DEVICE_ID  0x56A0
#define INTEL_ARC_A770_8GB_DEVICE_ID   0x56A1
#define INTEL_ARC_A750_DEVICE_ID       0x56A2
#define INTEL_ARC_A580_DEVICE_ID       0x5690
#define INTEL_ARC_A380_DEVICE_ID       0x5691
#define INTEL_ARC_A310_DEVICE_ID       0x5692

// Intel GPU generations
typedef enum {
    INTEL_GPU_GEN12,        // Xe-LP (Tiger Lake, Rocket Lake)
    INTEL_GPU_GEN12_5,      // Xe-HPG (Arc Alchemist)
    INTEL_GPU_GEN13,        // Xe-HPP (Ponte Vecchio)
    INTEL_GPU_GEN14         // Xe-HPC (Aurora)
} intel_gpu_generation_t;

// Intel GPU SKUs
typedef enum {
    INTEL_SKU_ACM_G10,      // Arc A770/A750 (High-end)
    INTEL_SKU_ACM_G11,      // Arc A380/A310 (Entry-level)
    INTEL_SKU_ACM_G12       // Arc A580 (Mid-range)
} intel_gpu_sku_t;

// Intel GPU register blocks
#define INTEL_MMIO_BASE               0x00000000
#define INTEL_RENDER_RING_BASE        0x00002000
#define INTEL_BLITTER_RING_BASE       0x00022000
#define INTEL_VIDEO_RING_BASE         0x0001C000
#define INTEL_VEBOX_RING_BASE         0x0001A000
#define INTEL_COMPUTE_RING_BASE       0x0001E000

// Graphics Technology (GT) registers
#define INTEL_GT_THREAD_STATUS        0x00005008
#define INTEL_GT_CORE_STATUS          0x0000500C
#define INTEL_GT_FIFO_FREE_ENTRIES    0x00005010
#define INTEL_GT_MODE                 0x0000700C
#define INTEL_GT_CHICKEN_BIT          0x00007300

// Render engine registers
#define INTEL_RENDER_HWSP_PGA         0x00002080
#define INTEL_RENDER_RING_HEAD        0x00002034
#define INTEL_RENDER_RING_TAIL        0x00002030
#define INTEL_RENDER_RING_START       0x00002038
#define INTEL_RENDER_RING_CTL         0x0000203C

// Memory interface registers  
#define INTEL_MCHBAR_MIRROR           0x00140000
#define INTEL_GEN6_GDRST              0x00941408
#define INTEL_GEN6_PCODE_MAILBOX      0x00138124
#define INTEL_GEN6_PCODE_DATA         0x00138128

// Display engine registers
#define INTEL_DE_PIPE_A_CONF          0x00070008
#define INTEL_DE_PIPE_B_CONF          0x00071008  
#define INTEL_DE_PIPE_C_CONF          0x00072008
#define INTEL_DE_PORT_HOTPLUG         0x000C4030

// Power management registers
#define INTEL_RPNSWREQ                0x0000A008
#define INTEL_RPNSWREQ_FREQUENCY      0x0000A00C
#define INTEL_RPSTAT1                 0x0000A01C
#define INTEL_RPINCLIMIT              0x0000A020
#define INTEL_RPDECLIMIT              0x0000A024

// XeSS quality modes
typedef enum {
    XESS_QUALITY_PERFORMANCE = 0,   // 2x upscale
    XESS_QUALITY_BALANCED   = 1,    // 1.7x upscale
    XESS_QUALITY_QUALITY    = 2,    // 1.5x upscale
    XESS_QUALITY_ULTRA      = 3     // 1.3x upscale
} intel_xess_quality_t;

// XeSS features
typedef enum {
    XESS_FEATURE_UPSCALING     = (1 << 0),
    XESS_FEATURE_ANTI_ALIASING = (1 << 1),
    XESS_FEATURE_MOTION_VECTORS = (1 << 2),
    XESS_FEATURE_DYNAMIC_RES   = (1 << 3)
} intel_xess_features_t;

// Xe-core configuration
typedef struct intel_xe_core {
    uint32_t vector_engines;        // Vector processing engines
    uint32_t matrix_engines;        // XMX matrix engines
    uint32_t sampler_units;         // Texture sampler units
    uint32_t pixel_backends;        // Pixel backend units
    uint32_t geometry_pipelines;    // Geometry processing pipelines
    uint32_t l1_cache_size;         // L1 cache per Xe-core
    uint32_t shared_local_memory;   // Shared local memory
} intel_xe_core_t;

// Intel specific GPU device structure
typedef struct intel_gpu_device {
    gpu_device_t base;              // Base GPU device
    
    // GPU identification
    intel_gpu_generation_t generation;
    intel_gpu_sku_t sku;
    uint32_t gt_level;              // GT1, GT2, GT3, etc.
    uint32_t stepping;              // Silicon stepping
    
    // Xe-HPG architecture details
    struct {
        uint32_t xe_cores;          // Total Xe-cores
        uint32_t rt_units;          // Ray tracing units
        uint32_t xe_media_engines;  // Media processing engines
        uint32_t xe_copy_engines;   // Copy/DMA engines
        uint32_t l3_cache_size;     // Shared L3 cache (KB)
        intel_xe_core_t xe_core_spec; // Xe-core specifications
    } xe_hpg;
    
    // Memory subsystem
    struct {
        uint64_t local_memory_size; // Local GDDR6 memory
        uint32_t memory_bus_width;  // Memory interface width
        uint32_t memory_channels;   // Memory channels
        uint64_t memory_bandwidth;  // Peak bandwidth GB/s
        bool resizable_bar;         // Resizable BAR support
        uint32_t system_memory_access; // System memory bandwidth
    } memory;
    
    // Clock domains
    struct {
        uint32_t base_freq;         // Base frequency (MHz)
        uint32_t max_freq;          // Maximum frequency (MHz)
        uint32_t efficient_freq;    // Most efficient frequency
        uint32_t memory_freq;       // Memory frequency (MHz)
        uint32_t media_freq;        // Media engine frequency
        uint32_t current_freq;      // Current operating frequency
    } clocks;
    
    // Power management
    struct {
        uint32_t tdp;               // Thermal Design Power (W)
        uint32_t max_turbo_power;   // Maximum turbo power
        uint32_t current_power;     // Current power consumption
        uint32_t power_limit_1;     // PL1 sustained power
        uint32_t power_limit_2;     // PL2 burst power
        bool adaptive_sync;         // Adaptive sync enabled
        uint32_t voltage_offset;    // Voltage offset (mV)
    } power;
    
    // Thermal management
    struct {
        uint32_t max_temp;          // Maximum junction temp (°C)
        uint32_t throttle_temp;     // Throttle temperature
        uint32_t current_temp;      // Current temperature
        uint32_t fan_speed;         // Fan speed (%)
        bool thermal_throttling;    // Throttling active
    } thermal;
    
    // XeSS state
    struct {
        bool initialized;
        intel_xess_quality_t quality_mode;
        uint32_t supported_features;
        bool motion_vectors_enabled;
        bool dynamic_resolution;
        float sharpening_factor;
    } xess;
    
    // Ray tracing state
    struct {
        bool enabled;
        uint32_t rt_unit_utilization;
        uint64_t rays_dispatched;
        uint64_t bvh_intersections;
        uint32_t rt_shader_count;
    } ray_tracing;
    
    // Media capabilities
    struct {
        bool dual_av1_encoders;     // Dual AV1 encode engines
        bool av1_decode;            // AV1 decode support
        bool h264_encode_decode;    // H.264 codec
        bool h265_encode_decode;    // H.265/HEVC codec
        bool vp9_decode;            // VP9 decode
        uint32_t max_encode_streams;
        uint32_t max_decode_streams;
    } media;
    
    // Display capabilities
    struct {
        uint32_t display_pipes;     // Display pipes available
        bool hdmi_21_support;       // HDMI 2.1 support
        bool dp_20_support;         // DisplayPort 2.0
        bool dsc_support;           // Display Stream Compression
        bool hdr_support;           // HDR support
        uint32_t max_outputs;       // Maximum display outputs
        uint32_t max_resolution;    // Maximum resolution per output
    } display;
    
    // Variable Rate Shading
    struct {
        bool tier1_support;         // VRS Tier 1
        bool tier2_support;         // VRS Tier 2
        uint32_t shading_rates;     // Supported shading rates
        bool per_draw_vrs;          // Per-draw VRS
        bool per_primitive_vrs;     // Per-primitive VRS
    } vrs;
    
    // Register mappings
    volatile uint32_t* mmio_regs;
    volatile uint32_t* gtt_regs;    // Graphics Translation Table
    size_t mmio_size;
    size_t gtt_size;
    
    // Command submission
    void* render_ring;              // Render command ring
    void* blitter_ring;             // Blitter command ring
    void* video_ring;               // Video command ring
    void* compute_ring;             // Compute command ring
    size_t ring_size;
    
    // GuC (Graphics Micro Controller)
    struct {
        bool enabled;
        void* firmware;
        size_t firmware_size;
        uint32_t version;
        bool submission_enabled;    // GuC command submission
        bool power_management;      // GuC power management
    } guc;
    
    // HuC (HEVC/H.265 Micro Controller)
    struct {
        bool enabled;
        void* firmware;
        size_t firmware_size;
        uint32_t version;
        bool authenticated;
    } huc;
    
    // Interrupt handling
    int irq_line;
    uint32_t irq_mask;
    uint32_t gt_irq_mask;
    
    // Performance monitoring
    struct {
        uint64_t render_engine_busy;
        uint64_t blitter_engine_busy;
        uint64_t video_engine_busy;
        uint64_t compute_engine_busy;
        uint64_t sampler_busy;
        uint64_t pixel_backend_busy;
        float gpu_utilization;
        float memory_utilization;
    } perf_counters;
    
    // Driver state
    bool initialized;
    bool suspended;
    hal_mutex_t* device_mutex;
    hal_spinlock_t* ring_lock;
} intel_gpu_device_t;

// Intel GPU driver operations
extern gpu_vendor_ops_t intel_arc_ops;

// Intel GPU management functions
int intel_arc_probe(device_t* device, const device_id_t* id);
int intel_arc_remove(device_t* device);
int intel_arc_init_device(gpu_device_t* gpu);
void intel_arc_cleanup_device(gpu_device_t* gpu);
int intel_arc_reset_device(gpu_device_t* gpu);

// Hardware detection
intel_gpu_sku_t intel_detect_sku(uint32_t device_id);
int intel_detect_xe_hpg_config(intel_gpu_device_t* igpu);
int intel_detect_memory_config(intel_gpu_device_t* igpu);
int intel_detect_display_outputs(intel_gpu_device_t* igpu);

// Memory management
int intel_alloc_memory(gpu_device_t* gpu, size_t size, uint32_t flags, gpu_buffer_t** buffer);
void intel_free_memory(gpu_device_t* gpu, gpu_buffer_t* buffer);
int intel_map_memory(gpu_device_t* gpu, gpu_buffer_t* buffer, void** cpu_addr);
void intel_unmap_memory(gpu_device_t* gpu, gpu_buffer_t* buffer);

// Command submission
int intel_submit_commands(gpu_device_t* gpu, gpu_command_buffer_t* cmd_buf);
int intel_wait_idle(gpu_device_t* gpu);
int intel_create_command_buffer(intel_gpu_device_t* igpu, uint32_t ring_type, gpu_command_buffer_t** cmd_buf);

// Shader management
int intel_create_shader(gpu_device_t* gpu, const void* bytecode, size_t size, gpu_shader_t** shader);
void intel_destroy_shader(gpu_device_t* gpu, gpu_shader_t* shader);
int intel_compile_gen_isa(const char* hlsl_source, void** isa, size_t* isa_size);

// Texture operations
int intel_create_texture(gpu_device_t* gpu, uint32_t width, uint32_t height, uint32_t format, gpu_texture_t** texture);
void intel_destroy_texture(gpu_device_t* gpu, gpu_texture_t* texture);
int intel_update_texture(gpu_device_t* gpu, gpu_texture_t* texture, const void* data, size_t size);

// XeSS operations
int intel_xess_init(intel_gpu_device_t* igpu);
int intel_xess_configure(intel_gpu_device_t* igpu, intel_xess_quality_t quality, uint32_t output_width, uint32_t output_height);
int intel_xess_upscale(intel_gpu_device_t* igpu, gpu_texture_t* input, gpu_texture_t* output, const void* motion_vectors);
int intel_xess_enable_motion_vectors(intel_gpu_device_t* igpu, bool enable);
int intel_xess_set_sharpening(intel_gpu_device_t* igpu, float factor);
void intel_xess_cleanup(intel_gpu_device_t* igpu);

// Ray tracing operations
int intel_rt_enable(intel_gpu_device_t* igpu, bool enable);
int intel_rt_build_acceleration_structure(intel_gpu_device_t* igpu, const void* geometry, void** as);
int intel_rt_dispatch_rays(intel_gpu_device_t* igpu, uint32_t width, uint32_t height, uint32_t depth);
int intel_rt_get_statistics(intel_gpu_device_t* igpu, uint64_t* rays_dispatched, uint64_t* intersections);

// Variable Rate Shading
int intel_vrs_enable(intel_gpu_device_t* igpu, bool enable);
int intel_vrs_set_per_draw_rate(intel_gpu_device_t* igpu, uint32_t rate);
int intel_vrs_set_per_primitive_rate(intel_gpu_device_t* igpu, gpu_texture_t* rate_image);

// Media operations
int intel_media_init(intel_gpu_device_t* igpu);
int intel_media_encode_h264(intel_gpu_device_t* igpu, const void* frame, void* output, size_t* output_size);
int intel_media_encode_h265(intel_gpu_device_t* igpu, const void* frame, void* output, size_t* output_size);
int intel_media_encode_av1(intel_gpu_device_t* igpu, const void* frame, void* output, size_t* output_size);
int intel_media_decode_h264(intel_gpu_device_t* igpu, const void* input, void* output, size_t* output_size);
int intel_media_decode_h265(intel_gpu_device_t* igpu, const void* input, void* output, size_t* output_size);
int intel_media_decode_av1(intel_gpu_device_t* igpu, const void* input, void* output, size_t* output_size);
void intel_media_cleanup(intel_gpu_device_t* igpu);

// Power management
int intel_set_power_state(gpu_device_t* gpu, uint32_t state);
int intel_get_temperature(gpu_device_t* gpu, uint32_t* temp);
int intel_set_fan_speed(gpu_device_t* gpu, uint32_t speed);
int intel_set_frequency(intel_gpu_device_t* igpu, uint32_t freq);
int intel_enable_turbo(intel_gpu_device_t* igpu, bool enable);
int intel_set_power_limits(intel_gpu_device_t* igpu, uint32_t pl1, uint32_t pl2);

// Performance monitoring
int intel_get_metrics(gpu_device_t* gpu, gpu_performance_metrics_t* metrics);
int intel_read_performance_counters(intel_gpu_device_t* igpu);
int intel_get_engine_utilization(intel_gpu_device_t* igpu, uint32_t engine, float* utilization);

// GuC (Graphics Micro Controller)
int intel_guc_init(intel_gpu_device_t* igpu);
int intel_guc_load_firmware(intel_gpu_device_t* igpu);
int intel_guc_enable_submission(intel_gpu_device_t* igpu, bool enable);
int intel_guc_submit_command(intel_gpu_device_t* igpu, gpu_command_buffer_t* cmd_buf);
void intel_guc_cleanup(intel_gpu_device_t* igpu);

// HuC (HEVC Micro Controller)
int intel_huc_init(intel_gpu_device_t* igpu);
int intel_huc_load_firmware(intel_gpu_device_t* igpu);
int intel_huc_authenticate(intel_gpu_device_t* igpu);
void intel_huc_cleanup(intel_gpu_device_t* igpu);

// Compute support (OpenCL/SYCL)
int intel_compute_init(intel_gpu_device_t* igpu);
int intel_compute_dispatch(intel_gpu_device_t* igpu, uint32_t grid_x, uint32_t grid_y, uint32_t grid_z);
int intel_compute_memcpy(intel_gpu_device_t* igpu, void* dst, const void* src, size_t size);
void intel_compute_cleanup(intel_gpu_device_t* igpu);

// Deep Link technology
int intel_deep_link_init(intel_gpu_device_t* igpu);
int intel_deep_link_encode_acceleration(intel_gpu_device_t* igpu, bool enable);
int intel_deep_link_transcode_optimization(intel_gpu_device_t* igpu, uint32_t quality);

// Utility functions
const char* intel_sku_to_string(intel_gpu_sku_t sku);
const char* intel_generation_to_string(intel_gpu_generation_t gen);
uint32_t intel_read_reg(intel_gpu_device_t* igpu, uint32_t offset);
void intel_write_reg(intel_gpu_device_t* igpu, uint32_t offset, uint32_t value);
int intel_wait_for_idle(intel_gpu_device_t* igpu, uint32_t timeout_ms);

// Interrupt handling
void intel_irq_handler(device_t* device, int irq, void* data);
int intel_enable_interrupts(intel_gpu_device_t* igpu);
void intel_disable_interrupts(intel_gpu_device_t* igpu);

// Firmware management
int intel_load_firmware(intel_gpu_device_t* igpu, const char* firmware_name, void** firmware, size_t* size);
int intel_verify_firmware(const void* firmware, size_t size);

// Device table for supported Intel Arc GPUs
extern device_id_t intel_arc_device_table[];
extern size_t intel_arc_device_table_size;

#ifdef __cplusplus
}
#endif

#endif // INTEL_ARC_H