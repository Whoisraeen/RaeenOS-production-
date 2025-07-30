#ifndef RAEENOS_INTERFACES_H
#define RAEENOS_INTERFACES_H

/**
 * @file raeenos_interfaces.h
 * @brief Master Interface Header for RaeenOS Subsystem Integration
 * 
 * This header brings together all RaeenOS subsystem interfaces and defines
 * the integration points between them to ensure seamless operation when
 * 42 development agents work simultaneously.
 * 
 * Version: 1.0
 * API Version: 1
 */

// Core system interfaces
#include "types.h"
#include "errno.h"
#include "hal_interface.h"
#include "driver_framework.h"
#include "memory_interface.h"
#include "process_interface.h"
#include "filesystem_interface.h"
#include "security_interface.h"
#include "ai_interface.h"
#include "system_services_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Master API version - increment when any interface changes
#define RAEENOS_API_VERSION_MAJOR  1
#define RAEENOS_API_VERSION_MINOR  0
#define RAEENOS_API_VERSION_PATCH  0

// System-wide constants
#define RAEENOS_MAX_SUBSYSTEMS     32
#define RAEENOS_SUBSYSTEM_NAME_MAX 64
#define RAEENOS_ERROR_MESSAGE_MAX  256

// RaeenOS subsystem identifiers
typedef enum {
    RAEENOS_SUBSYSTEM_HAL,              // Hardware Abstraction Layer
    RAEENOS_SUBSYSTEM_DRIVER_FRAMEWORK, // Driver Framework
    RAEENOS_SUBSYSTEM_MEMORY,           // Memory Management
    RAEENOS_SUBSYSTEM_PROCESS,          // Process & Thread Management
    RAEENOS_SUBSYSTEM_FILESYSTEM,       // File System
    RAEENOS_SUBSYSTEM_NETWORK,          // Network Stack
    RAEENOS_SUBSYSTEM_GRAPHICS,         // Graphics & Display
    RAEENOS_SUBSYSTEM_AUDIO,            // Audio Subsystem
    RAEENOS_SUBSYSTEM_SECURITY,         // Security Framework
    RAEENOS_SUBSYSTEM_VIRTUALIZATION,   // RaeenVM Hypervisor
    RAEENOS_SUBSYSTEM_AI,               // AI Integration
    RAEENOS_SUBSYSTEM_APP_FRAMEWORK,    // Application Framework
    RAEENOS_SUBSYSTEM_PACKAGE_MANAGER,  // Package Management
    RAEENOS_SUBSYSTEM_SYSTEM_SERVICES,  // System Services
    RAEENOS_SUBSYSTEM_POWER_MANAGEMENT, // Power Management
    RAEENOS_SUBSYSTEM_THERMAL,          // Thermal Management
    RAEENOS_SUBSYSTEM_COUNT             // Total number of subsystems
} raeenos_subsystem_id_t;

// Subsystem initialization states
typedef enum {
    SUBSYSTEM_STATE_UNINITIALIZED,      // Not initialized
    SUBSYSTEM_STATE_INITIALIZING,       // Currently initializing
    SUBSYSTEM_STATE_INITIALIZED,        // Successfully initialized
    SUBSYSTEM_STATE_FAILED,             // Initialization failed
    SUBSYSTEM_STATE_SHUTTING_DOWN,      // Shutting down
    SUBSYSTEM_STATE_SHUTDOWN            // Completely shutdown
} subsystem_state_t;

// Subsystem initialization order - critical for proper startup
typedef enum {
    INIT_ORDER_EARLY    = 0,    // HAL, Memory
    INIT_ORDER_CORE     = 100,  // Process, Security, Driver Framework
    INIT_ORDER_SERVICES = 200,  // File System, Network, System Services
    INIT_ORDER_ADVANCED = 300,  // Graphics, Audio, AI
    INIT_ORDER_HIGH     = 400,  // App Framework, Package Manager
    INIT_ORDER_LATE     = 500   // User space services
} init_order_t;

