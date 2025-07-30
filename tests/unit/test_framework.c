#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global test framework state
static test_suite_t* registered_suites = NULL;
static int total_suites = 0;
static mock_call_t* mock_calls = NULL;

// Memory tracking for leak detection
typedef struct memory_block {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct memory_block* next;
} memory_block_t;

static memory_block_t* allocated_blocks = NULL;
static size_t total_allocated = 0;
static size_t total_freed = 0;

void test_framework_init(void) {
    registered_suites = NULL;
    total_suites = 0;
    mock_calls = NULL;
    test_memory_init();
    
    printf("RaeenOS Test Framework v1.0\n");
    printf("==============================\n");
}

void test_framework_cleanup(void) {
    // Clean up test suites
    test_suite_t* current = registered_suites;
    while (current) {
        test_suite_t* next = (test_suite_t*)current->next;
        free(current);
        current = next;
    }
    
    // Clean up mocks
    mock_reset_all();
    
    // Check for memory leaks
    int leaks = test_memory_check_leaks();
    if (leaks > 0) {
        printf("WARNING: %d memory leaks detected during testing\n", leaks);
    }
    
    test_memory_cleanup();
}

test_suite_t* create_test_suite(const char* name, const char* description) {
    test_suite_t* suite = malloc(sizeof(test_suite_t));
    if (!suite) {
        return NULL;
    }
    
    suite->name = name;
    suite->description = description;
    suite->setup = NULL;
    suite->teardown = NULL;
    suite->test_cases = NULL;
    suite->test_count = 0;
    suite->passed = 0;
    suite->failed = 0;
    suite->skipped = 0;
    suite->errors = 0;
    
    // Add to global list
    suite->next = registered_suites;
    registered_suites = suite;
    total_suites++;
    
    return suite;
}

void register_test_case(test_suite_t* suite, test_case_t* test_case) {
    if (!suite || !test_case) {
        return;
    }
    
    test_case->next = suite->test_cases;
    suite->test_cases = test_case;
    suite->test_count++;
}

int run_test_suite(test_suite_t* suite) {
    if (!suite) {
        return -1;
    }
    
    printf("\n=== Running Test Suite: %s ===\n", suite->name);
    printf("Description: %s\n", suite->description);
    printf("Test count: %d\n\n", suite->test_count);
    
    // Run setup if provided
    if (suite->setup) {
        suite->setup();
    }
    
    // Run all test cases
    test_case_t* current = suite->test_cases;
    while (current) {
        printf("Running test: %s... ", current->name);
        fflush(stdout);
        
        // Check if test requires hardware that's not available
        if (current->requires_hardware) {
            // In CI/CD, hardware tests might be skipped or run in emulation
            printf("[HARDWARE] ");
        }
        
        // Run the test
        test_result_t result = current->test_func();
        
        switch (result) {
            case TEST_PASS:
                printf("PASS\n");
                suite->passed++;
                break;
            case TEST_FAIL:
                printf("FAIL\n");
                suite->failed++;
                break;
            case TEST_SKIP:
                printf("SKIP\n");
                suite->skipped++;
                break;
            case TEST_ERROR:
                printf("ERROR\n");
                suite->errors++;
                break;
        }
        
        current = current->next;
    }
    
    // Run teardown if provided
    if (suite->teardown) {
        suite->teardown();
    }
    
    test_print_results(suite);
    return suite->failed + suite->errors;
}

int run_all_tests(void) {
    printf("Running all registered test suites...\n");
    
    int total_failures = 0;
    int total_tests = 0;
    int total_passed = 0;
    int total_failed = 0;
    int total_skipped = 0;
    int total_errors = 0;
    
    test_suite_t* current = registered_suites;
    while (current) {
        int failures = run_test_suite(current);
        total_failures += failures;
        total_tests += current->test_count;
        total_passed += current->passed;
        total_failed += current->failed;
        total_skipped += current->skipped;
        total_errors += current->errors;
        
        current = (test_suite_t*)current->next;
    }
    
    printf("\n=== Final Test Results ===\n");
    printf("Total Suites: %d\n", total_suites);
    printf("Total Tests: %d\n", total_tests);
    printf("Passed: %d\n", total_passed);
    printf("Failed: %d\n", total_failed);
    printf("Skipped: %d\n", total_skipped);
    printf("Errors: %d\n", total_errors);
    
    if (total_failures == 0) {
        printf("\n🎉 ALL TESTS PASSED!\n");
    } else {
        printf("\n❌ %d TESTS FAILED\n", total_failures);
    }
    
    return total_failures;
}

void test_log_failure(const char* file, int line, const char* func, const char* message) {
    printf("\n  ASSERTION FAILED: %s\n", message);
    printf("  Location: %s:%d in %s()\n", file, line, func);
}

void test_log_failure_eq(const char* file, int line, const char* func, const char* message, 
                        uint64_t expected, uint64_t actual) {
    printf("\n  ASSERTION FAILED: %s\n", message);
    printf("  Expected: %lu, Actual: %lu\n", expected, actual);
    printf("  Location: %s:%d in %s()\n", file, line, func);
}

