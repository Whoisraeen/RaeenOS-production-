/**
 * @file hal_integration_tests.c
 * @brief HAL Integration Tests and Validation Suite
 * 
 * This module provides comprehensive testing for the Hardware Abstraction Layer
 * including integration tests, validation suites, and performance benchmarks.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#include "../../include/hal_interface.h"
#include "../../include/types.h"
#include "../../include/errno.h"
#include "../device/hal_device_manager.h"
#include "../performance/hal_performance.h"
#include "../detection/hal_hardware_detection.h"
#include "hal_integration_tests.h"
#include <stddef.h>
#include <string.h>

// Test framework state
static struct {
    hal_test_suite_t test_suites[HAL_MAX_TEST_SUITES];
    size_t suite_count;
    
    hal_test_result_t test_results[HAL_MAX_TEST_RESULTS];
    size_t result_count;
    
    hal_test_config_t config;
    hal_test_statistics_t statistics;
    
    bool initialized;
    bool running;
    
    // Test callbacks
    hal_test_callback_t callbacks[16];
    size_t callback_count;
    
    // Current test context
    hal_test_context_t current_context;
} test_framework = {0};

// Forward declarations
static int run_test_case(hal_test_case_t* test_case);
static void record_test_result(const char* test_name, hal_test_status_t status, 
                              uint64_t duration, const char* message);
static void notify_test_callbacks(hal_test_result_t* result);
static int validate_hal_interface(void);
static int test_cpu_operations(void);
static int test_memory_operations(void);
static int test_interrupt_operations(void);
static int test_device_operations(void);
static int test_performance_operations(void);
static int benchmark_hal_performance(void);

/**
 * Initialize the HAL test framework
 */
int hal_test_framework_init(void)
{
    if (test_framework.initialized) {
        return HAL_SUCCESS;
    }
    
    // Initialize default configuration
    test_framework.config.enable_stress_tests = false;
    test_framework.config.enable_performance_tests = true;
    test_framework.config.enable_compatibility_tests = true;
    test_framework.config.enable_regression_tests = true;
    test_framework.config.test_timeout_ms = 30000; // 30 seconds
    test_framework.config.stress_test_duration_ms = 60000; // 1 minute
    test_framework.config.performance_iterations = 1000;
    test_framework.config.memory_test_size = 1024 * 1024; // 1MB
    
    // Clear statistics
    memset(&test_framework.statistics, 0, sizeof(hal_test_statistics_t));
    
    // Register builtin test suites
    register_builtin_test_suites();
    
    test_framework.initialized = true;
    return HAL_SUCCESS;
}

/**
 * Run all HAL tests
 */
int hal_test_run_all(void)
{
    if (!test_framework.initialized || test_framework.running) {
        return -EINVAL;
    }
    
    test_framework.running = true;
    test_framework.statistics.start_time = hal->timer_get_ticks();
    
    // Clear previous results
    test_framework.result_count = 0;
    
    // Run all test suites
    for (size_t i = 0; i < test_framework.suite_count; i++) {
        hal_test_suite_t* suite = &test_framework.test_suites[i];
        
        if (!suite->enabled) {
            continue;
        }
        
        // Setup suite context
        test_framework.current_context.suite = suite;
        test_framework.current_context.current_test = 0;
        
        // Run suite setup
        if (suite->setup) {
            int result = suite->setup();
            if (result != HAL_SUCCESS) {
                record_test_result(suite->name, HAL_TEST_FAILED, 0, "Suite setup failed");
                continue;
            }
        }
        
        // Run all test cases in suite
        for (size_t j = 0; j < suite->test_count; j++) {
            hal_test_case_t* test_case = &suite->test_cases[j];
            test_framework.current_context.current_test = j;
            
            run_test_case(test_case);
        }
        
        // Run suite teardown
        if (suite->teardown) {
            suite->teardown();
        }
    }
    
    // Calculate final statistics
    test_framework.statistics.end_time = hal->timer_get_ticks();
    test_framework.statistics.total_duration = 
        test_framework.statistics.end_time - test_framework.statistics.start_time;
    
    test_framework.running = false;
    return HAL_SUCCESS;
}

/**
 * Run specific test suite
 */
