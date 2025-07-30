#include "unit/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test suite registration functions
extern void register_memory_tests(void);
extern void register_process_tests(void);
extern void register_integration_tests(void);
extern void register_system_tests(void);

// Command line argument parsing
typedef struct {
    bool run_unit_tests;
    bool run_integration_tests;
    bool run_system_tests;
    bool run_all_tests;
    bool enable_hardware_tests;
    bool verbose_output;
    char* filter_suite;
    char* filter_test;
} test_config_t;

void print_usage(const char* program_name) {
    printf("RaeenOS Test Runner\n");
    printf("==================\n\n");
    printf("Usage: %s [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -u, --unit           Run unit tests only\n");
    printf("  -i, --integration    Run integration tests only\n");
    printf("  -s, --system         Run system tests only\n");
    printf("  -a, --all            Run all tests (default)\n");
    printf("  -h, --hardware       Enable hardware-dependent tests\n");
    printf("  -v, --verbose        Enable verbose output\n");
    printf("  --suite <name>       Run specific test suite\n");
    printf("  --test <name>        Run specific test\n");
    printf("  --help               Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --unit --verbose\n", program_name);
    printf("  %s --suite \"Memory Management\"\n", program_name);
    printf("  %s --test test_pmm_allocation\n", program_name);
    printf("  %s --integration --hardware\n", program_name);
}

int parse_arguments(int argc, char* argv[], test_config_t* config) {
    // Initialize config with defaults
    memset(config, 0, sizeof(test_config_t));
    config->run_all_tests = true;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--unit") == 0) {
            config->run_unit_tests = true;
            config->run_all_tests = false;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--integration") == 0) {
            config->run_integration_tests = true;
            config->run_all_tests = false;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--system") == 0) {
            config->run_system_tests = true;
            config->run_all_tests = false;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            config->run_all_tests = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--hardware") == 0) {
            config->enable_hardware_tests = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config->verbose_output = true;
        } else if (strcmp(argv[i], "--suite") == 0) {
            if (i + 1 < argc) {
                config->filter_suite = argv[++i];
            } else {
                printf("Error: --suite requires a suite name\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--test") == 0) {
            if (i + 1 < argc) {
                config->filter_test = argv[++i];
            } else {
                printf("Error: --test requires a test name\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        } else {
            printf("Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return -1;
        }
    }
    
    return 0;
}

void print_test_configuration(const test_config_t* config) {
    printf("Test Configuration:\n");
    printf("==================\n");
    
    if (config->run_all_tests) {
        printf("Test Scope: All tests\n");
    } else {
        printf("Test Scope: ");
        if (config->run_unit_tests) printf("Unit ");
        if (config->run_integration_tests) printf("Integration ");
        if (config->run_system_tests) printf("System ");
        printf("\n");
    }
    
    printf("Hardware Tests: %s\n", config->enable_hardware_tests ? "Enabled" : "Disabled");
    printf("Verbose Output: %s\n", config->verbose_output ? "Enabled" : "Disabled");
    
    if (config->filter_suite) {
        printf("Suite Filter: %s\n", config->filter_suite);
    }
    
    if (config->filter_test) {
        printf("Test Filter: %s\n", config->filter_test);
    }
    
    printf("\n");
}

void setup_test_environment(const test_config_t* config) {
    // Set global test configuration
    if (config->verbose_output) {
        // Enable verbose logging
        set_test_verbosity(true);
    }
    
    if (!config->enable_hardware_tests) {
        // Disable hardware-dependent tests
        set_hardware_tests_enabled(false);
    }
    
    // Initialize test environment
    printf("Initializing test environment...\n");
    
    // Set up mock hardware if hardware tests are disabled
    if (!config->enable_hardware_tests) {
        setup_mock_hardware();
    }
    
    // Initialize kernel subsystems for testing
    setup_test_kernel_environment();
}

int run_filtered_tests(const test_config_t* config) {
    int total_failures = 0;
    
    // Register test suites based on configuration
    if (config->run_all_tests || config->run_unit_tests) {
        printf("Registering unit test suites...\n");
        register_memory_tests();
        register_process_tests();
    }
    
    if (config->run_all_tests || config->run_integration_tests) {
        printf("Registering integration test suites...\n");
        register_integration_tests();
    }
    
    if (config->run_all_tests || config->run_system_tests) {
        printf("Registering system test suites...\n");
        register_system_tests();
    }
    
    // Run tests with filters if specified
    if (config->filter_suite || config->filter_test) {
        total_failures = run_filtered_test_suites(config->filter_suite, config->filter_test);
    } else {
        total_failures = run_all_tests();
    }
    
    return total_failures;
}

void generate_test_report(int failures, int total_tests) {
    printf("\n");
    printf("========================================\n");
    printf("         RaeenOS Test Report\n");
    printf("========================================\n");
    
    if (failures == 0) {
        printf("🎉 ALL TESTS PASSED!\n");
        printf("Total tests executed: %d\n", total_tests);
        printf("Success rate: 100%%\n");
    } else {
        printf("❌ TESTS FAILED\n");
        printf("Total tests executed: %d\n", total_tests);
        printf("Failed tests: %d\n", failures);
        printf("Success rate: %.1f%%\n", 
               ((double)(total_tests - failures) / total_tests) * 100.0);
    }
    
    printf("\nTest execution completed at: %s", get_current_timestamp());
    printf("========================================\n");
}

// Mock functions for test environment setup
void set_test_verbosity(bool verbose) {
    // Set global verbose flag for test framework
    // This would control detailed output during test execution
}

void set_hardware_tests_enabled(bool enabled) {
    // Set global flag to enable/disable hardware-dependent tests
    // Tests marked with requires_hardware would check this flag
}

void setup_mock_hardware(void) {
    // Initialize mock hardware devices for testing without real hardware
    printf("Setting up mock hardware environment...\n");
    
    // Mock PCI devices
    // Mock storage devices  
    // Mock network interfaces
    // Mock graphics adapters
    // Mock audio devices
}

void setup_test_kernel_environment(void) {
    // Initialize minimal kernel environment for testing
    printf("Setting up test kernel environment...\n");
    
    // Initialize memory management for tests
    // Set up minimal process scheduler
    // Initialize device manager stubs
    // Set up filesystem stubs
}

const char* get_current_timestamp(void) {
    static char timestamp[32];
    // Get current time and format as string
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S\n", tm_info);
    return timestamp;
}

int run_filtered_test_suites(const char* suite_filter, const char* test_filter) {
    // Implementation would filter and run specific test suites/tests
    // For now, just run all tests
    return run_all_tests();
}

int main(int argc, char* argv[]) {
    test_config_t config;
    int failures = 0;
    
    // Parse command line arguments
    int parse_result = parse_arguments(argc, argv, &config);
    if (parse_result != 0) {
        return parse_result > 0 ? 0 : 1; // Help requested or error
    }
    
    // Initialize test framework
    test_framework_init();
    
    // Print configuration
    print_test_configuration(&config);
    
    // Set up test environment
    setup_test_environment(&config);
    
    printf("Starting RaeenOS test execution...\n\n");
    
    // Run tests
    failures = run_filtered_tests(&config);
    
    // Generate report
    int total_tests = get_total_test_count();
    generate_test_report(failures, total_tests);
    
    // Cleanup
    test_framework_cleanup();
    
    // Return appropriate exit code
    return failures == 0 ? 0 : 1;
}