// Subsystem information structure
typedef struct subsystem_info {
    raeenos_subsystem_id_t id;          // Subsystem ID
    char name[RAEENOS_SUBSYSTEM_NAME_MAX]; // Subsystem name
    uint32_t api_version;               // API version
    subsystem_state_t state;            // Current state
    init_order_t init_order;            // Initialization order
    
    // Dependencies
    raeenos_subsystem_id_t* dependencies; // Required subsystems
    size_t dependency_count;            // Number of dependencies
    
    // Initialization functions
    int (*init)(void);                  // Initialization function
    void (*cleanup)(void);              // Cleanup function
    int (*late_init)(void);             // Late initialization
    
    // Health monitoring
    int (*health_check)(void);          // Health check function
    int (*get_stats)(void* stats);      // Get statistics
    
    // Error information
    int last_error;                     // Last error code
    char error_message[RAEENOS_ERROR_MESSAGE_MAX]; // Error message
    
    // Timing information
    uint64_t init_start_time;           // Initialization start time
    uint64_t init_duration;             // Initialization duration
    uint64_t uptime;                    // Subsystem uptime
    
    void* private_data;                 // Subsystem-specific data
} subsystem_info_t;

// System integration points structure
typedef struct integration_points {
    // Memory allocation for all subsystems
    void* (*sys_alloc)(size_t size, uint32_t flags);
    void (*sys_free)(void* ptr);
    
    // Logging for all subsystems
    void (*sys_log)(int level, const char* subsystem, const char* message);
    
    // Error reporting
    void (*sys_error)(raeenos_subsystem_id_t subsystem, int error_code, const char* message);
    
    // Configuration access
    int (*sys_get_config)(const char* key, void* value, size_t* size);
    int (*sys_set_config)(const char* key, const void* value, size_t size);
    
    // Event system for inter-subsystem communication
    int (*sys_emit_event)(const char* event_type, void* data, size_t data_size);
    int (*sys_subscribe_event)(const char* event_type, void (*handler)(void* data));
    int (*sys_unsubscribe_event)(const char* event_type, void (*handler)(void* data));
    
    // Performance monitoring
    uint64_t (*sys_get_timestamp)(void);
    void (*sys_performance_start)(const char* operation);
    void (*sys_performance_end)(const char* operation);
    
    // Resource management
    int (*sys_request_resource)(const char* resource_name, void** handle);
    int (*sys_release_resource)(void* handle);
    
    // Synchronization primitives
    void* (*sys_create_lock)(void);
    void (*sys_destroy_lock)(void* lock);
    void (*sys_acquire_lock)(void* lock);
    void (*sys_release_lock)(void* lock);
    
    // Thread management
    int (*sys_create_thread)(void (*entry)(void*), void* arg, void** thread_handle);
    int (*sys_join_thread)(void* thread_handle);
    
    // Timer services
    int (*sys_create_timer)(uint64_t interval_ms, void (*callback)(void*), void* data, void** timer_handle);
    int (*sys_destroy_timer)(void* timer_handle);
} integration_points_t;

// Global system state
typedef struct raeenos_system {
    // Subsystem registry
    subsystem_info_t subsystems[RAEENOS_SUBSYSTEM_COUNT];
    bool subsystem_registered[RAEENOS_SUBSYSTEM_COUNT];
    
    // System state
    enum {
        SYSTEM_STATE_BOOTING,           // System is booting
        SYSTEM_STATE_INITIALIZING,     // Initializing subsystems
        SYSTEM_STATE_RUNNING,           // Normal operation
        SYSTEM_STATE_SHUTTING_DOWN,    // Shutting down
        SYSTEM_STATE_CRASHED           // System crashed
    } state;
    
    // Integration points
    integration_points_t* integration;
    
    // Configuration
    struct {
        bool debug_mode;                // Debug mode enabled
        bool safe_mode;                 // Safe mode enabled
        uint32_t log_level;             // System log level
        bool performance_monitoring;    // Performance monitoring enabled
    } config;
    
    // Statistics
    struct {
        uint64_t boot_time;             // System boot time
        uint64_t uptime;                // System uptime
        uint32_t subsystems_active;     // Active subsystems
        uint32_t subsystems_failed;     // Failed subsystems
        uint64_t total_memory_usage;    // Total memory usage
        uint32_t total_processes;       // Total processes
        uint32_t total_threads;         // Total threads
    } stats;
    
    // Error handling
    struct {
        uint32_t error_count;           // Total error count
        uint32_t critical_errors;       // Critical error count
        char last_error[RAEENOS_ERROR_MESSAGE_MAX]; // Last error message
    } errors;
    
    // Synchronization
    void* system_lock;                  // System-wide lock
} raeenos_system_t;

