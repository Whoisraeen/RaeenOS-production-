/**
 * @file security_tests.c
 * @brief Comprehensive Security Framework Test Suite
 * 
 * This test suite validates all aspects of the RaeenOS security framework:
 * - Core security functionality
 * - MAC policy enforcement
 * - Sandbox isolation
 * - Memory protection mechanisms
 * - Cryptographic operations
 * - Audit logging
 * - Intrusion detection
 * - Network security
 * - Integration testing
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "../security_core.h"
#include "../../include/process_interface.h"
#include "../../memory.h"
#include "../../string.h"

// Test framework
static struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    char current_test[256];
} test_state = {0};

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            kernel_printf("FAIL: %s - %s\n", test_state.current_test, message); \
            test_state.tests_failed++; \
            return -1; \
        } \
    } while(0)

#define TEST_START(name) \
    do { \
        strcpy(test_state.current_test, name); \
        test_state.tests_run++; \
        kernel_printf("TEST: %s\n", name); \
    } while(0)

#define TEST_PASS() \
    do { \
        kernel_printf("PASS: %s\n", test_state.current_test); \
        test_state.tests_passed++; \
        return 0; \
    } while(0)

// Forward declarations
static int test_security_core(void);
static int test_mac_framework(void);
static int test_sandbox_system(void);
static int test_memory_protection(void);
static int test_crypto_services(void);
static int test_audit_system(void);
static int test_intrusion_detection(void);
static int test_network_security(void);
static int test_integration(void);
static int test_performance(void);

/**
 * Run all security framework tests
 */
int run_security_tests(void) {
    kernel_printf("=== RaeenOS Security Framework Test Suite ===\n");
    
    // Initialize test state
    test_state.tests_run = 0;
    test_state.tests_passed = 0;
    test_state.tests_failed = 0;
    
    // Run test suites
    test_security_core();
    test_mac_framework();
    test_sandbox_system();
    test_memory_protection();
    test_crypto_services();
    test_audit_system();
    test_intrusion_detection();
    test_network_security();
    test_integration();
    test_performance();
    
    // Print results
    kernel_printf("\n=== Test Results ===\n");
    kernel_printf("Tests run: %d\n", test_state.tests_run);
    kernel_printf("Tests passed: %d\n", test_state.tests_passed);
    kernel_printf("Tests failed: %d\n", test_state.tests_failed);
    
    if (test_state.tests_failed == 0) {
        kernel_printf("ALL TESTS PASSED!\n");
        return 0;
    } else {
        kernel_printf("SOME TESTS FAILED!\n");
        return -1;
    }
}

/**
 * Test core security functionality
 */
static int test_security_core(void) {
    TEST_START("Security Core Initialization");
    
    // Test initialization
    int ret = security_init();
    TEST_ASSERT(ret == 0, "Security initialization failed");
    
    // Test security level management
    security_level_t level;
    ret = security_get_level(&level);
    TEST_ASSERT(ret == 0, "Failed to get security level");
    TEST_ASSERT(level == SECURITY_LEVEL_ENHANCED, "Default security level incorrect");
    
    ret = security_set_level(SECURITY_LEVEL_HIGH);
    TEST_ASSERT(ret == 0, "Failed to set security level");
    
    ret = security_get_level(&level);
    TEST_ASSERT(ret == 0, "Failed to get updated security level");
    TEST_ASSERT(level == SECURITY_LEVEL_HIGH, "Security level not updated");
    
    // Test security context creation
    security_context_t* context;
    ret = security_create_context("test_u:test_r:test_t:s0", &context);
    TEST_ASSERT(ret == 0, "Failed to create security context");
    TEST_ASSERT(context != NULL, "Security context is NULL");
    
    // Test context validation
    ret = security_check_context(context);
    TEST_ASSERT(ret == 0, "Security context validation failed");
    
    // Test capability checking
    ret = security_check_capability(CAP_SYS_ADMIN);
    TEST_ASSERT(ret != 0, "Should not have admin capability by default");
    
    security_destroy_context(context);
    
    TEST_PASS();
}

