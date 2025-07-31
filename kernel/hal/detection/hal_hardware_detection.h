/**
 * @file hal_hardware_detection.h
 * @brief HAL Hardware Detection and Compatibility Database Header
 * 
 * This header defines structures and functions for comprehensive hardware
 * detection and compatibility management across diverse platforms.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#ifndef HAL_HARDWARE_DETECTION_H
#define HAL_HARDWARE_DETECTION_H

#include "../../include/hal_interface.h"
#include "../../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum limits
#define HAL_MAX_HARDWARE_COMPONENTS     512
#define HAL_MAX_HARDWARE_PROFILES       32
#define HAL_MAX_COMPATIBILITY_ENTRIES   2048
#define HAL_MAX_QUIRK_ENTRIES           256
#define HAL_MAX_HARDWARE_REQUIREMENTS   64

// Special IDs for wildcard matching
#define HAL_VENDOR_ID_ANY               0xFFFFFFFF
#define HAL_DEVICE_ID_ANY               0xFFFFFFFF

// Hardware types
typedef enum {
    HAL_HW_TYPE_UNKNOWN,
    HAL_HW_TYPE_CPU,
    HAL_HW_TYPE_MEMORY,
    HAL_HW_TYPE_STORAGE,
    HAL_HW_TYPE_NETWORK,
    HAL_HW_TYPE_GRAPHICS,
    HAL_HW_TYPE_AUDIO,
    HAL_HW_TYPE_USB,
    HAL_HW_TYPE_BLUETOOTH,
    HAL_HW_TYPE_WIRELESS,
    HAL_HW_TYPE_CAMERA,
    HAL_HW_TYPE_SENSOR,
    HAL_HW_TYPE_POWER,
    HAL_HW_TYPE_THERMAL,
    HAL_HW_TYPE_SECURITY,
    HAL_HW_TYPE_BRIDGE,
    HAL_HW_TYPE_INPUT,
    HAL_HW_TYPE_DISPLAY,
    HAL_HW_TYPE_PLATFORM,
    HAL_HW_TYPE_ANY = 0xFF
} hal_hardware_type_t;

// Compatibility levels
typedef enum {
    HAL_COMPAT_UNKNOWN,         // Compatibility unknown
    HAL_COMPAT_UNSUPPORTED,     // Not supported
    HAL_COMPAT_LIMITED,         // Basic functionality only
    HAL_COMPAT_PARTIAL,         // Most features work
    HAL_COMPAT_FULL,            // All features supported
    HAL_COMPAT_NATIVE,          // Native/optimal support
    HAL_COMPAT_DEPRECATED       // Supported but deprecated
} hal_compatibility_level_t;

// Hardware capability flags
#define HAL_HW_CAP_NONE             0x00000000
#define HAL_HW_CAP_DMA              0x00000001
#define HAL_HW_CAP_BUS_MASTER       0x00000002
#define HAL_HW_CAP_POWER_MGMT       0x00000004
#define HAL_HW_CAP_MSI              0x00000008
#define HAL_HW_CAP_MSIX             0x00000010
#define HAL_HW_CAP_64BIT_ADDR       0x00000020
#define HAL_HW_CAP_HOTPLUG          0x00000040
#define HAL_HW_CAP_WAKE_ON_LAN      0x00000080
#define HAL_HW_CAP_RESET            0x00000100
#define HAL_HW_CAP_VIRTUALIZATION   0x00000200
#define HAL_HW_CAP_CRYPTO           0x00000400
#define HAL_HW_CAP_COMPRESS         0x00000800
#define HAL_HW_CAP_FPU              0x00001000
#define HAL_HW_CAP_SIMD             0x00002000
#define HAL_HW_CAP_NETWORK          0x00004000
#define HAL_HW_CAP_GRAPHICS         0x00008000
#define HAL_HW_CAP_AUDIO            0x00010000
#define HAL_HW_CAP_USB              0x00020000
#define HAL_HW_CAP_USB2             0x00040000
#define HAL_HW_CAP_USB3             0x00080000
#define HAL_HW_CAP_BLUETOOTH        0x00100000
#define HAL_HW_CAP_WIRELESS         0x00200000
#define HAL_HW_CAP_GPU_COMPUTE      0x00400000
#define HAL_HW_CAP_AI_ACCEL         0x00800000
#define HAL_HW_CAP_NVME             0x01000000
#define HAL_HW_CAP_SATA             0x02000000
#define HAL_HW_CAP_PCIE_GEN3        0x04000000
#define HAL_HW_CAP_PCIE_GEN4        0x08000000
#define HAL_HW_CAP_PCIE_GEN5        0x10000000
#define HAL_HW_CAP_THUNDERBOLT      0x20000000

// Hardware resource information
typedef struct {
    phys_addr_t base_address;
    size_t memory_size;
    uint32_t io_base;
    uint32_t io_size;
    int irq;
    int irq_count;
    uint32_t dma_channels;
} hal_hardware_resource_t;

// Hardware component structure
typedef struct {
    hal_hardware_type_t type;
    char name[64];
    char description[128];
    char manufacturer[64];
    char model[64];
    char version[32];
    char driver_name[32];
    
    // Hardware identification
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_vendor_id;
    uint32_t subsystem_device_id;
    uint32_t class_id;
    uint32_t subclass_id;
    uint32_t revision;
    
    // Bus information
    hal_bus_type_t bus_type;
    uint32_t bus_address;
    uint32_t slot_number;
    
    // Capabilities and features
    uint64_t capabilities;
    uint32_t max_power_consumption; // in mW
    uint32_t current_power_state;
    
    // Resource allocation
    hal_hardware_resource_t resource_info;
    
    // Compatibility information
    hal_compatibility_level_t compatibility_level;
    uint32_t compatibility_flags;
    uint64_t quirks_applied;        // Bitmask of applied quirks
    
    // Performance metrics
    uint32_t performance_rating;    // 0-100 scale
    uint32_t power_efficiency;      // 0-100 scale
    uint32_t heat_generation;       // Temperature delta in °C
    
    // State information
    bool present;
    bool enabled;
    bool initialized;
    bool error_state;
    char error_message[128];
    
    // Detection metadata
    uint64_t detection_timestamp;
    uint32_t detection_method;      // How was this component detected
    uint32_t confidence_level;      // Detection confidence (0-100)
} hal_hardware_component_t;

// Detected hardware structure
typedef struct {
    hal_hardware_component_t components[HAL_MAX_HARDWARE_COMPONENTS];
    size_t component_count;
    
    // Detection metadata
    uint64_t detection_timestamp;
    uint64_t detection_duration;
    uint32_t detection_version;
    char detection_method[32];
    
    // Summary statistics
    uint32_t cpu_count;
    uint64_t total_memory;
    uint32_t storage_devices;
    uint32_t network_devices;
    uint32_t graphics_devices;
    uint32_t audio_devices;
    uint32_t usb_devices;
    uint32_t unknown_devices;
} hal_detected_hardware_t;

// Hardware requirement for profiles
typedef struct {
    hal_hardware_type_t hardware_type;
    uint32_t vendor_id;             // HAL_VENDOR_ID_ANY for any
    uint32_t device_id;             // HAL_DEVICE_ID_ANY for any
    uint64_t required_capabilities; // Required capability flags
    uint32_t min_performance;       // Minimum performance rating
    bool required;                  // Is this component required?
    char description[64];
} hal_hardware_requirement_t;

// Hardware profile (for system classification)
typedef struct {
    char name[64];
    char description[256];
    char target_market[64];         // Desktop, Server, Mobile, etc.
    
    // Requirements
    hal_hardware_requirement_t requirements[HAL_MAX_HARDWARE_REQUIREMENTS];
    size_t requirement_count;
    
    // Recommended settings
    struct {
        hal_cpu_governor_t cpu_governor;
        uint32_t memory_policy;
        uint32_t io_scheduler;
        bool enable_power_saving;
        bool enable_performance_mode;
    } recommended_settings;
    
    // Profile characteristics
    uint32_t performance_weight;    // Weight given to performance
    uint32_t power_weight;          // Weight given to power efficiency
    uint32_t compatibility_weight;  // Weight given to compatibility
    
    // Validation
    uint32_t validation_score;      // How well does current HW match
    bool validated;                 // Has this profile been validated
} hal_hardware_profile_t;

// Compatibility database entry
typedef struct {
    uint32_t vendor_id;
    uint32_t device_id;
    hal_hardware_type_t hardware_type;
    hal_compatibility_level_t compatibility_level;
    
    char notes[256];
    char driver_name[32];
    char minimum_version[16];
    
    // Supported features
    uint64_t supported_features;
    uint64_t unsupported_features;
    
    // Performance characteristics
    uint32_t performance_rating;
    uint32_t stability_rating;
    uint32_t maturity_rating;
    
    // Version information
    uint32_t first_supported_version;
    uint32_t last_supported_version;
    bool deprecated;
    char replacement_recommendation[64];
} hal_compatibility_entry_t;

// Hardware quirk entry
typedef struct {
    uint32_t vendor_id;
    uint32_t device_id;
    hal_hardware_type_t hardware_type;
    
    char description[128];
    char workaround[256];
    
    // Quirk function and data
    int (*quirk_function)(hal_hardware_component_t* component, void* data);
    void* quirk_data;
    
    // Applicability
    uint32_t affected_versions;     // Bitmask of affected versions
    bool always_apply;              // Apply regardless of version
    uint32_t severity;              // 1=cosmetic, 5=critical
} hal_quirk_entry_t;

// Compatibility report
typedef struct {
    uint32_t total_components;
    uint32_t native_supported;
    uint32_t fully_supported;
    uint32_t partially_supported;
    uint32_t limited_support;
    uint32_t unsupported;
    uint32_t unknown;
    
    uint32_t compatibility_score;   // Overall score 0-100
    hal_compatibility_level_t overall_compatibility;
    
    // Specific compatibility issues
    struct {
        hal_hardware_component_t* component;
        char issue_description[128];
        char recommendation[128];
        uint32_t severity;
    } issues[32];
    size_t issue_count;
    
    // Recommendations
    char recommendations[512];
    bool requires_updates;
    bool requires_drivers;
    bool has_workarounds;
} hal_compatibility_report_t;

// Hardware statistics
typedef struct {
    uint32_t total_devices;
    uint32_t cpu_count;
    uint32_t memory_devices;
    uint32_t storage_devices;
    uint32_t network_devices;
    uint32_t graphics_devices;
    uint32_t audio_devices;
    uint32_t usb_devices;
    uint32_t other_devices;
    
    // Compatibility breakdown
    uint32_t native_supported;
    uint32_t fully_supported;
    uint32_t partially_supported;
    uint32_t limited_support;
    uint32_t unsupported;
    uint32_t unknown;
    
    // Detection performance
    uint32_t detection_time_ms;
    uint32_t quirks_applied;
    uint32_t errors_encountered;
} hal_hardware_stats_t;

// Callback function types
typedef void (*hal_hardware_detection_callback_t)(hal_hardware_component_t* component);
typedef hal_compatibility_level_t (*hal_compatibility_override_t)(uint32_t vendor_id, uint32_t device_id, hal_hardware_type_t type);

// Function prototypes

// Initialization and management
int hal_hardware_detection_init(void);
void hal_hardware_detection_shutdown(void);

// Hardware detection
int hal_hardware_detect_all(void);
int hal_hardware_detect_type(hal_hardware_type_t type);
int hal_hardware_rescan(void);

// Hardware information retrieval
int hal_hardware_get_detected(hal_detected_hardware_t* hardware);
hal_hardware_component_t* hal_hardware_find_component(uint32_t vendor_id, uint32_t device_id);
int hal_hardware_get_components_by_type(hal_hardware_type_t type, hal_hardware_component_t** components, size_t* count);
int hal_hardware_get_component_info(hal_hardware_component_t* component, char* buffer, size_t buffer_size);

// Compatibility management
hal_compatibility_level_t hal_hardware_check_compatibility(uint32_t vendor_id, uint32_t device_id, hal_hardware_type_t type);
int hal_hardware_get_compatibility_report(hal_compatibility_report_t* report);
int hal_hardware_update_compatibility_db(const char* filename);
int hal_hardware_export_compatibility_report(const char* filename);

// Hardware profiles
hal_hardware_profile_t* hal_hardware_get_profile(void);
int hal_hardware_set_profile(const char* profile_name);
int hal_hardware_create_custom_profile(const hal_hardware_profile_t* profile);
int hal_hardware_list_profiles(char profiles[][64], size_t* count);

// Hardware requirements validation
bool hal_hardware_meets_requirements(const hal_hardware_requirement_t* requirements, size_t count);
int hal_hardware_validate_system(hal_hardware_profile_t* profile, float* score);
int hal_hardware_get_missing_requirements(const hal_hardware_requirement_t* requirements, size_t count, 
                                         hal_hardware_requirement_t* missing, size_t* missing_count);

// Quirks and workarounds
int hal_hardware_apply_quirks(hal_hardware_component_t* component);
int hal_hardware_get_applied_quirks(hal_hardware_component_t* component, hal_quirk_entry_t* quirks, size_t* count);
int hal_hardware_register_quirk(const hal_quirk_entry_t* quirk);

// Statistics and reporting
int hal_hardware_get_statistics(hal_hardware_stats_t* stats);
void hal_hardware_print_summary(void);
void hal_hardware_dump_detected_hardware(void);
int hal_hardware_generate_report(const char* filename);

// Callback registration
int hal_hardware_register_detection_callback(hal_hardware_detection_callback_t callback);
int hal_hardware_unregister_detection_callback(hal_hardware_detection_callback_t callback);
int hal_hardware_register_compatibility_override(hal_compatibility_override_t override);

// Power and thermal management
int hal_hardware_get_power_consumption(uint32_t* total_power_mw);
int hal_hardware_get_thermal_status(uint32_t* max_temp_celsius);
int hal_hardware_enable_power_saving(bool enable);

// Hardware capability queries
bool hal_hardware_has_capability(hal_hardware_type_t type, uint64_t capability);
uint64_t hal_hardware_get_capabilities(hal_hardware_type_t type);
int hal_hardware_enable_capability(hal_hardware_component_t* component, uint64_t capability);

// Performance and benchmarking
int hal_hardware_benchmark_component(hal_hardware_component_t* component, uint32_t* performance_score);
int hal_hardware_get_performance_rating(hal_hardware_type_t type, uint32_t* rating);
int hal_hardware_optimize_for_hardware(void);

// Driver and firmware management
int hal_hardware_check_driver_status(hal_hardware_component_t* component);
int hal_hardware_update_firmware(hal_hardware_component_t* component, const char* firmware_path);
int hal_hardware_get_driver_recommendations(hal_hardware_component_t* component, char* recommendations, size_t size);

// Debugging and diagnostics
void hal_hardware_enable_debug_logging(bool enable);
int hal_hardware_run_diagnostics(hal_hardware_component_t* component);
int hal_hardware_get_diagnostic_info(hal_hardware_component_t* component, char* info, size_t size);

// Internal helper functions
hal_hardware_component_t* add_hardware_component(void);
uint32_t calculate_profile_match_score(hal_hardware_profile_t* profile);
const char* hal_hardware_type_to_string(hal_hardware_type_t type);
const char* hal_compatibility_level_to_string(hal_compatibility_level_t level);
const char* hal_bus_type_to_string(hal_bus_type_t type);

// Platform-specific detection functions
int detect_cpu_x86_64(hal_hardware_component_t* comp);
int detect_cpu_arm64(hal_hardware_component_t* comp);
int detect_platform_x86_64(void);
int detect_platform_arm64(void);

// Database initialization functions
void init_builtin_hardware_database(void);
void init_compatibility_database(void);
void init_quirks_database(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_HARDWARE_DETECTION_H