/**
 * @file amd_rdna3.h
 * @brief AMD RDNA3 GPU Driver for RaeenOS
 * 
 * Advanced driver implementation for AMD RX 7900 XTX/XT and other RDNA3 GPUs
 * with hardware ray tracing, FSR 3.0, and advanced compute capabilities.
 * 
 * Features:
 * - RDNA3 architecture with dual-issue SIMD units
 * - Hardware ray tracing with Ray Accelerator units
 * - FSR 3.0 with Frame Generation and fluid motion
 * - AV1 encoding and decoding support
 * - Infinity Cache for reduced memory bandwidth
 * - Variable Rate Shading (VRS) Tier 2
 * - Mesh shaders and primitive shaders
 * - Advanced power management with RDNA3 efficiency
 * - Smart Access Memory optimization
 * 
 * Supported GPUs:
 * - RX 7900 XTX (Navi 31 XTX)
 * - RX 7900 XT (Navi 31 XT) 
 * - RX 7800 XT (Navi 32 XT)
 * - RX 7700 XT (Navi 32 XL)
 * - RX 7600 XT (Navi 33 XT)
 * - RX 7600 (Navi 33 XL)
 * 
 * Author: RaeenOS AMD GPU Team
 * License: MIT
 * Version: 1.0.0
 */

#ifndef AMD_RDNA3_H
#define AMD_RDNA3_H

#include "../gpu.h"
#include "../../kernel/include/types.h"
#include "../../kernel/include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// AMD GPU device IDs (RDNA3)
#define AMD_RX_7900_XTX_DEVICE_ID     0x744C
#define AMD_RX_7900_XT_DEVICE_ID      0x7448
#define AMD_RX_7800_XT_DEVICE_ID      0x7700
#define AMD_RX_7700_XT_DEVICE_ID      0x7701
#define AMD_RX_7600_XT_DEVICE_ID      0x7800
#define AMD_RX_7600_DEVICE_ID         0x7801

// AMD GPU ASICs (RDNA3)
typedef enum {
    AMD_ASIC_NAVI31_XTX,    // RX 7900 XTX
    AMD_ASIC_NAVI31_XT,     // RX 7900 XT
    AMD_ASIC_NAVI32_XT,     // RX 7800 XT
    AMD_ASIC_NAVI32_XL,     // RX 7700 XT
    AMD_ASIC_NAVI33_XT,     // RX 7600 XT
    AMD_ASIC_NAVI33_XL      // RX 7600
} amd_asic_t;

// AMD register blocks
#define AMD_MMIO_BASE                 0x00000000
#define AMD_GC_BASE                   0x00001260  // Graphics and Compute
#define AMD_DCE_BASE                  0x000034C0  // Display Controller
#define AMD_VCN_BASE                  0x00007800  // Video Codec Next
#define AMD_MP0_BASE                  0x00016000  // Management Processor
#define AMD_NBIO_BASE                 0x00000000  // North Bridge IO
#define AMD_PCIE_BASE                 0x00000000  // PCIe interface

// Graphics and Compute registers
#define AMD_GC_USER_QUEUE_RPTR        (AMD_GC_BASE + 0x0000)
#define AMD_GC_USER_QUEUE_WPTR        (AMD_GC_BASE + 0x0004)
#define AMD_GC_CNTL                   (AMD_GC_BASE + 0x0008)
#define AMD_GC_STATUS                 (AMD_GC_BASE + 0x000C)
#define AMD_GC_COMPUTE_DISPATCH       (AMD_GC_BASE + 0x1000)
#define AMD_GC_GRAPHICS_SUBMIT        (AMD_GC_BASE + 0x2000)

// Ray tracing registers (RDNA3)
#define AMD_RT_BASE                   (AMD_GC_BASE + 0x3000)
#define AMD_RT_CONTROL                (AMD_RT_BASE + 0x0000)
#define AMD_RT_STATUS                 (AMD_RT_BASE + 0x0004)
#define AMD_RT_BVH_BASE               (AMD_RT_BASE + 0x0008)
#define AMD_RT_ACCEL_STRUCT           (AMD_RT_BASE + 0x000C)

// Memory controller registers
#define AMD_MC_BASE                   0x00002800
#define AMD_MC_VM_CONTEXT0_CNTL       (AMD_MC_BASE + 0x0000)
#define AMD_MC_VM_INVALIDATE_ENG17    (AMD_MC_BASE + 0x0044)
#define AMD_MC_VM_L2_CNTL             (AMD_MC_BASE + 0x1400)
#define AMD_MC_VM_L2_STATUS           (AMD_MC_BASE + 0x1404)