// Global system instance
extern raeenos_system_t* raeenos_system;

// Master system operations
typedef struct raeenos_operations {
    // System initialization
    int (*init)(void);
    void (*cleanup)(void);
    int (*boot)(void);
    int (*shutdown)(int timeout_seconds);
    
    // Subsystem management
    int (*register_subsystem)(subsystem_info_t* info);
    int (*unregister_subsystem)(raeenos_subsystem_id_t id);
    int (*init_subsystem)(raeenos_subsystem_id_t id);
    int (*shutdown_subsystem)(raeenos_subsystem_id_t id);
    subsystem_info_t* (*get_subsystem_info)(raeenos_subsystem_id_t id);
    
    // Dependency management
    int (*check_dependencies)(raeenos_subsystem_id_t id);
    int (*resolve_dependencies)(raeenos_subsystem_id_t* order, size_t* count);
    
    // System health
    int (*health_check_all)(void);
    float (*get_system_health)(void);
    int (*diagnose_issues)(char* report, size_t report_size);
    
    // Performance monitoring
    int (*start_performance_monitoring)(void);
    int (*stop_performance_monitoring)(void);
    int (*get_performance_report)(void* report);
    
    // Configuration management
    int (*load_config)(const char* config_file);
    int (*save_config)(const char* config_file);
    int (*get_config_value)(const char* key, void* value, size_t* size);
    int (*set_config_value)(const char* key, const void* value, size_t size);
    
    // Event system
    int (*init_event_system)(void);
    int (*emit_system_event)(const char* event, void* data);
    int (*subscribe_system_event)(const char* event, void (*handler)(void*));
    
    // Error handling
    void (*handle_critical_error)(raeenos_subsystem_id_t subsystem, int error_code);
    void (*log_system_error)(int level, const char* format, ...);
    
    // Resource management
    int (*track_resource_usage)(void);
    int (*optimize_resource_allocation)(void);
    
    // Debug and diagnostics
    int (*enable_debug_mode)(void);
    int (*disable_debug_mode)(void);
    int (*dump_system_state)(char* buffer, size_t size);
    int (*trace_subsystem_calls)(raeenos_subsystem_id_t id, bool enable);
} raeenos_ops_t;

// Global operations
extern raeenos_ops_t* raeenos;

// API compatibility checking
typedef struct api_version {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} api_version_t;

// Function declarations

// System initialization and management
int raeenos_init(void);
void raeenos_cleanup(void);
int raeenos_boot(void);
int raeenos_shutdown(int timeout_seconds);

// Subsystem management
int raeenos_register_subsystem(subsystem_info_t* info);
int raeenos_init_all_subsystems(void);
int raeenos_shutdown_all_subsystems(void);

// API version checking
bool raeenos_check_api_compatibility(api_version_t required, api_version_t available);
api_version_t raeenos_get_api_version(void);
const char* raeenos_get_version_string(void);

// Integration helpers
integration_points_t* raeenos_get_integration_points(void);
int raeenos_register_integration_points(integration_points_t* points);

// Health and diagnostics
int raeenos_system_health_check(void);
int raeenos_get_system_stats(void* stats);
int raeenos_diagnose_system(char* report, size_t size);

// Error handling
void raeenos_handle_error(raeenos_subsystem_id_t subsystem, int error_code, const char* message);
const char* raeenos_get_error_string(int error_code);

// Utility functions
const char* raeenos_subsystem_name(raeenos_subsystem_id_t id);
raeenos_subsystem_id_t raeenos_subsystem_from_name(const char* name);
bool raeenos_subsystem_is_active(raeenos_subsystem_id_t id);

// Macros for subsystem integration