void test_log_failure_neq(const char* file, int line, const char* func, const char* message, 
                         uint64_t not_expected, uint64_t actual) {
    printf("\n  ASSERTION FAILED: %s\n", message);
    printf("  Should not equal: %lu, but was: %lu\n", not_expected, actual);
    printf("  Location: %s:%d in %s()\n", file, line, func);
}

void test_print_results(test_suite_t* suite) {
    printf("\n--- %s Results ---\n", suite->name);
    printf("Passed: %d/%d\n", suite->passed, suite->test_count);
    printf("Failed: %d/%d\n", suite->failed, suite->test_count);
    printf("Skipped: %d/%d\n", suite->skipped, suite->test_count);
    printf("Errors: %d/%d\n", suite->errors, suite->test_count);
    
    double success_rate = suite->test_count > 0 ? 
        (double)suite->passed / suite->test_count * 100.0 : 0.0;
    printf("Success Rate: %.1f%%\n", success_rate);
}

// Mock system implementation
void mock_reset_all(void) {
    mock_call_t* current = mock_calls;
    while (current) {
        mock_call_t* next = current->next;
        if (current->args) free(current->args);
        if (current->return_value) free(current->return_value);
        free(current);
        current = next;
    }
    mock_calls = NULL;
}

void mock_expect_call(const char* func_name, void* args, size_t args_size, 
                     void* return_value, size_t return_size) {
    mock_call_t* call = malloc(sizeof(mock_call_t));
    if (!call) return;
    
    call->function_name = func_name;
    call->call_count = 0;
    
    if (args && args_size > 0) {
        call->args = malloc(args_size);
        memcpy(call->args, args, args_size);
        call->args_size = args_size;
    } else {
        call->args = NULL;
        call->args_size = 0;
    }
    
    if (return_value && return_size > 0) {
        call->return_value = malloc(return_size);
        memcpy(call->return_value, return_value, return_size);
        call->return_size = return_size;
    } else {
        call->return_value = NULL;
        call->return_size = 0;
    }
    
    call->next = mock_calls;
    mock_calls = call;
}

bool mock_verify_call(const char* func_name, void* args, size_t args_size) {
    mock_call_t* current = mock_calls;
    while (current) {
        if (strcmp(current->function_name, func_name) == 0) {
            current->call_count++;
            
            if (current->args_size != args_size) {
                return false;
            }
            
            if (current->args && args) {
                return memcmp(current->args, args, args_size) == 0;
            } else if (!current->args && !args) {
                return true;
            }
            
            return false;
        }
        current = current->next;
    }
    return false;
}

void* mock_get_return_value(const char* func_name) {
    mock_call_t* current = mock_calls;
    while (current) {
        if (strcmp(current->function_name, func_name) == 0) {
            return current->return_value;
        }
        current = current->next;
    }
    return NULL;
}

// Memory tracking implementation
void test_memory_init(void) {
    allocated_blocks = NULL;
    total_allocated = 0;
    total_freed = 0;
}

void test_memory_cleanup(void) {
    memory_block_t* current = allocated_blocks;
    while (current) {
        memory_block_t* next = current->next;
        free(current);
        current = next;
    }
    allocated_blocks = NULL;
}

void* test_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    
    memory_block_t* block = malloc(sizeof(memory_block_t));
    if (!block) {
        free(ptr);
        return NULL;
    }
    
    block->ptr = ptr;
    block->size = size;
    block->file = file;
    block->line = line;
    block->next = allocated_blocks;
    allocated_blocks = block;
    
    total_allocated += size;
    return ptr;
}

void test_free(void* ptr, const char* file, int line) {
    if (!ptr) return;
    
    memory_block_t** current = &allocated_blocks;
    while (*current) {
        if ((*current)->ptr == ptr) {
            memory_block_t* to_free = *current;
            total_freed += to_free->size;
            *current = to_free->next;
            free(to_free);
            break;
        }
        current = &(*current)->next;
    }
    
    free(ptr);
}

int test_memory_check_leaks(void) {
    int leak_count = 0;
    memory_block_t* current = allocated_blocks;
    
    if (current) {
        printf("\n=== Memory Leaks Detected ===\n");
    }
    
    while (current) {
        printf("LEAK: %zu bytes allocated at %s:%d\n", 
               current->size, current->file, current->line);
        leak_count++;
        current = current->next;
    }
    
    if (leak_count == 0) {
        printf("Memory leak check: PASSED\n");
    }
    
    return leak_count;
}

// Performance benchmarking implementation
void test_benchmark_start(test_benchmark_t* bench) {
    if (!bench) return;
    
    // Use rdtsc on x86 or equivalent timing mechanism
    #ifdef __x86_64__
    __asm__ volatile ("rdtsc" : "=A" (bench->start_cycles));
    #else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    bench->start_cycles = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    #endif
}

void test_benchmark_end(test_benchmark_t* bench) {
    if (!bench) return;
    
    #ifdef __x86_64__
    __asm__ volatile ("rdtsc" : "=A" (bench->end_cycles));
    #else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    bench->end_cycles = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    #endif
    
    bench->duration_us = (bench->end_cycles - bench->start_cycles) / 1000;
}

bool test_benchmark_check_performance(test_benchmark_t* bench, uint64_t max_cycles) {
    if (!bench) return false;
    
    uint64_t duration = bench->end_cycles - bench->start_cycles;
    return duration <= max_cycles;
}