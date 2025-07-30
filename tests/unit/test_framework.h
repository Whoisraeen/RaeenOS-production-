#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Test framework for RaeenOS unit testing
// Supports isolated testing of kernel components, drivers, and userland

// Test result status
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2,
    TEST_ERROR = 3
} test_result_t;

// Test case structure
typedef struct test_case {
    const char* name;
    const char* description;
    test_result_t (*test_func)(void);
    bool requires_hardware;
    const char* subsystem;
    struct test_case* next;
} test_case_t;

// Test suite structure
typedef struct test_suite {
    const char* name;
    const char* description;
    void (*setup)(void);
    void (*teardown)(void);
    test_case_t* test_cases;
    int test_count;
    int passed;
    int failed;
    int skipped;
    int errors;
} test_suite_t;

// Test macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            test_log_failure(__FILE__, __LINE__, __func__, message); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            test_log_failure_eq(__FILE__, __LINE__, __func__, message, expected, actual); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_NEQ(not_expected, actual, message) \
    do { \
        if ((not_expected) == (actual)) { \
            test_log_failure_neq(__FILE__, __LINE__, __func__, message, not_expected, actual); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr, message) \
    TEST_ASSERT((ptr) == NULL, message)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    TEST_ASSERT((ptr) != NULL, message)

#define DEFINE_TEST(test_name, test_desc, subsys, hw_req) \
    test_result_t test_##test_name(void); \
    test_case_t test_case_##test_name = { \
        .name = #test_name, \
        .description = test_desc, \
        .test_func = test_##test_name, \
        .requires_hardware = hw_req, \
        .subsystem = subsys, \
        .next = NULL \
    }; \
    test_result_t test_##test_name(void)

#define REGISTER_TEST(suite, test_name) \
    register_test_case(suite, &test_case_##test_name)

// Function declarations
void test_framework_init(void);
void test_framework_cleanup(void);
test_suite_t* create_test_suite(const char* name, const char* description);
void register_test_case(test_suite_t* suite, test_case_t* test_case);
int run_test_suite(test_suite_t* suite);
int run_all_tests(void);
void test_log_failure(const char* file, int line, const char* func, const char* message);
void test_log_failure_eq(const char* file, int line, const char* func, const char* message, 
                        uint64_t expected, uint64_t actual);
void test_log_failure_neq(const char* file, int line, const char* func, const char* message, 
                         uint64_t not_expected, uint64_t actual);
void test_print_results(test_suite_t* suite);

// Mock and stub support
typedef struct mock_call {
    const char* function_name;
    void* args;
    size_t args_size;
    void* return_value;
    size_t return_size;
    int call_count;
    struct mock_call* next;
} mock_call_t;

void mock_reset_all(void);
void mock_expect_call(const char* func_name, void* args, size_t args_size, 
                     void* return_value, size_t return_size);
bool mock_verify_call(const char* func_name, void* args, size_t args_size);
void* mock_get_return_value(const char* func_name);

// Memory leak detection for tests
void test_memory_init(void);
void test_memory_cleanup(void);
void* test_malloc(size_t size, const char* file, int line);
void test_free(void* ptr, const char* file, int line);
int test_memory_check_leaks(void);

#define TEST_MALLOC(size) test_malloc(size, __FILE__, __LINE__)
#define TEST_FREE(ptr) test_free(ptr, __FILE__, __LINE__)

// Performance benchmarking for tests
typedef struct {
    uint64_t start_cycles;
    uint64_t end_cycles;
    uint64_t duration_us;
} test_benchmark_t;

void test_benchmark_start(test_benchmark_t* bench);
void test_benchmark_end(test_benchmark_t* bench);
bool test_benchmark_check_performance(test_benchmark_t* bench, uint64_t max_cycles);

#endif // TEST_FRAMEWORK_H