/**
 * Test MAC framework
 */
static int test_mac_framework(void) {
    TEST_START("MAC Framework");
    
    // Test MAC initialization
    int ret = mac_init();
    TEST_ASSERT(ret == 0, "MAC initialization failed");
    
    // Test policy information retrieval
    char policy_name[64];
    bool enforcing;
    size_t rule_count;
    ret = mac_get_policy_info(policy_name, sizeof(policy_name), &enforcing, &rule_count);
    TEST_ASSERT(ret == 0, "Failed to get MAC policy info");
    TEST_ASSERT(strlen(policy_name) > 0, "Policy name is empty");
    TEST_ASSERT(rule_count > 0, "No MAC rules loaded");
    
    // Test security context validation
    ret = mac_validate_context("user_u:user_r:user_t:s0");
    TEST_ASSERT(ret == 0, "Valid security context rejected");
    
    ret = mac_validate_context("");
    TEST_ASSERT(ret != 0, "Empty security context accepted");
    
    // Test permission checking
    security_context_t subject_ctx = {0};
    security_context_t object_ctx = {0};
    
    strcpy(subject_ctx.context, "user_u:user_r:user_t:s0");
    strcpy(object_ctx.context, "system_u:object_r:file_t:s0");
    
    ret = mac_check_permission(&subject_ctx, &object_ctx, "read", 0x01);
    // This may succeed or fail depending on policy - just test that it doesn't crash
    
    TEST_PASS();
}

/**
 * Test sandbox system
 */
static int test_sandbox_system(void) {
    TEST_START("Sandbox System");
    
    // Test sandbox initialization
    int ret = sandbox_init();
    TEST_ASSERT(ret == 0, "Sandbox initialization failed");
    
    // Test sandbox profile creation
    sandbox_profile_t* profile;
    ret = security_create_sandbox("test_profile", &profile);
    TEST_ASSERT(ret == 0, "Failed to create sandbox profile");
    TEST_ASSERT(profile != NULL, "Sandbox profile is NULL");
    TEST_ASSERT(strcmp(profile->name, "test_profile") == 0, "Sandbox profile name incorrect");
    
    // Test default profiles exist
    sandbox_profile_t* default_profile;
    ret = security_create_sandbox("default", &default_profile);
    TEST_ASSERT(ret == -EEXIST, "Default profile should already exist");
    
    // Test resource limits
    TEST_ASSERT(profile->limits.max_memory > 0, "Memory limit not set");
    TEST_ASSERT(profile->limits.max_processes > 0, "Process limit not set");
    TEST_ASSERT(profile->limits.max_files > 0, "File limit not set");
    
    // Test network restrictions
    TEST_ASSERT(profile->network.allow_localhost == true, "Localhost should be allowed by default");
    
    security_destroy_sandbox(profile);
    
    TEST_PASS();
}

/**
 * Test memory protection mechanisms
 */
static int test_memory_protection(void) {
    TEST_START("Memory Protection");
    
    // Test memory protection initialization
    int ret = security_init_memory_protection();
    TEST_ASSERT(ret == 0, "Memory protection initialization failed");
    
    // Test ASLR
    ret = security_enable_aslr();
    TEST_ASSERT(ret == 0, "Failed to enable ASLR");
    
    // Test stack protection
    ret = security_enable_stack_protection();
    TEST_ASSERT(ret == 0, "Failed to enable stack protection");
    
    // Test heap protection
    ret = security_enable_heap_protection();
    TEST_ASSERT(ret == 0, "Failed to enable heap protection");
    
    // Test CFI
    ret = security_enable_cfi();
    TEST_ASSERT(ret == 0, "Failed to enable CFI");
    
    // Test stack canary functionality
    process_t test_process = {0};
    test_process.pid = 12345;
    
    uint32_t canary = security_get_stack_canary(&test_process);
    TEST_ASSERT(canary != 0, "Stack canary is zero");
    
    bool valid = security_check_stack_canary(&test_process, canary);
    TEST_ASSERT(valid == true, "Valid stack canary rejected");
    
    valid = security_check_stack_canary(&test_process, canary ^ 0xDEADBEEF);
    TEST_ASSERT(valid == false, "Invalid stack canary accepted");
    
    // Test protected heap allocation
    void* ptr = security_alloc_protected_heap(1024);
    TEST_ASSERT(ptr != NULL, "Protected heap allocation failed");
    
    security_free_protected_heap(ptr);
    
    TEST_PASS();
}

