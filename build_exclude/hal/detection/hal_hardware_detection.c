/**
 * @file hal_hardware_detection.c
 * @brief HAL Hardware Detection and Compatibility Database
 * 
 * This module provides comprehensive hardware detection and maintains
 * a compatibility database for ensuring RaeenOS runs optimally across
 * diverse hardware platforms.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#include "../../include/hal_interface.h"
#include "../../include/types.h"
#include "../../include/errno.h"
#include "hal_hardware_detection.h"
#include <stddef.h>
#include <string.h>

// Hardware compatibility database
static struct {
    hal_hardware_profile_t profiles[HAL_MAX_HARDWARE_PROFILES];
    size_t profile_count;
    
    hal_compatibility_entry_t compatibility_db[HAL_MAX_COMPATIBILITY_ENTRIES];
    size_t compatibility_count;
    
    hal_quirk_entry_t quirks_db[HAL_MAX_QUIRK_ENTRIES];
    size_t quirk_count;
    
    hal_detected_hardware_t detected_hardware;
    hal_compatibility_report_t compatibility_report;
    
    bool initialized;
    bool detection_complete;
    
    // Detection callbacks
    hal_hardware_detection_callback_t detection_callbacks[16];
    size_t detection_callback_count;
    
    // Compatibility override functions
    hal_compatibility_override_t overrides[32];
    size_t override_count;
} hw_detection = {0};

// Forward declarations
static int detect_cpu_hardware(void);
static int detect_memory_hardware(void);
static int detect_storage_hardware(void);
static int detect_network_hardware(void);
static int detect_graphics_hardware(void);
static int detect_audio_hardware(void);
static int detect_usb_hardware(void);
static int detect_platform_hardware(void);
static int build_compatibility_report(void);
static int apply_hardware_quirks(void);
static void notify_hardware_detection(hal_hardware_component_t* component);

/**
 * Initialize hardware detection system
 */
int hal_hardware_detection_init(void)
{
    if (hw_detection.initialized) {
        return HAL_SUCCESS;
    }
    
    // Initialize builtin hardware database
    init_builtin_hardware_database();
    
    // Initialize compatibility database
    init_compatibility_database();
    
    // Initialize quirks database
    init_quirks_database();
    
    hw_detection.initialized = true;
    return HAL_SUCCESS;
}

/**
 * Perform complete hardware detection
 */
int hal_hardware_detect_all(void)
{
    if (!hw_detection.initialized) {
        return -EINVAL;
    }
    
    if (hw_detection.detection_complete) {
        return HAL_SUCCESS;
    }
    
    // Clear previous detection results
    memset(&hw_detection.detected_hardware, 0, sizeof(hal_detected_hardware_t));
    
    // Start detection timestamp
    hw_detection.detected_hardware.detection_timestamp = hal->timer_get_ticks();
    
    // Detect different hardware categories
    detect_cpu_hardware();
    detect_memory_hardware();
    detect_storage_hardware();
    detect_network_hardware();
    detect_graphics_hardware();
    detect_audio_hardware();
    detect_usb_hardware();
    detect_platform_hardware();
    
    // Build compatibility report
    build_compatibility_report();
    
    // Apply hardware-specific quirks
    apply_hardware_quirks();
    
    // End detection timestamp
    hw_detection.detected_hardware.detection_duration = 
        hal->timer_get_ticks() - hw_detection.detected_hardware.detection_timestamp;
    
    // Notify callbacks
    for (size_t i = 0; i < hw_detection.detected_hardware.component_count; i++) {
        notify_hardware_detection(&hw_detection.detected_hardware.components[i]);
    }
    
    hw_detection.detection_complete = true;
    return HAL_SUCCESS;
}

/**
 * Get detected hardware information
 */
int hal_hardware_get_detected(hal_detected_hardware_t* hardware)
{
    if (!hardware || !hw_detection.detection_complete) {
        return -EINVAL;
    }
    
    *hardware = hw_detection.detected_hardware;
    return HAL_SUCCESS;
}

/**
 * Get compatibility report
 */