// Subsystem registration helper
#define RAEENOS_REGISTER_SUBSYSTEM(name, id, init_func, cleanup_func, order) \
    static subsystem_info_t name##_subsystem_info = { \
        .id = id, \
        .name = #name, \
        .api_version = RAEENOS_API_VERSION_MAJOR, \
        .state = SUBSYSTEM_STATE_UNINITIALIZED, \
        .init_order = order, \
        .init = init_func, \
        .cleanup = cleanup_func \
    }; \
    static void __attribute__((constructor)) register_##name##_subsystem(void) { \
        raeenos_register_subsystem(&name##_subsystem_info); \
    }

// Dependency declaration helper
#define RAEENOS_DECLARE_DEPENDENCIES(subsystem, ...) \
    static raeenos_subsystem_id_t subsystem##_deps[] = { __VA_ARGS__ }; \
    static void __attribute__((constructor)) set_##subsystem##_dependencies(void) { \
        subsystem_info_t* info = raeenos_get_subsystem_info(RAEENOS_SUBSYSTEM_##subsystem); \
        if (info) { \
            info->dependencies = subsystem##_deps; \
            info->dependency_count = sizeof(subsystem##_deps) / sizeof(raeenos_subsystem_id_t); \
        } \
    }

// Error handling macros
#define RAEENOS_ERROR(subsystem, code, msg) \
    raeenos_handle_error(RAEENOS_SUBSYSTEM_##subsystem, code, msg)

#define RAEENOS_CHECK_SUBSYSTEM(id) \
    raeenos_subsystem_is_active(RAEENOS_SUBSYSTEM_##id)

#define RAEENOS_WAIT_FOR_SUBSYSTEM(id, timeout_ms) \
    do { \
        uint64_t start = raeenos->get_timestamp(); \
        while (!raeenos_subsystem_is_active(RAEENOS_SUBSYSTEM_##id)) { \
            if (raeenos->get_timestamp() - start > (timeout_ms * 1000000)) { \
                return -ETIMEDOUT; \
            } \
            raeenos->yield(); \
        } \
    } while(0)

// API version compatibility check
#define RAEENOS_REQUIRE_API_VERSION(major, minor, patch) \
    do { \
        api_version_t required = { major, minor, patch }; \
        api_version_t current = raeenos_get_api_version(); \
        if (!raeenos_check_api_compatibility(required, current)) { \
            return -EVERSION; \
        } \
    } while(0)

// Integration point access
#define RAEENOS_ALLOC(size, flags) \
    raeenos_get_integration_points()->sys_alloc(size, flags)

#define RAEENOS_FREE(ptr) \
    raeenos_get_integration_points()->sys_free(ptr)

#define RAEENOS_LOG(level, subsystem, message) \
    raeenos_get_integration_points()->sys_log(level, subsystem, message)

// Common error codes for all subsystems
#define RAEENOS_SUCCESS             0
#define RAEENOS_ERR_GENERIC        -5000
#define RAEENOS_ERR_NOT_INITIALIZED -5001
#define RAEENOS_ERR_ALREADY_INIT   -5002
#define RAEENOS_ERR_NO_MEMORY      -5003
#define RAEENOS_ERR_INVALID_PARAM  -5004
#define RAEENOS_ERR_NOT_SUPPORTED  -5005
#define RAEENOS_ERR_TIMEOUT        -5006
#define RAEENOS_ERR_DEPENDENCY     -5007
#define RAEENOS_ERR_PERMISSION     -5008
#define RAEENOS_ERR_RESOURCE       -5009
#define RAEENOS_ERR_VERSION        -5010

// Subsystem-specific error ranges
#define HAL_ERROR_BASE             -1000
#define DRIVER_ERROR_BASE          -2000
#define MEMORY_ERROR_BASE          -2100
#define PROCESS_ERROR_BASE         -2200
#define FILESYSTEM_ERROR_BASE      -2300
#define NETWORK_ERROR_BASE         -2400
#define GRAPHICS_ERROR_BASE        -2500
#define AUDIO_ERROR_BASE           -2600
#define SECURITY_ERROR_BASE        -2700
#define VIRTUALIZATION_ERROR_BASE  -2800
#define AI_ERROR_BASE              -2900
#define APP_FRAMEWORK_ERROR_BASE   -3000
#define PACKAGE_MANAGER_ERROR_BASE -3100
#define SYSTEM_SERVICES_ERROR_BASE -4000

#ifdef __cplusplus
}
#endif

#endif // RAEENOS_INTERFACES_H