int hal_test_run_suite(const char* suite_name)
{
    if (!suite_name || !test_framework.initialized) {
        return -EINVAL;
    }
    
    // Find the test suite
    hal_test_suite_t* suite = NULL;
    for (size_t i = 0; i < test_framework.suite_count; i++) {
        if (strcmp(test_framework.test_suites[i].name, suite_name) == 0) {
            suite = &test_framework.test_suites[i];
            break;
        }
    }
    
    if (!suite) {
        return -ENOENT;
    }
    
    // Run the suite
    test_framework.current_context.suite = suite;
    
    if (suite->setup) {
        int result = suite->setup();
        if (result != HAL_SUCCESS) {
            return result;
        }
    }
    
    for (size_t i = 0; i < suite->test_count; i++) {
        test_framework.current_context.current_test = i;
        run_test_case(&suite->test_cases[i]);
    }
    
    if (suite->teardown) {
        suite->teardown();
    }
    
    return HAL_SUCCESS;
}

/**
 * Get test results
 */
int hal_test_get_results(hal_test_result_t* results, size_t* count)
{
    if (!results || !count) {
        return -EINVAL;
    }
    
    size_t max_count = *count;
    size_t actual_count = test_framework.result_count < max_count ? 
                         test_framework.result_count : max_count;
    
    for (size_t i = 0; i < actual_count; i++) {
        results[i] = test_framework.test_results[i];
    }
    
    *count = actual_count;
    return HAL_SUCCESS;
}

/**
 * Get test statistics
 */
int hal_test_get_statistics(hal_test_statistics_t* stats)
{
    if (!stats) {
        return -EINVAL;
    }
    
    *stats = test_framework.statistics;
    return HAL_SUCCESS;
}

/**
 * Register test callback
 */
int hal_test_register_callback(hal_test_callback_t callback)
{
    if (!callback || test_framework.callback_count >= 16) {
        return -EINVAL;
    }
    
    test_framework.callbacks[test_framework.callback_count++] = callback;
    return HAL_SUCCESS;
}

/**
 * Validate HAL installation
 */
int hal_test_validate_installation(void)
{
    int errors = 0;
    
    // Validate HAL interface
    if (validate_hal_interface() != HAL_SUCCESS) {
        errors++;
    }
    
    // Test basic HAL operations
    if (test_cpu_operations() != HAL_SUCCESS) {
        errors++;
    }
    
    if (test_memory_operations() != HAL_SUCCESS) {
        errors++;
    }
    
    if (test_interrupt_operations() != HAL_SUCCESS) {
        errors++;
    }
    
    // Test device management
    if (test_device_operations() != HAL_SUCCESS) {
        errors++;
    }
    
    // Test performance framework
    if (test_performance_operations() != HAL_SUCCESS) {
        errors++;
    }
    
    return (errors == 0) ? HAL_SUCCESS : -EFAULT;
}

/**
 * Benchmark HAL performance
 */
int hal_test_benchmark_performance(hal_performance_benchmark_t* benchmark)
{
    if (!benchmark) {
        return -EINVAL;
    }
    
    return benchmark_hal_performance();
}

/**
 * Test Implementation Functions
 */

static int run_test_case(hal_test_case_t* test_case)
{
    if (!test_case || !test_case->test_function) {
        return -EINVAL;
    }
    
    uint64_t start_time = hal->timer_get_ticks();
    hal_test_status_t status = HAL_TEST_PASSED;
    char message[256] = {0};
    
    // Setup test case context
    hal_test_case_context_t context = {
        .test_case = test_case,
        .timeout_ms = test_framework.config.test_timeout_ms,
        .iterations = 1,
        .message = message,
        .message_size = sizeof(message)
    };
    
    // Run the test
    int result = test_case->test_function(&context);
    if (result != HAL_SUCCESS) {
        status = HAL_TEST_FAILED;
        if (message[0] == '\0') {
            snprintf(message, sizeof(message), "Test failed with error code %d", result);
        }
    }
    
    uint64_t end_time = hal->timer_get_ticks();
    uint64_t duration = end_time - start_time;
    
    // Check for timeout
    uint64_t timeout_ticks = (test_framework.config.test_timeout_ms * 
                             hal->timer_get_frequency()) / 1000;
    if (duration > timeout_ticks) {
        status = HAL_TEST_TIMEOUT;
        snprintf(message, sizeof(message), "Test timed out after %llu ms", 
                (duration * 1000) / hal->timer_get_frequency());
    }
    
    // Record result
    record_test_result(test_case->name, status, duration, message);
    
    // Update statistics
    test_framework.statistics.total_tests++;
    switch (status) {
        case HAL_TEST_PASSED:
            test_framework.statistics.tests_passed++;
            break;
        case HAL_TEST_FAILED:
            test_framework.statistics.tests_failed++;
            break;
        case HAL_TEST_SKIPPED:
            test_framework.statistics.tests_skipped++;
            break;
        case HAL_TEST_TIMEOUT:
            test_framework.statistics.tests_timeout++;
            break;
    }
    
    return HAL_SUCCESS;
}

