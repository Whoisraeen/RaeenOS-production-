/**
 * @file security_core.h
 * @brief Core Security Framework Header for RaeenOS
 * 
 * Internal header for the security framework implementation.
 * This file contains private structures and functions used by
 * the security subsystem components.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#ifndef SECURITY_CORE_H
#define SECURITY_CORE_H

#include "../include/security_interface.h"
#include "../include/types.h"

// Security framework limits
#define SECURITY_AUDIT_BUFFER_SIZE      8192
#define MAX_SECURITY_RULES              4096
#define MAX_IDS_RULES                   512
#define SECURITY_HASH_TABLE_SIZE        256

// Security framework structure
typedef struct security_framework {
    uint32_t version;                   // Framework version
    bool initialized;                   // Initialization state
    security_level_t level;             // Current security level
    
    // Statistics
    struct {
        uint64_t access_checks;         // Total access checks
        uint64_t access_denied;         // Access denials
        uint64_t policy_violations;     // Policy violations
        uint64_t capability_checks;     // Capability checks
        uint64_t events_logged;         // Events logged
        uint64_t intrusions_detected;   // Intrusions detected
    } stats;
    
    // Runtime configuration
    struct {
        bool audit_enabled;             // Audit logging enabled
        bool ids_enabled;               // IDS enabled
        bool crypto_acceleration;       // Hardware crypto acceleration
        bool secure_boot_required;     // Secure boot enforcement
        uint32_t max_failed_attempts;  // Max failed auth attempts
        uint32_t lockout_duration;      // Account lockout duration
    } config;
} security_framework_t;

// MAC (Mandatory Access Control) structures
typedef struct mac_rule {
    char subject_type[64];              // Subject type pattern
    char object_type[64];               // Object type pattern
    char action[32];                    // Action
    bool allow;                         // Allow or deny
    uint32_t conditions;                // Additional conditions
} mac_rule_t;

typedef struct mac_policy {
    char name[64];                      // Policy name
    mac_rule_t* rules;                  // Policy rules
    size_t rule_count;                  // Number of rules
    bool enforcing;                     // Enforcing mode
} mac_policy_t;

// Sandbox profile structures
typedef struct sandbox_namespace {
    uint32_t type;                      // Namespace type
    char* path;                         // Mount path
    uint32_t flags;                     // Mount flags
} sandbox_namespace_t;

typedef struct sandbox_resource_limit {
    uint32_t resource;                  // Resource type
    uint64_t limit;                     // Resource limit
} sandbox_resource_limit_t;

// Intrusion Detection System structures
typedef struct ids_rule {
    uint32_t id;                        // Rule ID
    char pattern[256];                  // Pattern to match
    uint32_t event_types;               // Event types to monitor
    uint32_t threshold;                 // Threshold for triggering
    uint32_t window;                    // Time window (seconds)
    void (*callback)(security_event_t*); // Callback function
    uint32_t matches;                   // Current match count
    uint64_t last_match;                // Last match timestamp
} ids_rule_t;

// Cryptographic key store entry
typedef struct key_store_entry {
    char key_id[SECURITY_KEY_ID_MAX];   // Key identifier
    crypto_key_t* key;                  // Key data
    uint32_t access_count;              // Access count
    uint64_t last_access;               // Last access time
    struct key_store_entry* next;       // Hash table chaining
} key_store_entry_t;

// Hardware security features
typedef struct hardware_security {
    bool tpm_available;                 // TPM 2.0 available
    bool aes_ni_available;              // AES-NI support
    bool rdrand_available;              // Hardware RNG available
    bool smap_available;                // SMAP support
    bool smep_available;                // SMEP support
    bool cet_available;                 // Intel CET support
    bool mte_available;                 // ARM MTE support
    bool pauth_available;               // ARM Pointer Auth support
} hardware_security_t;

// Function declarations

// Core security functions
extern security_framework_t security_framework;
int security_init_hardware(void);
int security_init_memory_protection(void);
int security_load_default_policies(void);
void security_enable_enhanced_features(void);
bool security_verify_elevated_capability(process_t* process, capability_t capability);
uint64_t security_get_required_capabilities(security_level_t level);
int security_parse_context_label(const char* label, security_context_t* context);

// MAC (Mandatory Access Control) functions
int mac_init(void);
void mac_cleanup(void);
int mac_check_permission(security_context_t* subject, security_context_t* object, 
                        const char* action, uint32_t requested_access);
int mac_load_policy(const char* policy_path);
int mac_set_enforcing(bool enforcing);

// Capability management functions
int capability_init(void);
void capability_cleanup(void);
bool capability_is_valid(capability_t capability);
const char* capability_to_string(capability_t capability);

// Sandboxing functions
int sandbox_init(void);
void sandbox_cleanup(void);
int security_create_sandbox(const char* name, sandbox_profile_t** profile);
void security_destroy_sandbox(sandbox_profile_t* profile);
int security_apply_sandbox(process_t* process, sandbox_profile_t* profile);
int security_check_sandbox_access(process_t* process, const char* resource, const char* action);

// Policy management functions
int security_load_policy(const char* policy_path, security_policy_t** policy);
int security_unload_policy(security_policy_t* policy);
int security_set_policy(security_policy_t* policy);
security_policy_t* security_get_policy(void);
int security_check_policy(security_context_t* subject, security_context_t* object, const char* action);

// Cryptographic functions
int crypto_init(void);
void crypto_cleanup(void);
int crypto_generate_random(void* buffer, size_t len);
int crypto_hash_data(const void* data, size_t len, uint32_t algorithm, void* hash);

// Audit functions
int audit_init(void);
void audit_cleanup(void);
int audit_log_event(security_event_t* event);
int security_get_events(security_event_t** events, size_t* count, uint64_t since, uint32_t event_type);
int security_set_audit_policy(uint32_t event_mask, bool enable);

// Intrusion Detection System functions
int ids_init(void);
void ids_cleanup(void);
int ids_analyze_event(security_event_t* event);
int ids_register_rule(const char* pattern, void (*callback)(security_event_t*));
int ids_unregister_rule(const char* pattern);

// Secure boot and integrity functions
int security_verify_boot_integrity(void);
int security_verify_module_integrity(const char* module_path);
int security_verify_file_integrity(const char* file_path, const char* expected_hash);

// Hardware security functions
int security_init_tpm(void);
int security_tpm_seal_data(const void* data, size_t len, void** sealed, size_t* sealed_len);
int security_tpm_unseal_data(const void* sealed, size_t sealed_len, void** data, size_t* len);
int security_get_hardware_random(void* buffer, size_t len);

// Secure memory functions
void* security_alloc_secure_memory(size_t size);
void security_free_secure_memory(void* ptr, size_t size);
int security_lock_memory(void* ptr, size_t size);
int security_unlock_memory(void* ptr, size_t size);

// Utility functions
uint64_t get_system_time(void);
void kernel_printf(const char* format, ...);
process_t* current_process(void);

// Security event helpers
static uint32_t security_generate_event_id(void);
static const char* security_level_to_string(security_level_t level);
static capability_t security_action_to_capability(const char* action);

// Memory protection functions
int security_enable_aslr(void);
int security_enable_stack_protection(void);
int security_enable_heap_protection(void);
int security_enable_cfi(void);

// Network security functions
int security_init_network_filter(void);
int security_check_network_access(process_t* process, uint32_t addr, uint16_t port, const char* protocol);

// Error codes specific to security framework
#define ESECURITY_INVALID_CONTEXT   200
#define ESECURITY_POLICY_VIOLATION  201
#define ESECURITY_CAPABILITY_DENIED 202
#define ESECURITY_SANDBOX_VIOLATION 203
#define ESECURITY_INTRUSION_DETECTED 204
#define ESECURITY_CRYPTO_ERROR      205
#define ESECURITY_TPM_ERROR         206
#define ESECURITY_INTEGRITY_FAILED  207

// Security flags
#define SECURITY_FLAG_AUDIT_ALL     (1 << 0)
#define SECURITY_FLAG_STRICT_MODE   (1 << 1)
#define SECURITY_FLAG_PARANOID      (1 << 2)
#define SECURITY_FLAG_LOCKDOWN      (1 << 3)

// Audit event masks
#define AUDIT_MASK_FILE_ACCESS      (1 << 0)
#define AUDIT_MASK_NETWORK_ACCESS   (1 << 1)
#define AUDIT_MASK_PROCESS_CREATE   (1 << 2)
#define AUDIT_MASK_CAPABILITY_USE   (1 << 3)
#define AUDIT_MASK_POLICY_CHANGE    (1 << 4)
#define AUDIT_MASK_LOGIN_ATTEMPT    (1 << 5)
#define AUDIT_MASK_CRYPTO_OPERATION (1 << 6)
#define AUDIT_MASK_ALL              (~0U)

#endif // SECURITY_CORE_H