int hal_hardware_get_compatibility_report(hal_compatibility_report_t* report)
{
    if (!report || !hw_detection.detection_complete) {
        return -EINVAL;
    }
    
    *report = hw_detection.compatibility_report;
    return HAL_SUCCESS;
}

/**
 * Check hardware compatibility
 */
hal_compatibility_level_t hal_hardware_check_compatibility(uint32_t vendor_id, 
                                                          uint32_t device_id, 
                                                          hal_hardware_type_t type)
{
    // Search compatibility database
    for (size_t i = 0; i < hw_detection.compatibility_count; i++) {
        hal_compatibility_entry_t* entry = &hw_detection.compatibility_db[i];
        
        if (entry->vendor_id == vendor_id && 
            entry->device_id == device_id && 
            entry->hardware_type == type) {
            return entry->compatibility_level;
        }
    }
    
    // Check for generic vendor support
    for (size_t i = 0; i < hw_detection.compatibility_count; i++) {
        hal_compatibility_entry_t* entry = &hw_detection.compatibility_db[i];
        
        if (entry->vendor_id == vendor_id && 
            entry->device_id == HAL_DEVICE_ID_ANY && 
            entry->hardware_type == type) {
            return entry->compatibility_level;
        }
    }
    
    // Unknown hardware
    return HAL_COMPAT_UNKNOWN;
}

/**
 * Get hardware profile
 */
hal_hardware_profile_t* hal_hardware_get_profile(void)
{
    if (!hw_detection.detection_complete) {
        return NULL;
    }
    
    // Find best matching profile based on detected hardware
    hal_hardware_profile_t* best_profile = NULL;
    uint32_t best_score = 0;
    
    for (size_t i = 0; i < hw_detection.profile_count; i++) {
        hal_hardware_profile_t* profile = &hw_detection.profiles[i];
        uint32_t score = calculate_profile_match_score(profile);
        
        if (score > best_score) {
            best_score = score;
            best_profile = profile;
        }
    }
    
    return best_profile;
}

/**
 * Register hardware detection callback
 */
int hal_hardware_register_detection_callback(hal_hardware_detection_callback_t callback)
{
    if (!callback || hw_detection.detection_callback_count >= 16) {
        return -EINVAL;
    }
    
    hw_detection.detection_callbacks[hw_detection.detection_callback_count++] = callback;
    return HAL_SUCCESS;
}

/**
 * Register compatibility override
 */
int hal_hardware_register_compatibility_override(hal_compatibility_override_t override)
{
    if (!override || hw_detection.override_count >= 32) {
        return -EINVAL;
    }
    
    hw_detection.overrides[hw_detection.override_count++] = override;
    return HAL_SUCCESS;
}

/**
 * Get hardware statistics
 */
int hal_hardware_get_statistics(hal_hardware_stats_t* stats)
{
    if (!stats || !hw_detection.detection_complete) {
        return -EINVAL;
    }
    
    memset(stats, 0, sizeof(hal_hardware_stats_t));
    
    // Count components by type
    for (size_t i = 0; i < hw_detection.detected_hardware.component_count; i++) {
        hal_hardware_component_t* comp = &hw_detection.detected_hardware.components[i];
        
        switch (comp->type) {
            case HAL_HW_TYPE_CPU:
                stats->cpu_count++;
                break;
            case HAL_HW_TYPE_MEMORY:
                stats->memory_devices++;
                break;
            case HAL_HW_TYPE_STORAGE:
                stats->storage_devices++;
                break;
            case HAL_HW_TYPE_NETWORK:
                stats->network_devices++;
                break;
            case HAL_HW_TYPE_GRAPHICS:
                stats->graphics_devices++;
                break;
            case HAL_HW_TYPE_AUDIO:
                stats->audio_devices++;
                break;
            case HAL_HW_TYPE_USB:
                stats->usb_devices++;
                break;
            default:
                stats->other_devices++;
                break;
        }
        
        // Count compatibility levels
        switch (comp->compatibility_level) {
            case HAL_COMPAT_NATIVE:
                stats->native_supported++;
                break;
            case HAL_COMPAT_FULL:
                stats->fully_supported++;
                break;
            case HAL_COMPAT_PARTIAL:
                stats->partially_supported++;
                break;
            case HAL_COMPAT_LIMITED:
                stats->limited_support++;
                break;
            case HAL_COMPAT_UNSUPPORTED:
                stats->unsupported++;
                break;
            default:
                stats->unknown++;
                break;
        }
    }
    
    stats->total_devices = hw_detection.detected_hardware.component_count;
    stats->detection_time_ms = hw_detection.detected_hardware.detection_duration / 1000; // Convert to ms
    
    return HAL_SUCCESS;
}