static void record_test_result(const char* test_name, hal_test_status_t status, 
                              uint64_t duration, const char* message)
{
    if (test_framework.result_count >= HAL_MAX_TEST_RESULTS) {
        return;
    }
    
    hal_test_result_t* result = &test_framework.test_results[test_framework.result_count++];
    
    strncpy(result->test_name, test_name, sizeof(result->test_name) - 1);
    result->status = status;
    result->duration = duration;
    result->timestamp = hal->timer_get_ticks();
    
    if (message) {
        strncpy(result->message, message, sizeof(result->message) - 1);
    }
    
    notify_test_callbacks(result);
}

static void notify_test_callbacks(hal_test_result_t* result)
{
    for (size_t i = 0; i < test_framework.callback_count; i++) {
        test_framework.callbacks[i](result);
    }
}

static int validate_hal_interface(void)
{
    // Check if HAL is initialized
    if (!hal) {
        return -EFAULT;
    }
    
    // Check required function pointers
    if (!hal->init || !hal->cpu_init || !hal->cpu_halt ||
        !hal->mem_alloc_pages || !hal->mem_free_pages ||
        !hal->irq_init || !hal->irq_save || !hal->irq_restore) {
        return -EFAULT;
    }
    
    // Check HAL API version compatibility
    if (!hal_is_api_compatible(1)) {
        return -ENOTSUP;
    }
    
    return HAL_SUCCESS;
}

static int test_cpu_operations(void)
{
    // Test CPU feature detection
    hal_cpu_features_t features;
    int result = hal->cpu_get_features(&features);
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    // Test CPU timestamp
    uint64_t ts1 = hal->cpu_timestamp();
    hal->cpu_pause();
    uint64_t ts2 = hal->cpu_timestamp();
    
    if (ts2 <= ts1) {
        return -EFAULT; // Timestamp should advance
    }
    
    // Test memory barriers
    hal->cpu_memory_barrier();
    
    return HAL_SUCCESS;
}

static int test_memory_operations(void)
{
    // Test page allocation
    void* pages = hal->mem_alloc_pages(1, HAL_MEM_READ | HAL_MEM_WRITE);
    if (!pages) {
        return -ENOMEM;
    }
    
    // Test memory access
    volatile uint32_t* test_ptr = (volatile uint32_t*)pages;
    *test_ptr = 0x12345678;
    if (*test_ptr != 0x12345678) {
        hal->mem_free_pages(pages, 1);
        return -EFAULT;
    }
    
    // Test virtual to physical translation
    phys_addr_t phys = hal->mem_virt_to_phys(pages);
    if (phys == 0) {
        hal->mem_free_pages(pages, 1);
        return -EFAULT;
    }
    
    // Clean up
    hal->mem_free_pages(pages, 1);
    
    return HAL_SUCCESS;
}

static int test_interrupt_operations(void)
{
    // Test interrupt save/restore
    unsigned long flags1 = hal->irq_save();
    unsigned long flags2 = hal->irq_save();
    
    hal->irq_restore(flags2);
    hal->irq_restore(flags1);
    
    return HAL_SUCCESS;
}

static int test_device_operations(void)
{
    // Test device enumeration
    hal_device_t* devices[32];
    size_t count = 32;
    
    int result = hal_device_get_all(devices, &count);
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    // Should have at least one device (CPU)
    if (count == 0) {
        return -ENODEV;
    }
    
    return HAL_SUCCESS;
}

static int test_performance_operations(void)
{
    // Test CPU topology detection
    hal_cpu_topology_t topology;
    int result = hal_performance_get_cpu_topology(&topology);
    if (result != HAL_SUCCESS) {
        return result;
    }
    
    // Should have at least one CPU
    if (topology.total_cpus == 0) {
        return -EFAULT;
    }
    
    return HAL_SUCCESS;
}

