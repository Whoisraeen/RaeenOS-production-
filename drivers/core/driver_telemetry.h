#ifndef DRIVER_TELEMETRY_H
#define DRIVER_TELEMETRY_H

/**
 * @file driver_telemetry.h
 * @brief Driver Performance Monitoring, Telemetry, and Diagnostic System for RaeenOS
 * 
 * This implementation provides comprehensive monitoring including:
 * - Real-time performance metrics collection across all driver subsystems
 * - Advanced telemetry with predictive analytics and anomaly detection
 * - Comprehensive diagnostic tools with root cause analysis
 * - Driver health monitoring with automatic remediation
 * - Performance profiling with microsecond precision timing
 * - Memory usage tracking with leak detection
 * - Interrupt latency analysis and optimization suggestions
 * - Superior monitoring capabilities to Windows Performance Monitor/ETW
 * 
 * Author: RaeenOS Driver Telemetry Team
 * License: MIT
 * Version: 2.0.0
 */

#include "../kernel/include/types.h"
#include "../kernel/include/driver_framework.h"
#include "../kernel/include/hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Telemetry system version
#define DRIVER_TELEMETRY_VERSION        0x0200
#define TELEMETRY_MAX_COUNTERS          1024
#define TELEMETRY_MAX_TIMERS            256
#define TELEMETRY_MAX_EVENTS            10000
#define TELEMETRY_BUFFER_SIZE           (1024 * 1024) // 1MB telemetry buffer

// Performance metric types
typedef enum {
    METRIC_TYPE_COUNTER = 0,        // Monotonic counter
    METRIC_TYPE_GAUGE,              // Current value
    METRIC_TYPE_HISTOGRAM,          // Distribution of values
    METRIC_TYPE_TIMER,              // Timing measurements
    METRIC_TYPE_RATE,               // Rate of change
    METRIC_TYPE_MEMORY,             // Memory usage
    METRIC_TYPE_BANDWIDTH           // Data throughput
} metric_type_t;

// Driver subsystem categories
typedef enum {
    SUBSYSTEM_CORE = 0,             // Core driver framework
    SUBSYSTEM_PCI,                  // PCIe subsystem
    SUBSYSTEM_USB,                  // USB subsystem
    SUBSYSTEM_NVME,                 // NVMe storage
    SUBSYSTEM_INPUT,                // Input devices
    SUBSYSTEM_ACPI,                 // ACPI power management
    SUBSYSTEM_HOTPLUG,              // Hot-plug system
    SUBSYSTEM_NETWORK,              // Network drivers
    SUBSYSTEM_AUDIO,                // Audio drivers
    SUBSYSTEM_GPU,                  // Graphics drivers
    SUBSYSTEM_COUNT
} driver_subsystem_t;

// Performance metric structure
typedef struct {
    uint32_t id;                    // Unique metric ID
    char name[64];                  // Metric name
    char description[128];          // Metric description
    metric_type_t type;             // Metric type
    driver_subsystem_t subsystem;   // Owning subsystem
    
    // Current values
    union {
        uint64_t counter;           // Counter value
        int64_t gauge;              // Gauge value
        struct {
            uint64_t sum;           // Sum of all values
            uint64_t count;         // Number of samples
            uint64_t min;           // Minimum value
            uint64_t max;           // Maximum value
            uint64_t buckets[32];   // Histogram buckets
        } histogram;
        struct {
            uint64_t total_time;    // Total accumulated time
            uint64_t call_count;    // Number of calls
            uint64_t min_time;      // Minimum time
            uint64_t max_time;      // Maximum time
        } timer;
    } value;
    
    // Metadata
    uint64_t last_updated;          // Last update timestamp
    uint32_t update_count;          // Number of updates
    bool enabled;                   // Metric enabled
    uint32_t alert_threshold;       // Alert threshold
    
    // Performance tracking
    uint64_t samples_per_second;    // Sample rate
    uint32_t overhead_ns;           // Measurement overhead
} performance_metric_t;

// Diagnostic event types
typedef enum {
    DIAG_EVENT_INFO = 0,            // Informational
    DIAG_EVENT_WARNING,             // Warning condition
    DIAG_EVENT_ERROR,               // Error condition
    DIAG_EVENT_CRITICAL,            // Critical issue
    DIAG_EVENT_PERFORMANCE,         // Performance issue
    DIAG_EVENT_ANOMALY,             // Detected anomaly
    DIAG_EVENT_RECOVERY             // Recovery action
} diag_event_type_t;

// Diagnostic event structure
typedef struct {
    uint32_t id;                    // Event ID
    diag_event_type_t type;         // Event type
    driver_subsystem_t subsystem;   // Source subsystem
    uint64_t timestamp;             // Event timestamp
    uint32_t thread_id;             // Source thread
    uint32_t cpu_id;                // Source CPU
    
    char message[256];              // Event message
    char details[512];              // Detailed information
    
    // Context information
    uint64_t driver_address;        // Driver base address
    uint32_t error_code;            // Error code if applicable
    uint64_t related_metric_id;     // Related metric if any
    
    // Stack trace (simplified)
    struct {
        uint64_t addresses[16];     // Call stack addresses
        uint32_t depth;             // Stack depth
    } stack_trace;
} diagnostic_event_t;