/**
 * Hardware Detection Implementation Functions
 */

static int detect_cpu_hardware(void)
{
    hal_cpu_features_t features;
    int result = hal->cpu_get_features(&features);
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    // Create CPU component entry
    hal_hardware_component_t* cpu_comp = add_hardware_component();
    if (!cpu_comp) {
        return -ENOMEM;
    }
    
    cpu_comp->type = HAL_HW_TYPE_CPU;
    strcpy(cpu_comp->name, "CPU");
    strcpy(cpu_comp->description, features.arch_name);
    
    // Get CPU identification (platform-specific)
    hal_arch_t arch = hal_get_architecture();
    if (arch == HAL_ARCH_X86_64) {
        detect_cpu_x86_64(cpu_comp);
    } else if (arch == HAL_ARCH_ARM64) {
        detect_cpu_arm64(cpu_comp);
    }
    
    // Set capabilities
    cpu_comp->capabilities = 0;
    if (features.has_fpu) cpu_comp->capabilities |= HAL_HW_CAP_FPU;
    if (features.has_simd) cpu_comp->capabilities |= HAL_HW_CAP_SIMD;
    if (features.has_virtualization) cpu_comp->capabilities |= HAL_HW_CAP_VIRTUALIZATION;
    if (features.has_crypto) cpu_comp->capabilities |= HAL_HW_CAP_CRYPTO;
    
    // Check compatibility
    cpu_comp->compatibility_level = hal_hardware_check_compatibility(
        cpu_comp->vendor_id, cpu_comp->device_id, HAL_HW_TYPE_CPU);
    
    if (cpu_comp->compatibility_level == HAL_COMPAT_UNKNOWN) {
        // Default to native support for known architectures
        cpu_comp->compatibility_level = HAL_COMPAT_NATIVE;
    }
    
    return HAL_SUCCESS;
}

static int detect_memory_hardware(void)
{
    // Detect memory modules and controllers
    hal_hardware_component_t* mem_comp = add_hardware_component();
    if (!mem_comp) {
        return -ENOMEM;
    }
    
    mem_comp->type = HAL_HW_TYPE_MEMORY;
    strcpy(mem_comp->name, "System Memory");
    strcpy(mem_comp->description, "System RAM");
    
    // Get memory information from PMM
    extern uint64_t pmm_get_total_memory(void);
    extern uint64_t pmm_get_available_memory(void);
    
    mem_comp->resource_info.memory_size = pmm_get_total_memory();
    mem_comp->capabilities = HAL_HW_CAP_DMA; // Memory supports DMA
    mem_comp->compatibility_level = HAL_COMPAT_NATIVE;
    
    return HAL_SUCCESS;
}