static int benchmark_hal_performance(void)
{
    // Benchmark various HAL operations
    uint64_t iterations = test_framework.config.performance_iterations;
    
    // Benchmark memory allocation
    uint64_t start = hal->timer_get_ticks();
    for (uint64_t i = 0; i < iterations; i++) {
        void* ptr = hal->mem_alloc_pages(1, HAL_MEM_READ | HAL_MEM_WRITE);
        if (ptr) {
            hal->mem_free_pages(ptr, 1);
        }
    }
    uint64_t end = hal->timer_get_ticks();
    
    // Calculate performance metrics
    uint64_t duration = end - start;
    uint64_t ops_per_second = (iterations * hal->timer_get_frequency()) / duration;
    
    // Store results (simplified)
    test_framework.statistics.memory_alloc_ops_per_sec = ops_per_second;
    
    return HAL_SUCCESS;
}

/**
 * Builtin Test Suites
 */

void register_builtin_test_suites(void)
{
    // HAL Interface Test Suite
    hal_test_suite_t* suite = &test_framework.test_suites[test_framework.suite_count++];
    strcpy(suite->name, "hal_interface");
    strcpy(suite->description, "Tests basic HAL interface functionality");
    suite->enabled = true;
    suite->setup = NULL;
    suite->teardown = NULL;
    
    // Add test cases
    hal_test_case_t* test_case = &suite->test_cases[suite->test_count++];
    strcpy(test_case->name, "hal_initialization");
    strcpy(test_case->description, "Test HAL initialization");
    test_case->test_function = test_hal_initialization;
    test_case->category = HAL_TEST_CATEGORY_FUNCTIONAL;
    test_case->priority = HAL_TEST_PRIORITY_HIGH;
    
    test_case = &suite->test_cases[suite->test_count++];
    strcpy(test_case->name, "cpu_operations");
    strcpy(test_case->description, "Test CPU operations");
    test_case->test_function = test_cpu_operations_detailed;
    test_case->category = HAL_TEST_CATEGORY_FUNCTIONAL;
    test_case->priority = HAL_TEST_PRIORITY_HIGH;
    
    test_case = &suite->test_cases[suite->test_count++];
    strcpy(test_case->name, "memory_operations");
    strcpy(test_case->description, "Test memory operations");
    test_case->test_function = test_memory_operations_detailed;
    test_case->category = HAL_TEST_CATEGORY_FUNCTIONAL;
    test_case->priority = HAL_TEST_PRIORITY_HIGH;
    
    // Device Management Test Suite
    suite = &test_framework.test_suites[test_framework.suite_count++];
    strcpy(suite->name, "device_management");
    strcpy(suite->description, "Tests device discovery and management");
    suite->enabled = true;
    
    test_case = &suite->test_cases[suite->test_count++];
    strcpy(test_case->name, "device_discovery");
    strcpy(test_case->description, "Test device discovery");
    test_case->test_function = test_device_discovery;
    test_case->category = HAL_TEST_CATEGORY_FUNCTIONAL;
    test_case->priority = HAL_TEST_PRIORITY_MEDIUM;
    
    // Performance Test Suite
    suite = &test_framework.test_suites[test_framework.suite_count++];
    strcpy(suite->name, "performance");
    strcpy(suite->description, "Tests performance optimization framework");
    suite->enabled = test_framework.config.enable_performance_tests;
    
    test_case = &suite->test_cases[suite->test_count++];
    strcpy(test_case->name, "performance_monitoring");
    strcpy(test_case->description, "Test performance monitoring");
    test_case->test_function = test_performance_monitoring;
    test_case->category = HAL_TEST_CATEGORY_PERFORMANCE;
    test_case->priority = HAL_TEST_PRIORITY_MEDIUM;
}

// Test implementation functions (stubs)
int test_hal_initialization(hal_test_case_context_t* context) { (void)context; return HAL_SUCCESS; }
int test_cpu_operations_detailed(hal_test_case_context_t* context) { (void)context; return test_cpu_operations(); }
int test_memory_operations_detailed(hal_test_case_context_t* context) { (void)context; return test_memory_operations(); }
int test_device_discovery(hal_test_case_context_t* context) { (void)context; return test_device_operations(); }
int test_performance_monitoring(hal_test_case_context_t* context) { (void)context; return test_performance_operations(); }