// Driver health status
typedef enum {
    HEALTH_UNKNOWN = 0,             // Unknown status
    HEALTH_HEALTHY,                 // Operating normally
    HEALTH_DEGRADED,                // Performance degraded
    HEALTH_WARNING,                 // Warning conditions
    HEALTH_CRITICAL,                // Critical issues
    HEALTH_FAILED                   // Driver failed
} driver_health_t;

// Driver health information
typedef struct {
    driver_t* driver;               // Driver reference
    driver_health_t status;         // Current health status
    uint64_t last_check;            // Last health check
    
    // Performance indicators
    struct {
        uint32_t cpu_usage_percent; // CPU usage percentage
        uint64_t memory_usage_kb;   // Memory usage in KB
        uint32_t interrupt_rate;    // Interrupts per second
        uint32_t io_operations;     // I/O operations per second
        uint64_t error_rate;        // Errors per minute
    } performance;
    
    // Health metrics
    struct {
        uint32_t crash_count;       // Number of crashes
        uint32_t hang_count;        // Number of hangs
        uint32_t timeout_count;     // Number of timeouts
        uint32_t memory_leaks;      // Detected memory leaks
        uint64_t uptime;            // Driver uptime
        uint64_t last_error_time;   // Last error timestamp
    } reliability;
    
    // Recovery actions
    struct {
        uint32_t restart_count;     // Number of restarts
        uint32_t recovery_count;    // Number of recoveries
        bool auto_recovery_enabled; // Auto-recovery enabled
        uint32_t recovery_threshold; // Recovery threshold
    } recovery;
} driver_health_info_t;

// Telemetry configuration
typedef struct {
    bool enabled;                   // Telemetry enabled
    uint32_t collection_interval;   // Collection interval (ms)
    uint32_t buffer_size;           // Buffer size
    bool real_time_monitoring;      // Real-time monitoring
    bool anomaly_detection;         // Anomaly detection enabled
    bool predictive_analytics;      // Predictive analytics
    
    // Sampling configuration
    struct {
        uint32_t performance_samples; // Performance samples per second
        uint32_t memory_samples;     // Memory samples per second
        uint32_t io_samples;         // I/O samples per second
        bool adaptive_sampling;      // Adaptive sampling rate
    } sampling;
    
    // Alert configuration
    struct {
        bool email_alerts;          // Email alerts enabled
        bool system_notifications;  // System notifications
        uint32_t alert_threshold;   // Alert threshold
        uint32_t critical_threshold; // Critical threshold
    } alerts;
} telemetry_config_t;

// Telemetry manager structure
typedef struct {
    // Configuration
    telemetry_config_t config;
    bool initialized;
    
    // Metrics
    performance_metric_t* metrics;
    uint32_t metric_count;
    uint32_t next_metric_id;
    void* metrics_lock;
    
    // Events
    diagnostic_event_t* events;
    uint32_t event_capacity;
    uint32_t event_head;
    uint32_t event_tail;
    uint32_t next_event_id;
    void* events_lock;
    
    // Health monitoring
    driver_health_info_t* driver_health;
    uint32_t driver_count;
    void* health_lock;
    
    // Collection threads
    struct {
        void* performance_thread;
        void* health_thread;
        void* analytics_thread;
        bool collection_enabled;
    } threads;
    
    // Buffers
    struct {
        void* telemetry_buffer;
        uint32_t buffer_size;
        uint32_t buffer_head;
        uint32_t buffer_tail;
        void* buffer_lock;
    } buffers;
    
    // Statistics
    struct {
        uint64_t total_metrics_collected;
        uint64_t total_events_logged;
        uint64_t total_health_checks;
        uint64_t anomalies_detected;
        uint64_t predictions_made;
        uint32_t current_overhead_ns;
    } stats;
    
    // Anomaly detection
    struct {
        bool enabled;
        uint32_t detection_threshold;
        uint32_t baseline_window_size;
        uint64_t last_analysis_time;
        void* ml_model;             // Machine learning model for anomaly detection
    } anomaly_detection;
} telemetry_manager_t;

// Timer measurement structure for high-precision timing
typedef struct {
    uint64_t start_time;            // Start timestamp
    uint64_t end_time;              // End timestamp
    uint32_t metric_id;             // Associated metric ID
    const char* operation_name;     // Operation name
    bool active;                    // Timer active
} telemetry_timer_t;