static int detect_storage_hardware(void)
{
    // Detect storage devices through device manager
    hal_device_t* devices[32];
    size_t device_count = 32;
    
    // Find storage controllers
    int result = hal_device_find_by_class(0x01, devices, &device_count); // Storage class
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    for (size_t i = 0; i < device_count; i++) {
        hal_hardware_component_t* storage_comp = add_hardware_component();
        if (!storage_comp) {
            continue;
        }
        
        storage_comp->type = HAL_HW_TYPE_STORAGE;
        strcpy(storage_comp->name, devices[i]->name);
        strcpy(storage_comp->description, devices[i]->class_name);
        
        storage_comp->vendor_id = devices[i]->vendor_id;
        storage_comp->device_id = devices[i]->device_id;
        storage_comp->bus_type = devices[i]->bus_type;
        storage_comp->resource_info.base_address = devices[i]->base_addr;
        storage_comp->resource_info.memory_size = devices[i]->mem_size;
        storage_comp->resource_info.irq = devices[i]->irq;
        
        // Check for specific storage capabilities
        if (devices[i]->subclass_id == 0x08) { // NVMe
            storage_comp->capabilities |= HAL_HW_CAP_NVME;
        }
        if (devices[i]->subclass_id == 0x06) { // SATA
            storage_comp->capabilities |= HAL_HW_CAP_SATA;
        }
        
        storage_comp->compatibility_level = hal_hardware_check_compatibility(
            storage_comp->vendor_id, storage_comp->device_id, HAL_HW_TYPE_STORAGE);
    }
    
    return HAL_SUCCESS;
}

static int detect_network_hardware(void)
{
    // Detect network devices
    hal_device_t* devices[16];
    size_t device_count = 16;
    
    int result = hal_device_find_by_class(0x02, devices, &device_count); // Network class
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    for (size_t i = 0; i < device_count; i++) {
        hal_hardware_component_t* net_comp = add_hardware_component();
        if (!net_comp) {
            continue;
        }
        
        net_comp->type = HAL_HW_TYPE_NETWORK;
        strcpy(net_comp->name, devices[i]->name);
        strcpy(net_comp->description, "Network Controller");
        
        net_comp->vendor_id = devices[i]->vendor_id;
        net_comp->device_id = devices[i]->device_id;
        net_comp->bus_type = devices[i]->bus_type;
        net_comp->capabilities = HAL_HW_CAP_NETWORK;
        
        net_comp->compatibility_level = hal_hardware_check_compatibility(
            net_comp->vendor_id, net_comp->device_id, HAL_HW_TYPE_NETWORK);
    }
    
    return HAL_SUCCESS;
}

static int detect_graphics_hardware(void)
{
    // Detect graphics devices
    hal_device_t* devices[8];
    size_t device_count = 8;
    
    int result = hal_device_find_by_class(0x03, devices, &device_count); // Display class
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    for (size_t i = 0; i < device_count; i++) {
        hal_hardware_component_t* gpu_comp = add_hardware_component();
        if (!gpu_comp) {
            continue;
        }
        
        gpu_comp->type = HAL_HW_TYPE_GRAPHICS;
        strcpy(gpu_comp->name, devices[i]->name);
        strcpy(gpu_comp->description, "Graphics Controller");
        
        gpu_comp->vendor_id = devices[i]->vendor_id;
        gpu_comp->device_id = devices[i]->device_id;
        gpu_comp->bus_type = devices[i]->bus_type;
        gpu_comp->capabilities = HAL_HW_CAP_GRAPHICS | HAL_HW_CAP_DMA;
        
        // Check for specific GPU features
        if (gpu_comp->vendor_id == 0x10DE) { // NVIDIA
            gpu_comp->capabilities |= HAL_HW_CAP_GPU_COMPUTE;
        } else if (gpu_comp->vendor_id == 0x1002) { // AMD
            gpu_comp->capabilities |= HAL_HW_CAP_GPU_COMPUTE;
        }
        
        gpu_comp->compatibility_level = hal_hardware_check_compatibility(
            gpu_comp->vendor_id, gpu_comp->device_id, HAL_HW_TYPE_GRAPHICS);
    }
    
    return HAL_SUCCESS;
}

