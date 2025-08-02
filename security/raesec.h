/**
 * RaeenOS Security Framework (RaeSec)
 * Comprehensive security system with sandboxing, capabilities, and access controls
 */

#ifndef RAESEC_H
#define RAESEC_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

// Security context types
typedef enum {
    SEC_CONTEXT_SYSTEM,      // System/kernel context
    SEC_CONTEXT_ADMIN,       // Administrative context
    SEC_CONTEXT_USER,        // User application context
    SEC_CONTEXT_SANDBOX,     // Sandboxed application
    SEC_CONTEXT_RESTRICTED,  // Highly restricted context
    SEC_CONTEXT_UNTRUSTED    // Untrusted/guest context
} SecurityContextType;

// Capability types
typedef enum {
    CAP_FILESYSTEM_READ,
    CAP_FILESYSTEM_WRITE,
    CAP_FILESYSTEM_EXECUTE,
    CAP_NETWORK_ACCESS,
    CAP_NETWORK_BIND,
    CAP_NETWORK_LISTEN,
    CAP_HARDWARE_CAMERA,
    CAP_HARDWARE_MICROPHONE,
    CAP_HARDWARE_LOCATION,
    CAP_HARDWARE_BLUETOOTH,
    CAP_HARDWARE_USB,
    CAP_SYSTEM_ADMIN,
    CAP_SYSTEM_REBOOT,
    CAP_SYSTEM_TIME,
    CAP_PROCESS_CREATE,
    CAP_PROCESS_KILL,
    CAP_PROCESS_DEBUG,
    CAP_MEMORY_EXECUTE,
    CAP_MEMORY_MMAP,
    CAP_IPC_SHARED_MEMORY,
    CAP_IPC_SOCKETS,
    CAP_GRAPHICS_DISPLAY,
    CAP_GRAPHICS_CAPTURE,
    CAP_AUDIO_PLAYBACK,
    CAP_AUDIO_RECORD,
    CAP_MAX_CAPABILITY
} CapabilityType;

// Access control modes
typedef enum {
    ACCESS_DENY,
    ACCESS_ALLOW,
    ACCESS_PROMPT,
    ACCESS_CONDITIONAL
} AccessMode;

// Sandbox types
typedef enum {
    SANDBOX_NONE,
    SANDBOX_BASIC,
    SANDBOX_STRICT,
    SANDBOX_ISOLATED,
    SANDBOX_VIRTUAL
} SandboxType;

// Security policy actions
typedef enum {
    POLICY_ACTION_ALLOW,
    POLICY_ACTION_DENY,
    POLICY_ACTION_LOG,
    POLICY_ACTION_PROMPT,
    POLICY_ACTION_QUARANTINE
} PolicyAction;

// Threat levels
typedef enum {
    THREAT_LEVEL_NONE,
    THREAT_LEVEL_LOW,
    THREAT_LEVEL_MEDIUM,
    THREAT_LEVEL_HIGH,
    THREAT_LEVEL_CRITICAL
} ThreatLevel;

// Capability descriptor
typedef struct {
    CapabilityType type;
    char resource_path[512];
    AccessMode mode;
    time_t granted_time;
    time_t expiry_time;
    uint32_t usage_count;
    uint32_t max_usage;
    bool is_inherited;
    bool is_transferable;
} Capability;

// Security context
typedef struct {
    uint64_t context_id;
    SecurityContextType type;
    uint32_t process_id;
    uint32_t user_id;
    uint32_t group_id;
    
    // Capabilities
    Capability* capabilities;
    uint32_t capability_count;
    uint32_t capability_capacity;
    
    // Sandbox configuration
    SandboxType sandbox_type;
    char sandbox_root[512];
    char* allowed_paths;
    uint32_t allowed_path_count;
    char* denied_paths;
    uint32_t denied_path_count;
    
    // Resource limits
    uint64_t max_memory;
    uint64_t max_file_size;
    uint32_t max_open_files;
    uint32_t max_network_connections;
    uint32_t max_child_processes;
    
    // Security attributes
    char security_label[128];
    char integrity_level[64];
    ThreatLevel threat_level;
    bool is_trusted;
    bool allow_privilege_escalation;
    
    // Audit information
    time_t creation_time;
    time_t last_access_time;
    uint64_t access_count;
    char creator_process[256];
    
    pthread_mutex_t context_mutex;
} SecurityContext;

// Security policy rule
typedef struct {
    uint64_t rule_id;
    char name[128];
    char description[256];
    
    // Matching criteria
    char process_pattern[256];
    char user_pattern[64];
    char path_pattern[512];
    CapabilityType capability;
    SecurityContextType context_type;
    
    // Action
    PolicyAction action;
    char custom_message[256];
    
    // Conditions
    time_t start_time;
    time_t end_time;
    uint32_t max_violations;
    uint32_t current_violations;
    
    // Metadata
    bool enabled;
    uint32_t priority;
    time_t created_time;
    time_t modified_time;
    char creator[128];
    
    struct SecurityPolicyRule* next;
} SecurityPolicyRule;

