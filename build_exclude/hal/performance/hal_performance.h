/**
 * @file hal_performance.h
 * @brief HAL Performance Optimization Framework Header
 * 
 * This header defines structures and functions for comprehensive performance
 * optimization including CPU features, NUMA topology, and power management.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#ifndef HAL_PERFORMANCE_H
#define HAL_PERFORMANCE_H

#include "../../include/hal_interface.h"
#include "../../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum limits
#define HAL_MAX_CPU_CORES               256
#define HAL_MAX_NUMA_NODES              64
#define HAL_MAX_PERFORMANCE_PROFILES    16
#define HAL_MAX_PERFORMANCE_MONITORS    32
#define HAL_MAX_PERFORMANCE_CALLBACKS   16
#define HAL_MAX_PERFORMANCE_COUNTERS    16

// CPU governors
typedef enum {
    HAL_CPU_GOVERNOR_PERFORMANCE,   // Always maximum frequency
    HAL_CPU_GOVERNOR_POWERSAVE,     // Always minimum frequency
    HAL_CPU_GOVERNOR_ONDEMAND,      // Dynamic scaling based on load
    HAL_CPU_GOVERNOR_CONSERVATIVE,  // Gradual frequency changes
    HAL_CPU_GOVERNOR_USERSPACE,     // User-controlled frequency
    HAL_CPU_GOVERNOR_SCHEDUTIL      // Scheduler-driven scaling
} hal_cpu_governor_t;

// Memory policies
typedef enum {
    HAL_MEMORY_POLICY_PERFORMANCE,  // Optimize for speed
    HAL_MEMORY_POLICY_BALANCED,     // Balance speed and efficiency
    HAL_MEMORY_POLICY_POWER_SAVE,   // Optimize for power efficiency
    HAL_MEMORY_POLICY_INTERLEAVE,   // Interleave across NUMA nodes
    HAL_MEMORY_POLICY_BIND,         // Bind to specific NUMA node
    HAL_MEMORY_POLICY_PREFERRED     // Prefer specific NUMA node
} hal_memory_policy_t;

// I/O schedulers
typedef enum {
    HAL_IO_SCHEDULER_NOOP,          // No-operation (FIFO)
    HAL_IO_SCHEDULER_DEADLINE,      // Deadline scheduler
    HAL_IO_SCHEDULER_CFQ,           // Completely Fair Queuing
    HAL_IO_SCHEDULER_BFQ,           // Budget Fair Queuing
    HAL_IO_SCHEDULER_MQ_DEADLINE,   // Multi-queue deadline
    HAL_IO_SCHEDULER_KYBER          // Kyber scheduler
} hal_io_scheduler_t;

// Workload types
typedef enum {
    HAL_WORKLOAD_COMPUTE_INTENSIVE, // CPU-bound workloads
    HAL_WORKLOAD_MEMORY_INTENSIVE,  // Memory bandwidth bound
    HAL_WORKLOAD_IO_INTENSIVE,      // I/O bound workloads
    HAL_WORKLOAD_NETWORK_INTENSIVE, // Network bound workloads
    HAL_WORKLOAD_INTERACTIVE,       // Low latency required
    HAL_WORKLOAD_BATCH,             // High throughput batch jobs
    HAL_WORKLOAD_BALANCED           // Mixed workload
} hal_workload_type_t;

// Performance counter types
typedef enum {
    HAL_PERF_COUNTER_CYCLES,        // CPU cycles
    HAL_PERF_COUNTER_INSTRUCTIONS,  // Instructions executed
    HAL_PERF_COUNTER_CACHE_MISSES,  // Cache misses
    HAL_PERF_COUNTER_BRANCH_MISSES, // Branch mispredictions
    HAL_PERF_COUNTER_TLB_MISSES,    // TLB misses
    HAL_PERF_COUNTER_PAGE_FAULTS,   // Page faults
    HAL_PERF_COUNTER_CONTEXT_SWITCHES, // Context switches
    HAL_PERF_COUNTER_INTERRUPTS,    // Hardware interrupts
    HAL_PERF_COUNTER_MEMORY_READS,  // Memory read operations
    HAL_PERF_COUNTER_MEMORY_WRITES, // Memory write operations
    HAL_PERF_COUNTER_CUSTOM         // Platform-specific counters
} hal_perf_counter_type_t;

// CPU core information
typedef struct {
    uint32_t core_id;
    uint32_t package_id;
    uint32_t thread_id;
    
    // Frequency information
    uint32_t base_frequency;    // Base frequency in kHz
    uint32_t max_frequency;     // Maximum frequency in kHz  
    uint32_t min_frequency;     // Minimum frequency in kHz
    uint32_t current_frequency; // Current frequency in kHz
    
    // Cache information
    uint32_t cache_levels;
    uint32_t l1_cache_size;     // L1 cache size in bytes
    uint32_t l2_cache_size;     // L2 cache size in bytes
    uint32_t l3_cache_size;     // L3 cache size in bytes
    uint32_t cache_line_size;   // Cache line size in bytes
    
    // Thermal information
    uint32_t temperature;       // Temperature in degrees Celsius
    uint32_t thermal_limit;     // Thermal throttling limit
    
    // Power information
    uint32_t power_consumption; // Current power in mW
    uint32_t max_power;         // Maximum power in mW
    
    // Performance state
    uint32_t p_state;           // Current P-state
    uint32_t c_state;           // Current C-state
    
    // Utilization
    float utilization_percent;  // CPU utilization percentage
    uint64_t idle_time;         // Idle time in ticks
    uint64_t busy_time;         // Busy time in ticks
} hal_cpu_core_t;

// CPU topology information
typedef struct {
    uint32_t total_cpus;
    uint32_t physical_packages;
    uint32_t cores_per_package;
    uint32_t threads_per_core;
    
    hal_cpu_core_t cores[HAL_MAX_CPU_CORES];
    
    // Shared resources
    struct {
        uint32_t shared_l3_mask;    // Bitmask of cores sharing L3
        uint32_t shared_memory_controller; // Memory controller sharing
    } sharing_info;
} hal_cpu_topology_t;

// NUMA node information
typedef struct {
    uint32_t node_id;
    uint64_t total_memory;      // Total memory in bytes
    uint64_t free_memory;       // Free memory in bytes
    uint64_t used_memory;       // Used memory in bytes
    
    uint32_t cpu_mask;          // Bitmask of CPUs in this node
    uint32_t cpu_count;         // Number of CPUs
    
    // Distance to other nodes
    uint32_t distance[HAL_MAX_NUMA_NODES];
    
    // Memory bandwidth
    uint64_t memory_bandwidth;  // Memory bandwidth in MB/s
    uint64_t memory_latency_ns; // Memory access latency in ns
    
    // Performance counters
    uint64_t local_accesses;    // Local memory accesses
    uint64_t remote_accesses;   // Remote memory accesses
} hal_numa_node_t;

// NUMA topology information
typedef struct {
    bool available;
    uint32_t node_count;
    hal_numa_node_t nodes[HAL_MAX_NUMA_NODES];
    
    // Global NUMA statistics
    uint64_t total_memory;
    uint64_t migration_count;   // Page migrations between nodes
    float average_distance;     // Average inter-node distance
} hal_numa_topology_t;

// Performance counter
typedef struct {
    hal_perf_counter_type_t type;
    char name[32];
    uint32_t hw_counter_id;     // Hardware counter ID
    uint64_t start_value;
    uint64_t end_value;
    uint64_t delta;
    bool overflow;
} hal_performance_counter_t;

// Performance monitor
typedef struct {
    char name[64];
    bool active;
    uint64_t start_time;
    uint64_t end_time;
    uint64_t duration;
    
    // Counters
    hal_performance_counter_t counters[HAL_MAX_PERFORMANCE_COUNTERS];
    size_t counter_count;
    
    // Derived metrics
    float instructions_per_cycle;
    float cache_miss_rate;
    float branch_miss_rate;
    float memory_bandwidth_mbps;
    
    // Callback for results
    void (*result_callback)(struct hal_performance_monitor* monitor);
} hal_performance_monitor_t;

// Performance profile
typedef struct {
    char name[32];
    char description[128];
    
    // CPU settings
    hal_cpu_governor_t cpu_governor;
    uint32_t min_cpu_freq_percent;  // Minimum CPU frequency as percentage
    uint32_t max_cpu_freq_percent;  // Maximum CPU frequency as percentage
    bool enable_turbo;              // Enable turbo boost
    
    // Memory settings
    hal_memory_policy_t memory_policy;
    uint32_t preferred_numa_node;   // Preferred NUMA node (-1 for no preference)
    
    // I/O settings
    hal_io_scheduler_t io_scheduler;
    uint32_t io_queue_depth;        // I/O queue depth
    
    // Power settings
    uint32_t power_limit_mw;        // Power limit in milliwatts
    bool enable_c_states;           // Enable CPU C-states
    bool enable_p_states;           // Enable CPU P-states
    
    // Latency requirements
    uint32_t target_latency_us;     // Target latency in microseconds
    uint32_t target_throughput_percent; // Target throughput percentage
} hal_performance_profile_t;

// Power management information
typedef struct {
    bool available;
    uint32_t states_supported;      // Number of P-states supported
    uint32_t current_state;         // Current P-state
    uint32_t default_frequency;     // Default frequency in kHz
    uint32_t current_power_mw;      // Current power consumption in mW
    uint32_t max_power_mw;          // Maximum power consumption in mW
    
    // Thermal information
    uint32_t thermal_zones;         // Number of thermal zones
    uint32_t max_temperature;       // Maximum safe temperature
    bool thermal_throttling;        // Thermal throttling active
} hal_power_management_t;

// Performance counters information
typedef struct {
    bool available;
    uint32_t num_counters;          // Number of hardware counters
    uint32_t counter_width;         // Counter width in bits
    bool supports_overflow_interrupt; // Overflow interrupt support
    
    // Supported counter types
    uint32_t supported_types;       // Bitmask of supported counter types
} hal_performance_counters_t;

// Performance configuration
typedef struct {
    bool enable_cpu_scaling;        // Enable CPU frequency scaling
    bool enable_numa_balancing;     // Enable NUMA balancing
    bool enable_power_management;   // Enable power management
    bool enable_performance_monitoring; // Enable performance monitoring
    
    uint32_t target_latency_us;     // Target system latency
    uint32_t target_throughput_percent; // Target system throughput
    
    // Thresholds
    uint32_t cpu_high_threshold;    // CPU high utilization threshold
    uint32_t cpu_low_threshold;     // CPU low utilization threshold
    uint32_t memory_pressure_threshold; // Memory pressure threshold
    uint32_t thermal_warning_threshold; // Thermal warning threshold
} hal_performance_config_t;

// Performance statistics
typedef struct {
    // CPU statistics
    float cpu_usage[HAL_MAX_CPU_CORES];     // Per-CPU utilization
    float average_cpu_usage;                // Average CPU utilization
    uint32_t cpu_frequency[HAL_MAX_CPU_CORES]; // Per-CPU frequency
    
    // Memory statistics
    float memory_usage[HAL_MAX_NUMA_NODES]; // Per-node memory usage
    uint64_t total_memory;                  // Total system memory
    uint64_t free_memory;                   // Free system memory
    uint64_t cache_memory;                  // Cache memory
    uint64_t buffer_memory;                 // Buffer memory
    
    // Power and thermal
    uint32_t power_consumption_mw;          // Current power consumption
    uint32_t temperature_celsius;           // Current temperature
    
    // Performance counters
    uint64_t instructions_per_second;       // Instructions per second
    uint64_t cache_misses_per_second;       // Cache misses per second
    uint64_t page_faults_per_second;        // Page faults per second
    uint64_t context_switches_per_second;   // Context switches per second
    
    // I/O statistics
    uint64_t disk_reads_per_second;         // Disk reads per second
    uint64_t disk_writes_per_second;        // Disk writes per second
    uint64_t network_packets_per_second;    // Network packets per second
    
    // Latency measurements
    uint32_t average_latency_us;            // Average system latency
    uint32_t max_latency_us;                // Maximum observed latency
    uint32_t interrupt_latency_us;          // Interrupt handling latency
} hal_performance_stats_t;

// Callback function types
typedef void (*hal_performance_callback_t)(hal_performance_stats_t* stats);
typedef void (*hal_thermal_callback_t)(uint32_t temperature, bool throttling);
typedef void (*hal_power_callback_t)(uint32_t power_mw, hal_power_state_t state);

// Function prototypes

// Initialization and shutdown
int hal_performance_init(void);
void hal_performance_shutdown(void);

// Configuration
int hal_performance_set_config(const hal_performance_config_t* config);
int hal_performance_get_config(hal_performance_config_t* config);

// Topology information
int hal_performance_get_cpu_topology(hal_cpu_topology_t* topology);
int hal_performance_get_numa_topology(hal_numa_topology_t* topology);

// CPU frequency management
int hal_performance_set_cpu_frequency(uint32_t cpu_id, uint32_t frequency_khz);
uint32_t hal_performance_get_cpu_frequency(uint32_t cpu_id);
int hal_performance_set_cpu_governor(uint32_t cpu_id, hal_cpu_governor_t governor);
hal_cpu_governor_t hal_performance_get_cpu_governor(uint32_t cpu_id);

// Performance profiles
int hal_performance_set_profile(const char* profile_name);
int hal_performance_get_profile(char* profile_name, size_t name_size);
int hal_performance_create_profile(const hal_performance_profile_t* profile);
int hal_performance_delete_profile(const char* profile_name);
int hal_performance_list_profiles(char profiles[][32], size_t* count);

// Performance monitoring
int hal_performance_start_monitor(hal_performance_monitor_t* monitor);
int hal_performance_stop_monitor(hal_performance_monitor_t* monitor);
int hal_performance_create_monitor(const char* name, hal_performance_monitor_t** monitor);
int hal_performance_destroy_monitor(hal_performance_monitor_t* monitor);

// Performance optimization
int hal_performance_optimize_for_workload(hal_workload_type_t workload);
int hal_performance_auto_tune(void);
int hal_performance_reset_optimizations(void);

// Statistics and reporting
int hal_performance_get_stats(hal_performance_stats_t* stats);
int hal_performance_reset_stats(void);
int hal_performance_export_stats(const char* filename);

// NUMA management
int hal_performance_bind_to_numa_node(uint32_t node_id);
int hal_performance_get_numa_node_for_cpu(uint32_t cpu_id);
int hal_performance_allocate_numa_memory(uint32_t node_id, size_t size, void** ptr);
int hal_performance_migrate_pages(void* addr, size_t size, uint32_t from_node, uint32_t to_node);

// Power management
int hal_performance_set_power_limit(uint32_t limit_mw);
uint32_t hal_performance_get_power_consumption(void);
int hal_performance_enable_power_saving(bool enable);
int hal_performance_set_thermal_limit(uint32_t temperature_celsius);

// Callback registration
int hal_performance_register_callback(hal_performance_callback_t callback);
int hal_performance_unregister_callback(hal_performance_callback_t callback);
int hal_performance_register_thermal_callback(hal_thermal_callback_t callback);
int hal_performance_register_power_callback(hal_power_callback_t callback);

// Advanced features
int hal_performance_enable_branch_prediction_optimization(bool enable);
int hal_performance_enable_cache_prefetching(bool enable);
int hal_performance_set_memory_interleaving(bool enable);
int hal_performance_enable_cpu_hotplug(bool enable);

// Debugging and diagnostics
void hal_performance_dump_topology(void);
void hal_performance_dump_counters(void);
void hal_performance_dump_profiles(void);
int hal_performance_run_benchmark(const char* benchmark_name, hal_performance_stats_t* results);

// Internal functions (exposed for platform-specific implementations)
void init_default_profiles(void);
int detect_numa_x86_64(void);
int detect_numa_arm64(void);
int setup_perf_counters_x86_64(void);
int setup_perf_counters_arm64(void);
int apply_cpu_optimizations_x86_64(void);
int apply_cpu_optimizations_arm64(void);
void enable_numa_balancing(void);
void optimize_memory_allocation(void);
void enable_memory_interleaving(void);
uint64_t read_performance_counter(uint32_t counter_id);
void calculate_performance_metrics(hal_performance_monitor_t* monitor);
int apply_performance_profile(hal_performance_profile_t* profile);
uint32_t get_cpu_temperature(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_PERFORMANCE_H