// Power management registers
#define AMD_PWR_BASE                  0x00016C00
#define AMD_PWR_MISC_CNTL_STATUS      (AMD_PWR_BASE + 0x0000)
#define AMD_PWR_CLK_FREQ_INFO         (AMD_PWR_BASE + 0x0004)
#define AMD_PWR_VOLTAGE_FREQ_INFO     (AMD_PWR_BASE + 0x0008)

// Thermal registers
#define AMD_THM_BASE                  0x00016600
#define AMD_THM_TCON_CUR_TMP          (AMD_THM_BASE + 0x0000)
#define AMD_THM_TCON_HTC              (AMD_THM_BASE + 0x0004)
#define AMD_THM_PWM_CTRL              (AMD_THM_BASE + 0x0008)

// FSR quality modes
typedef enum {
    FSR_QUALITY_ULTRA_PERFORMANCE = 0,  // 3x upscale
    FSR_QUALITY_PERFORMANCE      = 1,   // 2x upscale
    FSR_QUALITY_BALANCED         = 2,   // 1.7x upscale
    FSR_QUALITY_QUALITY          = 3,   // 1.5x upscale
    FSR_QUALITY_NATIVE_AA        = 4    // 1x with anti-aliasing
} amd_fsr_quality_t;

// FSR features
typedef enum {
    FSR_FEATURE_UPSCALING       = (1 << 0),
    FSR_FEATURE_FRAME_GEN       = (1 << 1),
    FSR_FEATURE_FLUID_MOTION    = (1 << 2),
    FSR_FEATURE_ANTI_LAG        = (1 << 3)
} amd_fsr_features_t;

// RDNA3 compute units
typedef struct amd_compute_unit {
    uint32_t simd_count;            // SIMD units per CU
    uint32_t workgroup_processors;  // WGPs
    uint32_t stream_processors;     // Stream processors
    uint32_t texture_units;         // Texture units
    uint32_t l0_cache_size;         // L0 vector cache
    uint32_t lds_size;              // Local Data Share
} amd_compute_unit_t;

// AMD specific GPU device structure
typedef struct amd_gpu_device {
    gpu_device_t base;              // Base GPU device
    
    // ASIC information
    amd_asic_t asic;
    uint32_t asic_revision;
    uint32_t pci_revision;
    
    // RDNA3 architecture details
    struct {
        uint32_t shader_engines;    // Shader Engines
        uint32_t shader_arrays;     // Shader Arrays per SE
        uint32_t compute_units;     // Compute Units total
        uint32_t ray_accelerators;  // Ray Accelerator units
        uint32_t rops;              // Render Output Units
        uint32_t tmus;              // Texture Mapping Units
        amd_compute_unit_t cu_spec; // CU specifications
    } rdna3;
    
    // Memory subsystem
    struct {
        uint64_t vram_size;         // GDDR6 VRAM size
        uint32_t memory_bus_width;  // Memory bus width
        uint32_t memory_channels;   // Memory channels
        uint64_t memory_bandwidth;  // Memory bandwidth GB/s
        uint32_t infinity_cache_size; // Infinity Cache (MB)
        bool smart_access_memory;   // SAM support
    } memory;
    
    // Clock domains
    struct {
        uint32_t base_gfx_clock;    // Base graphics clock
        uint32_t game_gfx_clock;    // Game graphics clock  
        uint32_t boost_gfx_clock;   // Boost graphics clock
        uint32_t base_mem_clock;    // Base memory clock
        uint32_t boost_mem_clock;   // Boost memory clock
        uint32_t soc_clock;         // SoC clock
        uint32_t fabric_clock;      // Fabric clock
    } clocks;
    
    // Power management (RDNA3 efficiency)
    struct {
        uint32_t tgp;               // Total Graphics Power
        uint32_t tdt;               // Total Design Thermal
        uint32_t current_power_limit;
        uint32_t max_power_limit;
        bool gpu_scaling_enabled;
        uint32_t voltage_offset;    // mV
        uint32_t power_play_table_version;
    } power;
    
    // Thermal management
    struct {
        uint32_t max_temp;          // Junction temperature
        uint32_t throttle_temp;     // Thermal throttle temp
        uint32_t current_temp;      // Current temperature
        uint32_t hotspot_temp;      // Hotspot temperature
        uint32_t fan_speed;         // Fan speed %
        bool zero_rpm_mode;         // Zero RPM fan mode
    } thermal;
    
    // FSR state
    struct {
        bool initialized;
        amd_fsr_quality_t quality_mode;
        uint32_t supported_features;
        bool frame_generation_enabled;
        bool fluid_motion_enabled;
        float sharpening;
        uint32_t motion_vector_scale;
    } fsr;
    
    // Ray tracing state (RDNA3)
    struct {
        bool enabled;
        uint32_t ra_utilization;    // Ray Accelerator utilization
        uint64_t rays_traced;
        uint64_t bvh_traversals;
        uint32_t rt_pipeline_depth;
    } ray_tracing;
    
    // Video codec support
    struct {
        bool av1_decode;
        bool av1_encode;
        bool h264_decode;
        bool h264_encode;
        bool h265_decode;
        bool h265_encode;
        uint32_t max_decode_sessions;
        uint32_t max_encode_sessions;
    } vcn;
    
    // Display capabilities
    struct {
        uint32_t display_controllers;
        bool hdmi_21_support;
        bool dp_20_support;
        bool dsc_support;           // Display Stream Compression
        bool hdr_support;
        uint32_t max_hdmi_tmds_clock;
        uint32_t max_dp_link_rate;
    } display;
    
    // Infinity Cache
    struct {
        uint32_t total_size;        // Total cache size (MB)
        uint32_t active_size;       // Active cache size
        uint64_t hit_rate;          // Cache hit rate
        uint64_t bandwidth_savings; // Bandwidth saved
    } infinity_cache;
    
    // Register mappings
    volatile uint32_t* mmio_regs;
    size_t mmio_size;
    
    // Command submission
    void* gfx_ring;                 // Graphics command ring
    void* compute_ring;             // Compute command ring
    void* dma_ring;                 // DMA command ring
    size_t ring_size;
    
    // Interrupt handling
    int irq_line;
    uint32_t irq_sources;
    
    // Performance counters
    struct {
        uint64_t gfx_busy_cycles;
        uint64_t compute_busy_cycles;
        uint64_t memory_controller_busy;
        uint64_t infinity_cache_hits;
        uint64_t infinity_cache_misses;
        float gpu_utilization;
        float memory_utilization;
    } perf_counters;
    
    // Driver state
    bool initialized;
    hal_mutex_t* device_mutex;
    hal_spinlock_t* ring_lock;
} amd_gpu_device_t;