// Security event
typedef struct {
    uint64_t event_id;
    time_t timestamp;
    SecurityContextType source_context;
    uint32_t source_pid;
    CapabilityType capability;
    char resource_path[512];
    PolicyAction action_taken;
    ThreatLevel threat_level;
    char description[512];
    bool blocked;
} SecurityEvent;

// Sandbox configuration
typedef struct {
    SandboxType type;
    char name[128];
    char root_directory[512];
    
    // Filesystem restrictions
    char** allowed_read_paths;
    char** allowed_write_paths;
    char** allowed_execute_paths;
    char** denied_paths;
    uint32_t allowed_read_count;
    uint32_t allowed_write_count;
    uint32_t allowed_execute_count;
    uint32_t denied_count;
    
    // Network restrictions
    bool allow_network;
    char** allowed_hosts;
    uint16_t* allowed_ports;
    uint32_t allowed_host_count;
    uint32_t allowed_port_count;
    
    // System call filtering
    uint32_t* allowed_syscalls;
    uint32_t* denied_syscalls;
    uint32_t allowed_syscall_count;
    uint32_t denied_syscall_count;
    
    // Resource limits
    uint64_t max_memory;
    uint64_t max_disk_usage;
    uint32_t max_processes;
    uint32_t max_threads;
    uint32_t cpu_quota_percent;
    
    // Security options
    bool no_new_privileges;
    bool disable_ptrace;
    bool readonly_root;
    bool private_tmp;
    bool private_network;
    
    pthread_mutex_t config_mutex;
} SandboxConfig;

// Access control entry
typedef struct {
    char subject[128];        // User, group, or process
    char object[512];         // Resource path or identifier
    CapabilityType capability;
    AccessMode mode;
    char conditions[256];     // Additional conditions
    time_t granted_time;
    time_t expiry_time;
    uint32_t usage_count;
    struct AccessControlEntry* next;
} AccessControlEntry;

// Security audit log
typedef struct {
    SecurityEvent* events;
    uint32_t event_count;
    uint32_t event_capacity;
    uint64_t next_event_id;
    
    // Log configuration
    char log_file_path[512];
    uint32_t max_log_size;
    uint32_t max_log_files;
    bool log_to_syslog;
    bool log_to_file;
    
    // Filtering
    ThreatLevel min_log_level;
    bool log_allowed_actions;
    bool log_denied_actions;
    
    pthread_mutex_t log_mutex;
} SecurityAuditLog;

// Main security framework context
typedef struct {
    // Security contexts
    SecurityContext** contexts;
    uint32_t context_count;
    uint32_t context_capacity;
    uint64_t next_context_id;
    
    // Security policies
    SecurityPolicyRule* policy_rules;
    uint32_t rule_count;
    char policy_file_path[512];
    
    // Sandbox configurations
    SandboxConfig* sandbox_configs;
    uint32_t sandbox_config_count;
    uint32_t sandbox_config_capacity;
    
    // Access control
    AccessControlEntry* acl_entries;
    uint32_t acl_count;
    
    // Audit logging
    SecurityAuditLog* audit_log;
    
    // Configuration
    char config_dir[512];
    char policy_dir[512];
    char log_dir[512];
    bool enforcement_enabled;
    bool learning_mode;
    bool paranoid_mode;
    ThreatLevel default_threat_level;
    
    // Statistics
    uint64_t total_access_checks;
    uint64_t allowed_accesses;
    uint64_t denied_accesses;
    uint64_t prompted_accesses;
    uint64_t security_violations;
    time_t last_policy_update;
    
    // Synchronization
    pthread_mutex_t framework_mutex;
    pthread_rwlock_t policy_rwlock;
    bool is_initialized;
} SecurityFramework;

// Cryptographic context
typedef struct {
    char algorithm[64];
    uint8_t* key_data;
    uint32_t key_length;
    uint8_t* iv_data;
    uint32_t iv_length;
    void* crypto_context;
    bool is_initialized;
} CryptoContext;

// Code signing verification
typedef struct {
    char signature_algorithm[64];
    uint8_t* signature_data;
    uint32_t signature_length;
    char signer_certificate[512];
    char trust_chain[1024];
    time_t signature_time;
    bool is_valid;
    bool is_trusted;
} CodeSignature;

// Function declarations

// Core security framework
SecurityFramework* raesec_init(const char* config_dir);
void raesec_shutdown(SecurityFramework* framework);
bool raesec_load_config(SecurityFramework* framework, const char* config_file);
bool raesec_save_config(SecurityFramework* framework, const char* config_file);

// Security context management
SecurityContext* raesec_create_context(SecurityFramework* framework, SecurityContextType type, uint32_t process_id);
void raesec_destroy_context(SecurityFramework* framework, SecurityContext* context);
SecurityContext* raesec_get_context(SecurityFramework* framework, uint64_t context_id);
SecurityContext* raesec_get_process_context(SecurityFramework* framework, uint32_t process_id);
bool raesec_switch_context(SecurityFramework* framework, SecurityContext* new_context);

