/**
 * @file hal_integration_tests.h
 * @brief HAL Integration Tests and Validation Suite Header
 * 
 * This header defines structures and functions for comprehensive HAL testing
 * including integration tests, validation suites, and performance benchmarks.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#ifndef HAL_INTEGRATION_TESTS_H
#define HAL_INTEGRATION_TESTS_H

#include "../../include/hal_interface.h"
#include "../../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum limits
#define HAL_MAX_TEST_SUITES         32
#define HAL_MAX_TEST_CASES          64
#define HAL_MAX_TEST_RESULTS        1024

// Test status
typedef enum {
    HAL_TEST_NOT_RUN,
    HAL_TEST_RUNNING,
    HAL_TEST_PASSED,
    HAL_TEST_FAILED,
    HAL_TEST_SKIPPED,
    HAL_TEST_TIMEOUT,
    HAL_TEST_ERROR
} hal_test_status_t;

// Test categories
typedef enum {
    HAL_TEST_CATEGORY_UNIT,         // Unit tests
    HAL_TEST_CATEGORY_FUNCTIONAL,   // Functional tests
    HAL_TEST_CATEGORY_INTEGRATION,  // Integration tests
    HAL_TEST_CATEGORY_PERFORMANCE,  // Performance tests
    HAL_TEST_CATEGORY_STRESS,       // Stress tests
    HAL_TEST_CATEGORY_REGRESSION,   // Regression tests
    HAL_TEST_CATEGORY_COMPATIBILITY,// Compatibility tests
    HAL_TEST_CATEGORY_SECURITY,     // Security tests
    HAL_TEST_CATEGORY_RELIABILITY   // Reliability tests
} hal_test_category_t;

// Test priority
typedef enum {
    HAL_TEST_PRIORITY_LOW,
    HAL_TEST_PRIORITY_NORMAL,
    HAL_TEST_PRIORITY_MEDIUM,
    HAL_TEST_PRIORITY_HIGH,
    HAL_TEST_PRIORITY_CRITICAL
} hal_test_priority_t;

// Test configuration
typedef struct {
    bool enable_stress_tests;
    bool enable_performance_tests;
    bool enable_compatibility_tests;
    bool enable_regression_tests;
    bool enable_verbose_logging;
    bool stop_on_first_failure;
    
    uint32_t test_timeout_ms;
    uint32_t stress_test_duration_ms;
    uint32_t performance_iterations;
    uint32_t memory_test_size;
    uint32_t max_concurrent_tests;
    
    char log_file[256];
    char output_format[32];     // "text", "xml", "json"
} hal_test_config_t;

// Test case context
typedef struct {
    struct hal_test_case* test_case;
    uint32_t timeout_ms;
    uint32_t iterations;
    void* user_data;
    char* message;
    size_t message_size;
} hal_test_case_context_t;

// Test case function type
typedef int (*hal_test_function_t)(hal_test_case_context_t* context);

// Test case structure
typedef struct hal_test_case {
    char name[64];
    char description[256];
    hal_test_function_t test_function;
    hal_test_category_t category;
    hal_test_priority_t priority;
    
    // Test requirements
    bool requires_hardware;
    bool requires_specific_platform;
    char required_platform[32];
    uint64_t required_capabilities;
    
    // Test configuration
    uint32_t timeout_ms;
    uint32_t iterations;
    bool skip_if_unsupported;
    
    // Dependencies
    char dependencies[8][64];
    size_t dependency_count;
    
    // Expected results
    hal_test_status_t expected_status;
    char expected_message[128];
} hal_test_case_t;

// Test suite structure
typedef struct {
    char name[64];
    char description[256];
    bool enabled;
    
    // Test cases
    hal_test_case_t test_cases[HAL_MAX_TEST_CASES];
    size_t test_count;
    
    // Suite setup and teardown
    int (*setup)(void);
    void (*teardown)(void);
    
    // Suite configuration
    hal_test_category_t category;
    hal_test_priority_t priority;
    uint32_t timeout_ms;
    
    // Suite requirements
    bool requires_root;
    bool requires_hardware;
    char required_arch[32];
    
    // Statistics
    uint32_t tests_run;
    uint32_t tests_passed;
    uint32_t tests_failed;
    uint64_t total_duration;
} hal_test_suite_t;

// Test result structure
typedef struct {
    char test_name[128];
    char suite_name[64];
    hal_test_status_t status;
    uint64_t duration;          // In timer ticks
    uint64_t timestamp;         // When test was run
    char message[512];          // Error message or details
    
    // Performance metrics (if applicable)
    struct {
        uint64_t operations_per_second;
        uint64_t memory_usage_bytes;
        uint32_t cpu_usage_percent;
        uint32_t latency_microseconds;
        uint32_t throughput_mbps;
    } performance;
    
    // Resource usage
    struct {
        uint64_t peak_memory_usage;
        uint32_t max_cpu_usage;
        uint32_t file_descriptors_used;
        uint32_t interrupts_handled;
    } resources;
} hal_test_result_t;

// Test statistics
typedef struct {
    uint32_t total_tests;
    uint32_t tests_passed;
    uint32_t tests_failed;
    uint32_t tests_skipped;
    uint32_t tests_timeout;
    uint32_t tests_error;
    
    uint64_t start_time;
    uint64_t end_time;
    uint64_t total_duration;
    
    // Performance metrics
    uint64_t memory_alloc_ops_per_sec;
    uint64_t interrupt_latency_ns;
    uint64_t context_switch_time_ns;
    uint64_t dma_throughput_mbps;
    
    // Error statistics
    uint32_t assertion_failures;
    uint32_t timeout_errors;
    uint32_t memory_errors;
    uint32_t hardware_errors;
    
    // Coverage metrics
    float code_coverage_percent;
    float feature_coverage_percent;
    float platform_coverage_percent;
} hal_test_statistics_t;

// Test context (current running state)
typedef struct {
    hal_test_suite_t* suite;
    size_t current_test;
    uint64_t suite_start_time;
    bool suite_setup_done;
} hal_test_context_t;

// Performance benchmark structure
typedef struct {
    char name[64];
    char description[256];
    
    // Benchmark metrics
    uint64_t memory_alloc_time_ns;
    uint64_t memory_free_time_ns;
    uint64_t page_fault_time_ns;
    uint64_t interrupt_latency_ns;
    uint64_t context_switch_time_ns;
    uint64_t syscall_overhead_ns;
    
    // Throughput metrics
    uint64_t memory_bandwidth_mbps;
    uint64_t cache_bandwidth_mbps;
    uint64_t dma_throughput_mbps;
    uint64_t io_throughput_mbps;
    
    // CPU metrics
    uint64_t cpu_cycles_per_instruction;
    float cpu_cache_miss_rate;
    float cpu_branch_miss_rate;
    uint32_t cpu_frequency_mhz;
    
    // Platform specific
    char platform[32];
    char architecture[16];
    uint32_t num_cpus;
    uint64_t total_memory;
} hal_performance_benchmark_t;

// Callback function types
typedef void (*hal_test_callback_t)(hal_test_result_t* result);
typedef void (*hal_test_progress_callback_t)(const char* test_name, float progress);
typedef void (*hal_test_log_callback_t)(const char* message);

// Assertion macros for tests
#define HAL_TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            if (context->message) { \
                snprintf(context->message, context->message_size, \
                        "Assertion failed: %s at %s:%d", message, __FILE__, __LINE__); \
            } \
            return -EFAULT; \
        } \
    } while (0)

#define HAL_TEST_ASSERT_EQ(expected, actual, message) \
    HAL_TEST_ASSERT((expected) == (actual), message)

#define HAL_TEST_ASSERT_NE(expected, actual, message) \
    HAL_TEST_ASSERT((expected) != (actual), message)

#define HAL_TEST_ASSERT_LT(a, b, message) \
    HAL_TEST_ASSERT((a) < (b), message)

#define HAL_TEST_ASSERT_LE(a, b, message) \
    HAL_TEST_ASSERT((a) <= (b), message)

#define HAL_TEST_ASSERT_GT(a, b, message) \
    HAL_TEST_ASSERT((a) > (b), message)

#define HAL_TEST_ASSERT_GE(a, b, message) \
    HAL_TEST_ASSERT((a) >= (b), message)

#define HAL_TEST_ASSERT_NULL(ptr, message) \
    HAL_TEST_ASSERT((ptr) == NULL, message)

#define HAL_TEST_ASSERT_NOT_NULL(ptr, message) \
    HAL_TEST_ASSERT((ptr) != NULL, message)

#define HAL_TEST_ASSERT_SUCCESS(result, message) \
    HAL_TEST_ASSERT((result) == HAL_SUCCESS, message)

// Function prototypes

// Test framework management
int hal_test_framework_init(void);
void hal_test_framework_shutdown(void);
int hal_test_set_config(const hal_test_config_t* config);
int hal_test_get_config(hal_test_config_t* config);

// Test execution
int hal_test_run_all(void);
int hal_test_run_suite(const char* suite_name);
int hal_test_run_test(const char* suite_name, const char* test_name);
int hal_test_run_category(hal_test_category_t category);
int hal_test_run_priority(hal_test_priority_t min_priority);

// Test suite management
int hal_test_register_suite(const hal_test_suite_t* suite);
int hal_test_unregister_suite(const char* suite_name);
int hal_test_list_suites(char suites[][64], size_t* count);
hal_test_suite_t* hal_test_find_suite(const char* name);

// Test case management
int hal_test_add_test_case(const char* suite_name, const hal_test_case_t* test_case);
int hal_test_remove_test_case(const char* suite_name, const char* test_name);
int hal_test_list_tests(const char* suite_name, char tests[][64], size_t* count);

// Results and reporting
int hal_test_get_results(hal_test_result_t* results, size_t* count);
int hal_test_get_suite_results(const char* suite_name, hal_test_result_t* results, size_t* count);
int hal_test_get_statistics(hal_test_statistics_t* stats);
int hal_test_export_results(const char* filename, const char* format);
void hal_test_print_summary(void);

// Callback registration
int hal_test_register_callback(hal_test_callback_t callback);
int hal_test_register_progress_callback(hal_test_progress_callback_t callback);
int hal_test_register_log_callback(hal_test_log_callback_t callback);

// Validation and verification
int hal_test_validate_installation(void);
int hal_test_verify_hardware_compatibility(void);
int hal_test_check_system_requirements(void);
int hal_test_validate_configuration(void);

// Performance testing
int hal_test_benchmark_performance(hal_performance_benchmark_t* benchmark);
int hal_test_stress_test(uint32_t duration_ms);
int hal_test_memory_stress_test(size_t memory_size, uint32_t duration_ms);
int hal_test_cpu_stress_test(uint32_t num_threads, uint32_t duration_ms);

// Compatibility testing
int hal_test_platform_compatibility(void);
int hal_test_driver_compatibility(void);
int hal_test_api_compatibility(uint32_t api_version);

// Regression testing
int hal_test_run_regression_suite(void);
int hal_test_compare_results(const char* baseline_file, const char* current_file);

// Mock and simulation support
int hal_test_enable_mock_mode(bool enable);
int hal_test_mock_device(uint32_t vendor_id, uint32_t device_id, const char* mock_data);
int hal_test_simulate_hardware_failure(hal_hardware_type_t type, const char* failure_mode);

// Debugging and diagnostics
void hal_test_enable_verbose_logging(bool enable);
void hal_test_dump_test_state(void);
int hal_test_debug_test_case(const char* suite_name, const char* test_name);
int hal_test_profile_test_case(const char* suite_name, const char* test_name);

// Utility functions
const char* hal_test_status_to_string(hal_test_status_t status);
const char* hal_test_category_to_string(hal_test_category_t category);
const char* hal_test_priority_to_string(hal_test_priority_t priority);
uint64_t hal_test_get_timestamp(void);
void hal_test_sleep_ms(uint32_t milliseconds);

// Specific test implementations
int test_hal_initialization(hal_test_case_context_t* context);
int test_cpu_operations_detailed(hal_test_case_context_t* context);
int test_memory_operations_detailed(hal_test_case_context_t* context);
int test_interrupt_operations_detailed(hal_test_case_context_t* context);
int test_device_discovery(hal_test_case_context_t* context);
int test_device_enumeration(hal_test_case_context_t* context);
int test_performance_monitoring(hal_test_case_context_t* context);
int test_power_management(hal_test_case_context_t* context);
int test_memory_allocation_performance(hal_test_case_context_t* context);
int test_interrupt_latency(hal_test_case_context_t* context);
int test_context_switch_time(hal_test_case_context_t* context);
int test_cache_coherency(hal_test_case_context_t* context);
int test_numa_awareness(hal_test_case_context_t* context);
int test_virtualization_support(hal_test_case_context_t* context);
int test_security_features(hal_test_case_context_t* context);
int test_error_handling(hal_test_case_context_t* context);
int test_resource_cleanup(hal_test_case_context_t* context);

// Platform-specific tests
int test_x86_64_features(hal_test_case_context_t* context);
int test_arm64_features(hal_test_case_context_t* context);
int test_acpi_functionality(hal_test_case_context_t* context);
int test_device_tree_parsing(hal_test_case_context_t* context);
int test_pci_enumeration(hal_test_case_context_t* context);
int test_usb_detection(hal_test_case_context_t* context);

// Stress tests
int stress_test_memory_allocation(hal_test_case_context_t* context);
int stress_test_interrupt_handling(hal_test_case_context_t* context);
int stress_test_device_operations(hal_test_case_context_t* context);
int stress_test_multicore_operations(hal_test_case_context_t* context);

// Internal helper functions
void register_builtin_test_suites(void);
int setup_test_environment(void);
void cleanup_test_environment(void);
int verify_test_prerequisites(hal_test_case_t* test_case);

#ifdef __cplusplus
}
#endif

#endif // HAL_INTEGRATION_TESTS_H