// AMD GPU driver operations
extern gpu_vendor_ops_t amd_rdna3_ops;

// AMD GPU management functions
int amd_rdna3_probe(device_t* device, const device_id_t* id);
int amd_rdna3_remove(device_t* device);
int amd_rdna3_init_device(gpu_device_t* gpu);
void amd_rdna3_cleanup_device(gpu_device_t* gpu);
int amd_rdna3_reset_device(gpu_device_t* gpu);

// Hardware detection
amd_asic_t amd_detect_asic(uint32_t device_id);
int amd_detect_rdna3_config(amd_gpu_device_t* amdgpu);
int amd_detect_memory_config(amd_gpu_device_t* amdgpu);
int amd_detect_display_outputs(amd_gpu_device_t* amdgpu);

// Memory management
int amd_alloc_memory(gpu_device_t* gpu, size_t size, uint32_t flags, gpu_buffer_t** buffer);
void amd_free_memory(gpu_device_t* gpu, gpu_buffer_t* buffer);
int amd_map_memory(gpu_device_t* gpu, gpu_buffer_t* buffer, void** cpu_addr);
void amd_unmap_memory(gpu_device_t* gpu, gpu_buffer_t* buffer);

// Command submission
int amd_submit_commands(gpu_device_t* gpu, gpu_command_buffer_t* cmd_buf);
int amd_wait_idle(gpu_device_t* gpu);
int amd_create_command_buffer(amd_gpu_device_t* amdgpu, uint32_t ring_type, gpu_command_buffer_t** cmd_buf);

// Shader management
int amd_create_shader(gpu_device_t* gpu, const void* bytecode, size_t size, gpu_shader_t** shader);
void amd_destroy_shader(gpu_device_t* gpu, gpu_shader_t* shader);
int amd_compile_shader(const char* hlsl_source, const char* target, void** bytecode, size_t* bytecode_size);

// Texture operations
int amd_create_texture(gpu_device_t* gpu, uint32_t width, uint32_t height, uint32_t format, gpu_texture_t** texture);
void amd_destroy_texture(gpu_device_t* gpu, gpu_texture_t* texture);
int amd_update_texture(gpu_device_t* gpu, gpu_texture_t* texture, const void* data, size_t size);

// FSR operations
int amd_fsr_init(amd_gpu_device_t* amdgpu);
int amd_fsr_configure(amd_gpu_device_t* amdgpu, amd_fsr_quality_t quality, uint32_t output_width, uint32_t output_height);
int amd_fsr_upscale(amd_gpu_device_t* amdgpu, gpu_texture_t* input, gpu_texture_t* output, float sharpening);
int amd_fsr_enable_frame_generation(amd_gpu_device_t* amdgpu, bool enable);
int amd_fsr_enable_fluid_motion(amd_gpu_device_t* amdgpu, bool enable);
void amd_fsr_cleanup(amd_gpu_device_t* amdgpu);