/**
 * Test cryptographic services
 */
static int test_crypto_services(void) {
    TEST_START("Cryptographic Services");
    
    // Test crypto initialization
    int ret = crypto_init();
    TEST_ASSERT(ret == 0, "Crypto initialization failed");
    
    // Test key generation
    crypto_key_t* key;
    ret = crypto_generate_key(CRYPTO_ALG_AES, 256, &key);
    TEST_ASSERT(ret == 0, "Failed to generate AES key");
    TEST_ASSERT(key != NULL, "Generated key is NULL");
    TEST_ASSERT(key->key_length == 256, "Key length incorrect");
    TEST_ASSERT(key->algorithm == CRYPTO_ALG_AES, "Key algorithm incorrect");
    
    // Test key storage and retrieval
    ret = crypto_store_key(key, "test_key");
    TEST_ASSERT(ret == 0, "Failed to store key");
    
    crypto_key_t* retrieved_key;
    ret = crypto_retrieve_key("test_key", &retrieved_key);
    TEST_ASSERT(ret == 0, "Failed to retrieve key");
    TEST_ASSERT(retrieved_key == key, "Retrieved key is different");
    
    // Test random number generation
    uint8_t random_data[32];
    ret = crypto_generate_random(random_data, sizeof(random_data));
    TEST_ASSERT(ret == 0, "Failed to generate random data");
    
    // Check that random data is not all zeros
    bool all_zero = true;
    for (size_t i = 0; i < sizeof(random_data); i++) {
        if (random_data[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT(all_zero == false, "Random data is all zeros");
    
    // Test encryption/decryption
    const char* plaintext = "Hello, World!";
    void* ciphertext;
    size_t cipher_len;
    ret = crypto_encrypt_data(key, plaintext, strlen(plaintext), &ciphertext, &cipher_len);
    TEST_ASSERT(ret == 0, "Failed to encrypt data");
    TEST_ASSERT(ciphertext != NULL, "Ciphertext is NULL");
    TEST_ASSERT(cipher_len > 0, "Cipher length is zero");
    
    void* decrypted;
    size_t decrypted_len;
    ret = crypto_decrypt_data(key, ciphertext, cipher_len, &decrypted, &decrypted_len);
    TEST_ASSERT(ret == 0, "Failed to decrypt data");
    TEST_ASSERT(decrypted != NULL, "Decrypted data is NULL");
    TEST_ASSERT(decrypted_len == strlen(plaintext), "Decrypted length incorrect");
    TEST_ASSERT(memcmp(decrypted, plaintext, decrypted_len) == 0, "Decrypted data incorrect");
    
    // Cleanup
    ret = crypto_delete_key("test_key");
    TEST_ASSERT(ret == 0, "Failed to delete key");
    
    kfree(ciphertext);
    kfree(decrypted);
    
    TEST_PASS();
}

/**
 * Test audit system
 */
static int test_audit_system(void) {
    TEST_START("Audit System");
    
    // Test audit initialization
    int ret = audit_init();
    TEST_ASSERT(ret == 0, "Audit initialization failed");
    
    // Test audit event logging
    security_event_t test_event = {
        .event_id = 12345,
        .timestamp = get_system_time(),
        .pid = 999,
        .uid = 1000,
        .type = SECURITY_EVENT_FILE_ACCESS,
        .severity = 5,
        .blocked = false
    };
    strcpy(test_event.description, "Test audit event");
    strcpy(test_event.subject, "test_process");
    strcpy(test_event.object, "/test/file");
    strcpy(test_event.action, "read");
    
    ret = audit_log_event(&test_event);
    TEST_ASSERT(ret == 0, "Failed to log audit event");
    
    // Test event retrieval
    security_event_t* events;
    size_t event_count;
    ret = security_get_events(&events, &event_count, 0, SECURITY_EVENT_FILE_ACCESS);
    TEST_ASSERT(ret == 0, "Failed to retrieve audit events");
    TEST_ASSERT(event_count > 0, "No events retrieved");
    
    // Find our test event
    bool found = false;
    for (size_t i = 0; i < event_count; i++) {
        if (events[i].pid == 999 && events[i].uid == 1000) {
            found = true;
            TEST_ASSERT(events[i].type == SECURITY_EVENT_FILE_ACCESS, "Event type incorrect");
            TEST_ASSERT(events[i].severity == 5, "Event severity incorrect");
            TEST_ASSERT(strcmp(events[i].description, "Test audit event") == 0, "Event description incorrect");
            break;
        }
    }
    TEST_ASSERT(found, "Test event not found in audit log");
    
    kfree(events);
    
    // Test audit policy configuration
    ret = security_set_audit_policy(AUDIT_MASK_FILE_ACCESS, false);
    TEST_ASSERT(ret == 0, "Failed to set audit policy");
    
    ret = security_set_audit_policy(AUDIT_MASK_FILE_ACCESS, true);
    TEST_ASSERT(ret == 0, "Failed to restore audit policy");
    
    TEST_PASS();
}

/**
 * Test intrusion detection system
 */
static int test_intrusion_detection(void) {
    TEST_START("Intrusion Detection System");
    
    // Test IDS initialization
    int ret = ids_init();
    TEST_ASSERT(ret == 0, "IDS initialization failed");
    
    // Test IDS rule registration
    ret = ids_register_rule("test_rule:file_access:threshold=5:window=60", NULL);
    TEST_ASSERT(ret == 0, "Failed to register IDS rule");
    
    // Test event analysis
    security_event_t test_event = {
        .event_id = 54321,
        .timestamp = get_system_time(),
        .pid = 888,
        .uid = 1001,
        .type = SECURITY_EVENT_FILE_ACCESS,
        .severity = 3,
        .blocked = false
    };
    strcpy(test_event.description, "Test IDS event");
    
    ret = ids_analyze_event(&test_event);
    TEST_ASSERT(ret >= 0, "IDS event analysis failed");
    
    // Test IDS parameter configuration
    ret = ids_set_parameters(75, 600, false);
    TEST_ASSERT(ret == 0, "Failed to set IDS parameters");
    
    // Test IDS statistics
    ids_statistics_t stats;
    ret = ids_get_statistics(&stats);
    TEST_ASSERT(ret == 0, "Failed to get IDS statistics");
    TEST_ASSERT(stats.enabled == true, "IDS should be enabled");
    TEST_ASSERT(stats.events_analyzed > 0, "No events analyzed");
    
    // Test rule unregistration
    ret = ids_unregister_rule("test_rule:file_access:threshold=5:window=60");
    TEST_ASSERT(ret == 0, "Failed to unregister IDS rule");
    
    TEST_PASS();
}

/**
 * Test network security
 */
static int test_network_security(void) {
    TEST_START("Network Security");
    
    // Test network security initialization
    int ret = security_init_network_filter();
    TEST_ASSERT(ret == 0, "Network security initialization failed");
    
    // Test packet processing
    uint8_t test_packet[64] = {0};
    ret = net_security_process_packet(test_packet, sizeof(test_packet),
                                     0x7F000001, 0x7F000001, // localhost to localhost
                                     12345, 80, 6, false); // TCP, inbound
    TEST_ASSERT(ret == 0, "Localhost packet should be allowed");
    
    // Test network access checking for process
    process_t test_process = {0};
    test_process.pid = 777;
    test_process.creds.uid = 1002;
    
    ret = security_check_network_access(&test_process, 0x7F000001, 80, "TCP");
    TEST_ASSERT(ret == 0, "Network access check failed");
    
    TEST_PASS();
}

/**
 * Test security integration
 */
static int test_integration(void) {
    TEST_START("Security Integration");
    
    // Test integration initialization
    int ret = security_init_integration();
    TEST_ASSERT(ret == 0, "Security integration initialization failed");
    
    // Test process creation hook
    process_t parent_process = {0};
    process_t child_process = {0};
    parent_process.pid = 100;
    child_process.pid = 101;
    parent_process.creds.capabilities = (1ULL << CAP_FORK);
    
    ret = security_hook_process_create(&parent_process, &child_process);
    TEST_ASSERT(ret == 0, "Process creation hook failed");
    
    // Test file access hook
    ret = security_hook_file_access(&parent_process, "/tmp/test_file", 0x01);
    TEST_ASSERT(ret == 0, "File access hook failed");
    
    // Test system call hook
    ret = security_hook_syscall_enter(&parent_process, 1, NULL); // sys_exit
    TEST_ASSERT(ret == 0, "System call hook failed");
    
    // Test integration statistics
    security_integration_stats_t stats;
    ret = security_get_integration_stats(&stats);
    TEST_ASSERT(ret == 0, "Failed to get integration statistics");
    TEST_ASSERT(stats.initialized == true, "Integration should be initialized");
    TEST_ASSERT(stats.security_checks_performed > 0, "No security checks performed");
    
    TEST_PASS();
}

/**
 * Test security framework performance
 */
static int test_performance(void) {
    TEST_START("Performance Tests");
    
    // Test capability check performance
    uint64_t start_time = get_system_time();
    
    for (int i = 0; i < 1000; i++) {
        security_check_capability(CAP_SYS_ADMIN);
    }
    
    uint64_t end_time = get_system_time();
    uint64_t elapsed = end_time - start_time;
    uint64_t avg_per_check = elapsed / 1000;
    
    kernel_printf("Capability check performance: %llu microseconds average\n", avg_per_check);
    TEST_ASSERT(avg_per_check < 10, "Capability check too slow");
    
    // Test MAC permission check performance
    security_context_t subject_ctx = {0};
    security_context_t object_ctx = {0};
    strcpy(subject_ctx.context, "user_u:user_r:user_t:s0");
    strcpy(object_ctx.context, "system_u:object_r:file_t:s0");
    
    start_time = get_system_time();
    
    for (int i = 0; i < 1000; i++) {
        security_check_permission(&subject_ctx, &object_ctx, "read", 0x01);
    }
    
    end_time = get_system_time();
    elapsed = end_time - start_time;
    avg_per_check = elapsed / 1000;
    
    kernel_printf("MAC permission check performance: %llu microseconds average\n", avg_per_check);
    TEST_ASSERT(avg_per_check < 10, "MAC permission check too slow");
    
    // Test audit logging performance
    security_event_t perf_event = {
        .event_id = 99999,
        .timestamp = get_system_time(),
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 2,
        .blocked = false
    };
    strcpy(perf_event.description, "Performance test event");
    
    start_time = get_system_time();
    
    for (int i = 0; i < 1000; i++) {
        audit_log_event(&perf_event);
    }
    
    end_time = get_system_time();
    elapsed = end_time - start_time;
    avg_per_check = elapsed / 1000;
    
    kernel_printf("Audit logging performance: %llu microseconds average\n", avg_per_check);
    TEST_ASSERT(avg_per_check < 5, "Audit logging too slow");
    
    TEST_PASS();
}

/**
 * Simple memory comparison function
 */
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}