// Memory tracking structure
typedef struct {
    void* address;                  // Memory address
    size_t size;                    // Allocation size
    uint64_t timestamp;             // Allocation timestamp
    const char* source_file;        // Source file
    uint32_t source_line;           // Source line
    uint32_t thread_id;             // Allocating thread
    bool freed;                     // Memory freed
} memory_allocation_t;

// Function prototypes

// Core telemetry functions
int telemetry_init(void);
int telemetry_shutdown(void);
int telemetry_configure(const telemetry_config_t* config);

// Metric management
uint32_t telemetry_register_metric(const char* name, const char* description,
                                  metric_type_t type, driver_subsystem_t subsystem);
int telemetry_update_counter(uint32_t metric_id, uint64_t value);
int telemetry_update_gauge(uint32_t metric_id, int64_t value);
int telemetry_record_histogram(uint32_t metric_id, uint64_t value);
int telemetry_update_timer(uint32_t metric_id, uint64_t duration_ns);

// High-precision timing
telemetry_timer_t* telemetry_start_timer(const char* operation_name);
int telemetry_stop_timer(telemetry_timer_t* timer);
uint64_t telemetry_get_time_ns(void);

// Event logging
int telemetry_log_event(diag_event_type_t type, driver_subsystem_t subsystem,
                       const char* message, ...);
int telemetry_log_error(driver_subsystem_t subsystem, uint32_t error_code,
                       const char* message, ...);

// Health monitoring
int telemetry_register_driver_health(driver_t* driver);
int telemetry_update_driver_health(driver_t* driver);
driver_health_t telemetry_get_driver_health(driver_t* driver);
int telemetry_check_all_driver_health(void);

// Memory tracking
int telemetry_track_memory_allocation(void* address, size_t size,
                                     const char* file, uint32_t line);
int telemetry_track_memory_free(void* address);
int telemetry_detect_memory_leaks(void);

// Performance analysis
int telemetry_analyze_performance(driver_subsystem_t subsystem);
int telemetry_detect_anomalies(void);
int telemetry_predict_performance_issues(void);

// Reporting and export
int telemetry_generate_report(char* buffer, size_t buffer_size);
int telemetry_export_metrics(const char* filename);
int telemetry_export_events(const char* filename);

// Diagnostic utilities
int telemetry_run_system_diagnostics(void);
int telemetry_benchmark_subsystem(driver_subsystem_t subsystem);
int telemetry_stress_test_driver(driver_t* driver);

// Convenience macros for easy integration
#define TELEMETRY_COUNTER(name, subsystem) \
    telemetry_register_metric(name, #name " counter", METRIC_TYPE_COUNTER, subsystem)

#define TELEMETRY_GAUGE(name, subsystem) \
    telemetry_register_metric(name, #name " gauge", METRIC_TYPE_GAUGE, subsystem)

#define TELEMETRY_TIMER(name, subsystem) \
    telemetry_register_metric(name, #name " timer", METRIC_TYPE_TIMER, subsystem)

#define TELEMETRY_INCREMENT(metric_id) \
    telemetry_update_counter(metric_id, 1)

#define TELEMETRY_ADD(metric_id, value) \
    telemetry_update_counter(metric_id, value)

#define TELEMETRY_SET(metric_id, value) \
    telemetry_update_gauge(metric_id, value)

#define TELEMETRY_TIME_START(name) \
    telemetry_timer_t* __timer_##name = telemetry_start_timer(#name)

#define TELEMETRY_TIME_END(name) \
    telemetry_stop_timer(__timer_##name)

#define TELEMETRY_LOG_INFO(subsystem, msg, ...) \
    telemetry_log_event(DIAG_EVENT_INFO, subsystem, msg, ##__VA_ARGS__)

#define TELEMETRY_LOG_WARNING(subsystem, msg, ...) \
    telemetry_log_event(DIAG_EVENT_WARNING, subsystem, msg, ##__VA_ARGS__)

#define TELEMETRY_LOG_ERROR(subsystem, msg, ...) \
    telemetry_log_event(DIAG_EVENT_ERROR, subsystem, msg, ##__VA_ARGS__)

// Memory tracking macros
#define TELEMETRY_MALLOC(size) \
    ({ \
        void* __ptr = hal_alloc(size); \
        if (__ptr) telemetry_track_memory_allocation(__ptr, size, __FILE__, __LINE__); \
        __ptr; \
    })

#define TELEMETRY_FREE(ptr) \
    do { \
        if (ptr) { \
            telemetry_track_memory_free(ptr); \
            hal_free(ptr); \
            ptr = NULL; \
        } \
    } while(0)

// Predefined metric IDs for common metrics
extern uint32_t g_metric_driver_load_time;
extern uint32_t g_metric_interrupt_latency;
extern uint32_t g_metric_memory_usage;
extern uint32_t g_metric_cpu_usage;
extern uint32_t g_metric_io_throughput;
extern uint32_t g_metric_error_rate;
extern uint32_t g_metric_device_count;

#ifdef __cplusplus
}
#endif

#endif // DRIVER_TELEMETRY_H