// Capability management
bool raesec_grant_capability(SecurityContext* context, CapabilityType capability, const char* resource_path, AccessMode mode);
bool raesec_revoke_capability(SecurityContext* context, CapabilityType capability, const char* resource_path);
bool raesec_check_capability(SecurityContext* context, CapabilityType capability, const char* resource_path);
bool raesec_transfer_capability(SecurityContext* from_context, SecurityContext* to_context, CapabilityType capability);

// Access control
bool raesec_check_access(SecurityFramework* framework, SecurityContext* context, const char* resource_path, CapabilityType capability);
bool raesec_request_access(SecurityFramework* framework, SecurityContext* context, const char* resource_path, CapabilityType capability);
bool raesec_add_acl_entry(SecurityFramework* framework, const char* subject, const char* object, CapabilityType capability, AccessMode mode);
bool raesec_remove_acl_entry(SecurityFramework* framework, const char* subject, const char* object, CapabilityType capability);

// Sandbox management
SandboxConfig* raesec_create_sandbox_config(const char* name, SandboxType type);
void raesec_destroy_sandbox_config(SandboxConfig* config);
bool raesec_apply_sandbox(SecurityFramework* framework, SecurityContext* context, SandboxConfig* config);
bool raesec_escape_sandbox(SecurityFramework* framework, SecurityContext* context);

// Policy management
bool raesec_load_policies(SecurityFramework* framework, const char* policy_dir);
bool raesec_save_policies(SecurityFramework* framework, const char* policy_dir);
bool raesec_add_policy_rule(SecurityFramework* framework, SecurityPolicyRule* rule);
bool raesec_remove_policy_rule(SecurityFramework* framework, uint64_t rule_id);
bool raesec_evaluate_policy(SecurityFramework* framework, SecurityContext* context, const char* resource_path, CapabilityType capability, PolicyAction* action);

// Audit and logging
bool raesec_log_security_event(SecurityFramework* framework, SecurityContext* context, CapabilityType capability, const char* resource_path, PolicyAction action, const char* description);
SecurityEvent** raesec_query_security_events(SecurityFramework* framework, time_t start_time, time_t end_time, uint32_t* event_count);
bool raesec_export_audit_log(SecurityFramework* framework, const char* file_path, time_t start_time, time_t end_time);

// Cryptographic functions
CryptoContext* raesec_create_crypto_context(const char* algorithm, const uint8_t* key, uint32_t key_length);
void raesec_destroy_crypto_context(CryptoContext* context);
bool raesec_encrypt_data(CryptoContext* context, const uint8_t* plaintext, uint32_t plaintext_length, uint8_t** ciphertext, uint32_t* ciphertext_length);
bool raesec_decrypt_data(CryptoContext* context, const uint8_t* ciphertext, uint32_t ciphertext_length, uint8_t** plaintext, uint32_t* plaintext_length);
bool raesec_hash_data(const uint8_t* data, uint32_t data_length, const char* algorithm, uint8_t** hash, uint32_t* hash_length);

// Code signing and verification
bool raesec_verify_code_signature(const char* file_path, CodeSignature** signature);
bool raesec_sign_code(const char* file_path, const char* key_file, const char* cert_file);
bool raesec_verify_trust_chain(const char* certificate, const char* trusted_ca_dir);

// System integration
bool raesec_hook_syscalls(SecurityFramework* framework);
bool raesec_unhook_syscalls(SecurityFramework* framework);
bool raesec_intercept_file_access(const char* path, CapabilityType capability, SecurityContext* context);
bool raesec_intercept_network_access(const char* host, uint16_t port, SecurityContext* context);

// Threat detection
ThreatLevel raesec_assess_threat_level(SecurityFramework* framework, SecurityContext* context, const char* resource_path, CapabilityType capability);
bool raesec_detect_anomaly(SecurityFramework* framework, SecurityContext* context, const char* activity_description);
bool raesec_quarantine_process(SecurityFramework* framework, uint32_t process_id, const char* reason);

// Configuration and utilities
bool raesec_set_enforcement_mode(SecurityFramework* framework, bool enabled);
bool raesec_set_learning_mode(SecurityFramework* framework, bool enabled);
bool raesec_set_paranoid_mode(SecurityFramework* framework, bool enabled);
void raesec_get_statistics(SecurityFramework* framework, uint64_t* total_checks, uint64_t* allowed, uint64_t* denied, uint64_t* violations);
void raesec_print_statistics(SecurityFramework* framework);

// Utility functions
const char* raesec_capability_to_string(CapabilityType capability);
const char* raesec_context_type_to_string(SecurityContextType type);
const char* raesec_access_mode_to_string(AccessMode mode);
const char* raesec_threat_level_to_string(ThreatLevel level);
const char* raesec_policy_action_to_string(PolicyAction action);
bool raesec_parse_capability(const char* capability_str, CapabilityType* capability);
bool raesec_parse_access_mode(const char* mode_str, AccessMode* mode);

#endif // RAESEC_H