static int detect_audio_hardware(void)
{
    // Detect audio devices
    hal_device_t* devices[8];
    size_t device_count = 8;
    
    int result = hal_device_find_by_class(0x04, devices, &device_count); // Multimedia class
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    for (size_t i = 0; i < device_count; i++) {
        hal_hardware_component_t* audio_comp = add_hardware_component();
        if (!audio_comp) {
            continue;
        }
        
        audio_comp->type = HAL_HW_TYPE_AUDIO;
        strcpy(audio_comp->name, devices[i]->name);
        strcpy(audio_comp->description, "Audio Controller");
        
        audio_comp->vendor_id = devices[i]->vendor_id;
        audio_comp->device_id = devices[i]->device_id;
        audio_comp->bus_type = devices[i]->bus_type;
        audio_comp->capabilities = HAL_HW_CAP_AUDIO;
        
        audio_comp->compatibility_level = hal_hardware_check_compatibility(
            audio_comp->vendor_id, audio_comp->device_id, HAL_HW_TYPE_AUDIO);
    }
    
    return HAL_SUCCESS;
}

static int detect_usb_hardware(void)
{
    // Detect USB controllers
    hal_device_t* devices[16];
    size_t device_count = 16;
    
    int result = hal_device_find_by_class(0x0C, devices, &device_count); // Serial bus class
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    for (size_t i = 0; i < device_count; i++) {
        if (devices[i]->subclass_id != 0x03) { // Not USB
            continue;
        }
        
        hal_hardware_component_t* usb_comp = add_hardware_component();
        if (!usb_comp) {
            continue;
        }
        
        usb_comp->type = HAL_HW_TYPE_USB;
        strcpy(usb_comp->name, devices[i]->name);
        strcpy(usb_comp->description, "USB Controller");
        
        usb_comp->vendor_id = devices[i]->vendor_id;
        usb_comp->device_id = devices[i]->device_id;
        usb_comp->bus_type = devices[i]->bus_type;
        usb_comp->capabilities = HAL_HW_CAP_USB;
        
        // Determine USB version from programming interface
        uint8_t prog_if = devices[i]->revision; // Simplified
        if (prog_if == 0x30) {
            usb_comp->capabilities |= HAL_HW_CAP_USB3;
        } else if (prog_if == 0x20) {
            usb_comp->capabilities |= HAL_HW_CAP_USB2;
        }
        
        usb_comp->compatibility_level = hal_hardware_check_compatibility(
            usb_comp->vendor_id, usb_comp->device_id, HAL_HW_TYPE_USB);
    }
    
    return HAL_SUCCESS;
}

static int detect_platform_hardware(void)
{
    // Detect platform-specific hardware
    hal_arch_t arch = hal_get_architecture();
    
    if (arch == HAL_ARCH_X86_64) {
        return detect_platform_x86_64();
    } else if (arch == HAL_ARCH_ARM64) {
        return detect_platform_arm64();
    }
    
    return HAL_SUCCESS;
}

static int build_compatibility_report(void)
{
    hal_compatibility_report_t* report = &hw_detection.compatibility_report;
    memset(report, 0, sizeof(hal_compatibility_report_t));
    
    report->total_components = hw_detection.detected_hardware.component_count;
    
    // Count compatibility levels
    for (size_t i = 0; i < hw_detection.detected_hardware.component_count; i++) {
        hal_hardware_component_t* comp = &hw_detection.detected_hardware.components[i];
        
        switch (comp->compatibility_level) {
            case HAL_COMPAT_NATIVE:
                report->native_supported++;
                break;
            case HAL_COMPAT_FULL:
                report->fully_supported++;
                break;
            case HAL_COMPAT_PARTIAL:
                report->partially_supported++;
                break;
            case HAL_COMPAT_LIMITED:
                report->limited_support++;
                break;
            case HAL_COMPAT_UNSUPPORTED:
                report->unsupported++;
                break;
            default:
                report->unknown++;
                break;
        }
    }
    
    // Calculate compatibility score (0-100)
    if (report->total_components > 0) {
        float score = (report->native_supported * 100.0f + 
                      report->fully_supported * 90.0f +
                      report->partially_supported * 70.0f +
                      report->limited_support * 50.0f) / report->total_components;
        report->compatibility_score = (uint32_t)score;
    }
    
    // Determine overall compatibility level
    if (report->compatibility_score >= 95) {
        report->overall_compatibility = HAL_COMPAT_NATIVE;
    } else if (report->compatibility_score >= 80) {
        report->overall_compatibility = HAL_COMPAT_FULL;
    } else if (report->compatibility_score >= 60) {
        report->overall_compatibility = HAL_COMPAT_PARTIAL;
    } else if (report->compatibility_score >= 40) {
        report->overall_compatibility = HAL_COMPAT_LIMITED;
    } else {
        report->overall_compatibility = HAL_COMPAT_UNSUPPORTED;
    }
    
    return HAL_SUCCESS;
}