// Ray tracing operations
int amd_rt_enable(amd_gpu_device_t* amdgpu, bool enable);
int amd_rt_build_bvh(amd_gpu_device_t* amdgpu, const void* geometry, void** bvh);
int amd_rt_trace_rays(amd_gpu_device_t* amdgpu, uint32_t width, uint32_t height, uint32_t depth);
int amd_rt_get_statistics(amd_gpu_device_t* amdgpu, uint64_t* rays_traced, uint64_t* bvh_traversals);

// Video codec operations
int amd_vcn_init(amd_gpu_device_t* amdgpu);
int amd_vcn_decode_h264(amd_gpu_device_t* amdgpu, const void* input, void* output, size_t* output_size);
int amd_vcn_decode_h265(amd_gpu_device_t* amdgpu, const void* input, void* output, size_t* output_size);
int amd_vcn_decode_av1(amd_gpu_device_t* amdgpu, const void* input, void* output, size_t* output_size);
int amd_vcn_encode_h264(amd_gpu_device_t* amdgpu, const void* frame, void* output, size_t* output_size);
int amd_vcn_encode_h265(amd_gpu_device_t* amdgpu, const void* frame, void* output, size_t* output_size);
int amd_vcn_encode_av1(amd_gpu_device_t* amdgpu, const void* frame, void* output, size_t* output_size);
void amd_vcn_cleanup(amd_gpu_device_t* amdgpu);

// Power management
int amd_set_power_state(gpu_device_t* gpu, uint32_t state);
int amd_get_temperature(gpu_device_t* gpu, uint32_t* temp);
int amd_set_fan_speed(gpu_device_t* gpu, uint32_t speed);
int amd_set_power_limit(amd_gpu_device_t* amdgpu, uint32_t watts);
int amd_enable_gpu_scaling(amd_gpu_device_t* amdgpu, bool enable);
int amd_set_clock_frequencies(amd_gpu_device_t* amdgpu, uint32_t gfx_clock, uint32_t mem_clock);

// Performance monitoring
int amd_get_metrics(gpu_device_t* gpu, gpu_performance_metrics_t* metrics);
int amd_read_performance_counters(amd_gpu_device_t* amdgpu);
int amd_get_gpu_utilization(amd_gpu_device_t* amdgpu, float* utilization);
int amd_get_memory_utilization(amd_gpu_device_t* amdgpu, float* utilization);
int amd_get_infinity_cache_stats(amd_gpu_device_t* amdgpu, uint64_t* hits, uint64_t* misses);

// Smart Access Memory
int amd_enable_sam(amd_gpu_device_t* amdgpu, bool enable);
int amd_configure_sam(amd_gpu_device_t* amdgpu, uint64_t aperture_size);

// Multi-GPU support (CrossFire)
int amd_create_crossfire_group(amd_gpu_device_t** gpus, uint32_t count);
int amd_balance_crossfire_workload(amd_gpu_device_t** gpus, uint32_t count, const void* workload);

// Compute support (OpenCL/ROCm)
int amd_compute_init(amd_gpu_device_t* amdgpu);
int amd_compute_dispatch(amd_gpu_device_t* amdgpu, uint32_t grid_x, uint32_t grid_y, uint32_t grid_z);
int amd_compute_memcpy(amd_gpu_device_t* amdgpu, void* dst, const void* src, size_t size);
void amd_compute_cleanup(amd_gpu_device_t* amdgpu);

// Utility functions
const char* amd_asic_to_string(amd_asic_t asic);
uint32_t amd_read_reg(amd_gpu_device_t* amdgpu, uint32_t offset);
void amd_write_reg(amd_gpu_device_t* amdgpu, uint32_t offset, uint32_t value);
int amd_wait_for_idle(amd_gpu_device_t* amdgpu, uint32_t timeout_ms);

// Interrupt handling
void amd_irq_handler(device_t* device, int irq, void* data);
int amd_enable_interrupts(amd_gpu_device_t* amdgpu);
void amd_disable_interrupts(amd_gpu_device_t* amdgpu);

// BIOS and firmware
int amd_load_gpu_firmware(amd_gpu_device_t* amdgpu);
int amd_parse_vbios(amd_gpu_device_t* amdgpu);
int amd_init_power_play_tables(amd_gpu_device_t* amdgpu);

// Device table for supported AMD RDNA3 GPUs
extern device_id_t amd_rdna3_device_table[];
extern size_t amd_rdna3_device_table_size;

#ifdef __cplusplus
}
#endif

#endif // AMD_RDNA3_H