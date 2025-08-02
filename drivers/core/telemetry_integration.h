#ifndef TELEMETRY_INTEGRATION_H
#define TELEMETRY_INTEGRATION_H

/**
 * @file telemetry_integration.h
 * @brief Driver Telemetry Integration Helpers for RaeenOS
 * 
 * This header provides easy-to-use macros and functions for integrating
 * telemetry into existing driver subsystems without major code changes.
 * It provides automatic metric registration, performance tracking, and
 * health monitoring integration.
 * 
 * Author: RaeenOS Driver Telemetry Team
 * License: MIT
 * Version: 2.0.0
 */

#include "driver_telemetry.h"

#ifdef __cplusplus
extern "C" {
#endif

// Subsystem-specific metric IDs (automatically registered)
typedef struct {
    // Core framework metrics
    uint32_t driver_load_time;
    uint32_t driver_unload_time;
    uint32_t crash_recovery_count;
    uint32_t sandbox_violations;
    
    // PCIe subsystem metrics
    uint32_t pcie_device_count;
    uint32_t pcie_enumeration_time;
    uint32_t pcie_bandwidth_usage;
    uint32_t msi_interrupts_processed;
    uint32_t pcie_link_errors;
    uint32_t pcie_power_state_changes;
    
    // USB subsystem metrics
    uint32_t usb_device_count;
    uint32_t usb_enumeration_time;
    uint32_t usb_transfer_rate;
    uint32_t usb_errors;
    uint32_t thunderbolt_bandwidth;
    uint32_t usb_c_power_negotiations;
    
    // NVMe subsystem metrics
    uint32_t nvme_io_operations;
    uint32_t nvme_queue_depth;
    uint32_t nvme_response_time;
    uint32_t nvme_bandwidth;
    uint32_t nvme_errors;
    uint32_t nvme_thermal_throttling;
    
    // Input subsystem metrics
    uint32_t input_events_processed;
    uint32_t input_latency;
    uint32_t gesture_recognition_time;
    uint32_t gaming_mode_switches;
    uint32_t haptic_feedback_calls;
    
    // ACPI subsystem metrics
    uint32_t acpi_frequency_changes;
    uint32_t acpi_power_transitions;
    uint32_t thermal_events;
    uint32_t sci_interrupts;
    uint32_t cpu_temperature;
    
    // Hot-plug subsystem metrics
    uint32_t hotplug_detection_time;
    uint32_t device_arrivals;
    uint32_t device_removals;
    uint32_t driver_load_failures;
} subsystem_metrics_t;

// Global subsystem metrics instance
extern subsystem_metrics_t g_subsystem_metrics;

// Initialization function for all subsystem metrics
int telemetry_init_subsystem_metrics(void);

// Subsystem-specific telemetry macros

// === PCIe Subsystem Telemetry ===
#define PCIE_TELEMETRY_START_ENUMERATION() \\
    telemetry_timer_t* __pcie_enum_timer = telemetry_start_timer("pcie_enumeration")

#define PCIE_TELEMETRY_END_ENUMERATION() \\
    telemetry_stop_timer(__pcie_enum_timer); \\
    telemetry_update_counter(g_subsystem_metrics.pcie_device_count, 1)

#define PCIE_TELEMETRY_LOG_ERROR(error_code, msg, ...) \\
    telemetry_update_counter(g_subsystem_metrics.pcie_link_errors, 1); \\
    telemetry_log_error(SUBSYSTEM_PCI, error_code, msg, ##__VA_ARGS__)

#define PCIE_TELEMETRY_MSI_INTERRUPT() \\
    telemetry_update_counter(g_subsystem_metrics.msi_interrupts_processed, 1)

#define PCIE_TELEMETRY_BANDWIDTH_UPDATE(bytes_per_second) \\
    telemetry_update_gauge(g_subsystem_metrics.pcie_bandwidth_usage, bytes_per_second)

// === USB Subsystem Telemetry ===
#define USB_TELEMETRY_START_ENUMERATION() \\
    telemetry_timer_t* __usb_enum_timer = telemetry_start_timer("usb_enumeration")

#define USB_TELEMETRY_END_ENUMERATION() \\
    telemetry_stop_timer(__usb_enum_timer); \\
    telemetry_update_counter(g_subsystem_metrics.usb_device_count, 1)

#define USB_TELEMETRY_TRANSFER_RATE(bytes_per_second) \\
    telemetry_update_gauge(g_subsystem_metrics.usb_transfer_rate, bytes_per_second)

#define USB_TELEMETRY_ERROR() \\
    telemetry_update_counter(g_subsystem_metrics.usb_errors, 1)

#define USB_TELEMETRY_THUNDERBOLT_BANDWIDTH(bandwidth) \\
    telemetry_update_gauge(g_subsystem_metrics.thunderbolt_bandwidth, bandwidth)

#define USB_TELEMETRY_POWER_NEGOTIATION() \\
    telemetry_update_counter(g_subsystem_metrics.usb_c_power_negotiations, 1)

// === NVMe Subsystem Telemetry ===
#define NVME_TELEMETRY_IO_START() \\
    telemetry_timer_t* __nvme_io_timer = telemetry_start_timer("nvme_io_operation"); \\
    telemetry_update_counter(g_subsystem_metrics.nvme_io_operations, 1)

#define NVME_TELEMETRY_IO_END() \\
    telemetry_stop_timer(__nvme_io_timer)

#define NVME_TELEMETRY_QUEUE_DEPTH(depth) \\
    telemetry_update_gauge(g_subsystem_metrics.nvme_queue_depth, depth)

#define NVME_TELEMETRY_BANDWIDTH(bytes_per_second) \\
    telemetry_update_gauge(g_subsystem_metrics.nvme_bandwidth, bytes_per_second)

#define NVME_TELEMETRY_ERROR() \\
    telemetry_update_counter(g_subsystem_metrics.nvme_errors, 1)

#define NVME_TELEMETRY_THERMAL_THROTTLE() \\
    telemetry_update_counter(g_subsystem_metrics.nvme_thermal_throttling, 1)

// === Input Subsystem Telemetry ===
#define INPUT_TELEMETRY_EVENT_PROCESSED() \\
    telemetry_update_counter(g_subsystem_metrics.input_events_processed, 1)

#define INPUT_TELEMETRY_LATENCY(latency_ns) \\
    telemetry_record_histogram(g_subsystem_metrics.input_latency, latency_ns)

#define INPUT_TELEMETRY_GESTURE_START() \\
    telemetry_timer_t* __gesture_timer = telemetry_start_timer("gesture_recognition")

#define INPUT_TELEMETRY_GESTURE_END() \\
    telemetry_stop_timer(__gesture_timer)

#define INPUT_TELEMETRY_GAMING_MODE_SWITCH() \\
    telemetry_update_counter(g_subsystem_metrics.gaming_mode_switches, 1)

#define INPUT_TELEMETRY_HAPTIC_FEEDBACK() \\
    telemetry_update_counter(g_subsystem_metrics.haptic_feedback_calls, 1)

// === ACPI Subsystem Telemetry ===
#define ACPI_TELEMETRY_FREQUENCY_CHANGE() \\
    telemetry_update_counter(g_subsystem_metrics.acpi_frequency_changes, 1)

#define ACPI_TELEMETRY_POWER_TRANSITION() \\
    telemetry_update_counter(g_subsystem_metrics.acpi_power_transitions, 1)

#define ACPI_TELEMETRY_THERMAL_EVENT() \\
    telemetry_update_counter(g_subsystem_metrics.thermal_events, 1)

#define ACPI_TELEMETRY_SCI_INTERRUPT() \\
    telemetry_update_counter(g_subsystem_metrics.sci_interrupts, 1)

#define ACPI_TELEMETRY_CPU_TEMPERATURE(temp_decidegrees) \\
    telemetry_update_gauge(g_subsystem_metrics.cpu_temperature, temp_decidegrees)

// === Hot-plug Subsystem Telemetry ===
#define HOTPLUG_TELEMETRY_DETECTION_START() \\
    telemetry_timer_t* __hotplug_timer = telemetry_start_timer("hotplug_detection")

#define HOTPLUG_TELEMETRY_DETECTION_END() \\
    telemetry_stop_timer(__hotplug_timer)

#define HOTPLUG_TELEMETRY_DEVICE_ARRIVAL() \\
    telemetry_update_counter(g_subsystem_metrics.device_arrivals, 1)

#define HOTPLUG_TELEMETRY_DEVICE_REMOVAL() \\
    telemetry_update_counter(g_subsystem_metrics.device_removals, 1)

#define HOTPLUG_TELEMETRY_DRIVER_LOAD_FAILURE() \\
    telemetry_update_counter(g_subsystem_metrics.driver_load_failures, 1)

// === Core Framework Telemetry ===
#define DRIVER_TELEMETRY_LOAD_START() \\
    telemetry_timer_t* __driver_load_timer = telemetry_start_timer("driver_load")

#define DRIVER_TELEMETRY_LOAD_END() \\
    telemetry_stop_timer(__driver_load_timer)

#define DRIVER_TELEMETRY_CRASH_RECOVERY() \\
    telemetry_update_counter(g_subsystem_metrics.crash_recovery_count, 1)

#define DRIVER_TELEMETRY_SANDBOX_VIOLATION() \\
    telemetry_update_counter(g_subsystem_metrics.sandbox_violations, 1)

// Performance monitoring helpers
typedef struct {
    uint64_t start_time;
    uint32_t metric_id;
    const char* operation;
} performance_context_t;

#define TELEMETRY_PERF_START(operation, metric_id) \\
    performance_context_t __perf_ctx = { \\
        .start_time = telemetry_get_time_ns(), \\
        .metric_id = metric_id, \\
        .operation = operation \\
    }

#define TELEMETRY_PERF_END() \\
    do { \\
        uint64_t duration = telemetry_get_time_ns() - __perf_ctx.start_time; \\
        telemetry_update_timer(__perf_ctx.metric_id, duration); \\
        if (duration > 1000000) { /* > 1ms */ \\
            telemetry_log_event(DIAG_EVENT_PERFORMANCE, SUBSYSTEM_CORE, \\
                               "Slow operation: %s took %llu μs", \\
                               __perf_ctx.operation, duration / 1000); \\
        } \\
    } while(0)

// Health monitoring helpers
#define TELEMETRY_HEALTH_CHECK(driver_ptr) \\
    telemetry_update_driver_health(driver_ptr)

#define TELEMETRY_MEMORY_ALLOC(size) \\
    ({ \\
        void* __ptr = hal_alloc(size); \\
        if (__ptr) { \\
            telemetry_track_memory_allocation(__ptr, size, __FILE__, __LINE__); \\
        } \\
        __ptr; \\
    })

#define TELEMETRY_MEMORY_FREE(ptr) \\
    do { \\
        if (ptr) { \\
            telemetry_track_memory_free(ptr); \\
            hal_free(ptr); \\
            ptr = NULL; \\
        } \\
    } while(0)

// Error tracking helpers
#define TELEMETRY_ERROR_RATE_INCREMENT(subsystem) \\
    telemetry_update_counter(g_metric_error_rate, 1); \\
    telemetry_log_event(DIAG_EVENT_ERROR, subsystem, \\
                       "Error occurred at %s:%d", __FILE__, __LINE__)

// Bandwidth tracking helpers
#define TELEMETRY_BANDWIDTH_UPDATE(subsystem, bytes_transferred, duration_ns) \\
    do { \\
        if (duration_ns > 0) { \\
            uint64_t bandwidth = (bytes_transferred * 1000000000ULL) / duration_ns; \\
            switch (subsystem) { \\
                case SUBSYSTEM_PCI: \\
                    telemetry_update_gauge(g_subsystem_metrics.pcie_bandwidth_usage, bandwidth); \\
                    break; \\
                case SUBSYSTEM_USB: \\
                    telemetry_update_gauge(g_subsystem_metrics.usb_transfer_rate, bandwidth); \\
                    break; \\
                case SUBSYSTEM_NVME: \\
                    telemetry_update_gauge(g_subsystem_metrics.nvme_bandwidth, bandwidth); \\
                    break; \\
                default: \\
                    telemetry_update_gauge(g_metric_io_throughput, bandwidth); \\
                    break; \\
            } \\
        } \\
    } while(0)

// Diagnostic helpers
#define TELEMETRY_ASSERT(condition, subsystem, msg, ...) \\
    do { \\
        if (!(condition)) { \\
            telemetry_log_event(DIAG_EVENT_CRITICAL, subsystem, \\
                               "Assertion failed at %s:%d: " msg, \\
                               __FILE__, __LINE__, ##__VA_ARGS__); \\
        } \\
    } while(0)

#define TELEMETRY_DEBUG(subsystem, msg, ...) \\
    telemetry_log_event(DIAG_EVENT_INFO, subsystem, msg, ##__VA_ARGS__)

#define TELEMETRY_WARNING(subsystem, msg, ...) \\
    telemetry_log_event(DIAG_EVENT_WARNING, subsystem, msg, ##__VA_ARGS__)

// Function prototypes for integration helpers
int telemetry_integrate_with_driver_framework(void);
int telemetry_integrate_with_pcie_subsystem(void);
int telemetry_integrate_with_usb_subsystem(void);
int telemetry_integrate_with_nvme_subsystem(void);
int telemetry_integrate_with_input_subsystem(void);
int telemetry_integrate_with_acpi_subsystem(void);
int telemetry_integrate_with_hotplug_subsystem(void);

// Convenience function to initialize all subsystem integrations
int telemetry_init_all_integrations(void);

// Performance benchmarking utilities
typedef struct {
    const char* test_name;
    uint32_t iterations;
    uint64_t total_time;
    uint64_t min_time;
    uint64_t max_time;
    uint64_t avg_time;
} benchmark_result_t;

int telemetry_benchmark_operation(const char* operation_name, 
                                 void (*operation_func)(void*), 
                                 void* operation_data,
                                 uint32_t iterations,
                                 benchmark_result_t* result);

// System-wide telemetry dashboard data
typedef struct {
    // Overall system health
    uint32_t healthy_drivers;
    uint32_t degraded_drivers;
    uint32_t failed_drivers;
    
    // Performance summary
    uint64_t total_interrupts_per_second;
    uint64_t total_io_operations_per_second;
    uint32_t average_cpu_usage_percent;
    uint64_t total_memory_usage_kb;
    
    // Error summary
    uint32_t errors_per_minute;
    uint32_t critical_events_count;
    uint32_t anomalies_detected_today;
    
    // Top metrics
    struct {
        const char* name;
        uint64_t value;
        const char* unit;
    } top_metrics[10];
} telemetry_dashboard_t;

int telemetry_get_dashboard_data(telemetry_dashboard_t* dashboard);

#ifdef __cplusplus
}
#endif

#endif // TELEMETRY_INTEGRATION_H