static int apply_hardware_quirks(void)
{
    // Apply hardware-specific workarounds and optimizations
    for (size_t i = 0; i < hw_detection.detected_hardware.component_count; i++) {
        hal_hardware_component_t* comp = &hw_detection.detected_hardware.components[i];
        
        // Find applicable quirks
        for (size_t j = 0; j < hw_detection.quirk_count; j++) {
            hal_quirk_entry_t* quirk = &hw_detection.quirks_db[j];
            
            if ((quirk->vendor_id == comp->vendor_id || quirk->vendor_id == HAL_VENDOR_ID_ANY) &&
                (quirk->device_id == comp->device_id || quirk->device_id == HAL_DEVICE_ID_ANY) &&
                (quirk->hardware_type == comp->type || quirk->hardware_type == HAL_HW_TYPE_ANY)) {
                
                // Apply quirk
                if (quirk->quirk_function) {
                    quirk->quirk_function(comp, quirk->quirk_data);
                }
                
                // Mark quirk as applied
                comp->quirks_applied |= (1ULL << j);
            }
        }
    }
    
    return HAL_SUCCESS;
}

static void notify_hardware_detection(hal_hardware_component_t* component)
{
    for (size_t i = 0; i < hw_detection.detection_callback_count; i++) {
        hw_detection.detection_callbacks[i](component);
    }
}

/**
 * Helper Functions
 */

hal_hardware_component_t* add_hardware_component(void)
{
    if (hw_detection.detected_hardware.component_count >= HAL_MAX_HARDWARE_COMPONENTS) {
        return NULL;
    }
    
    hal_hardware_component_t* comp = 
        &hw_detection.detected_hardware.components[hw_detection.detected_hardware.component_count++];
    
    memset(comp, 0, sizeof(hal_hardware_component_t));
    return comp;
}

uint32_t calculate_profile_match_score(hal_hardware_profile_t* profile)
{
    uint32_t score = 0;
    
    // Compare detected hardware against profile requirements
    for (size_t i = 0; i < hw_detection.detected_hardware.component_count; i++) {
        hal_hardware_component_t* comp = &hw_detection.detected_hardware.components[i];
        
        // Check if component matches profile requirements
        for (size_t j = 0; j < profile->requirement_count; j++) {
            hal_hardware_requirement_t* req = &profile->requirements[j];
            
            if (req->hardware_type == comp->type &&
                (req->vendor_id == comp->vendor_id || req->vendor_id == HAL_VENDOR_ID_ANY) &&
                (req->device_id == comp->device_id || req->device_id == HAL_DEVICE_ID_ANY)) {
                
                // Award points based on compatibility level
                switch (comp->compatibility_level) {
                    case HAL_COMPAT_NATIVE:
                        score += 100;
                        break;
                    case HAL_COMPAT_FULL:
                        score += 90;
                        break;
                    case HAL_COMPAT_PARTIAL:
                        score += 70;
                        break;
                    case HAL_COMPAT_LIMITED:
                        score += 50;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    return score;
}

// Platform-specific detection functions (stubs)
int detect_cpu_x86_64(hal_hardware_component_t* comp) { (void)comp; return HAL_SUCCESS; }
int detect_cpu_arm64(hal_hardware_component_t* comp) { (void)comp; return HAL_SUCCESS; }
int detect_platform_x86_64(void) { return HAL_SUCCESS; }
int detect_platform_arm64(void) { return HAL_SUCCESS; }

// Database initialization functions (implemented elsewhere)
void init_builtin_hardware_database(void) { }
void init_compatibility_database(void) { }
void init_quirks_database(void) { }