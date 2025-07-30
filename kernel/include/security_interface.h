#ifndef SECURITY_INTERFACE_H
#define SECURITY_INTERFACE_H

/**
 * @file security_interface.h
 * @brief Comprehensive Security Framework Interface for RaeenOS
 * 
 * This interface defines the security subsystem APIs including sandboxing,
 * access control, authentication, encryption, and security policy management
 * for RaeenOS's defense-in-depth security architecture.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "process_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Security API version
#define SECURITY_API_VERSION 1

// Security context limits
#define SECURITY_CONTEXT_MAX    256
#define SECURITY_LABEL_MAX      64
#define SECURITY_KEY_ID_MAX     64
#define SECURITY_RULE_MAX       1024
#define MAX_SECURITY_POLICIES   256
#define MAX_CAPABILITIES        64
#define MAX_SECURITY_CONTEXTS   1024

// Security levels
typedef enum {
    SECURITY_LEVEL_NONE,        // No security enforcement
    SECURITY_LEVEL_BASIC,       // Basic access control
    SECURITY_LEVEL_ENHANCED,    // Enhanced security with MAC
    SECURITY_LEVEL_HIGH,        // High security with strict policies
    SECURITY_LEVEL_MAXIMUM      // Maximum security (lockdown mode)
} security_level_t;

// Access control modes
typedef enum {
    ACCESS_MODE_DISCRETIONARY,  // Discretionary Access Control (DAC)
    ACCESS_MODE_MANDATORY,      // Mandatory Access Control (MAC)  
    ACCESS_MODE_ROLE_BASED,     // Role-Based Access Control (RBAC)
    ACCESS_MODE_ATTRIBUTE_BASED // Attribute-Based Access Control (ABAC)
} access_mode_t;

// Security policy types
typedef enum {
    POLICY_TYPE_SELINUX,        // SELinux-style policy
    POLICY_TYPE_APPARMOR,       // AppArmor-style policy
    POLICY_TYPE_CAPABILITY,     // POSIX capabilities
    POLICY_TYPE_NAMESPACE,      // Namespace isolation
    POLICY_TYPE_SECCOMP,        // System call filtering
    POLICY_TYPE_CUSTOM          // Custom policy
} policy_type_t;

// Capability flags (Linux-compatible)
typedef enum {
    CAP_CHOWN = 0,              // Change file ownership
    CAP_DAC_OVERRIDE,           // Override DAC permissions
    CAP_DAC_READ_SEARCH,        // Override DAC read/search
    CAP_FOWNER,                 // File owner checks
    CAP_FSETID,                 // Set file SUID/SGID
    CAP_KILL,                   // Send signals
    CAP_SETGID,                 // Set GID
    CAP_SETUID,                 // Set UID
    CAP_SETPCAP,                // Set process capabilities
    CAP_LINUX_IMMUTABLE,        // Modify immutable files
    CAP_NET_BIND_SERVICE,       // Bind privileged ports
    CAP_NET_BROADCAST,          // Network broadcast
    CAP_NET_ADMIN,              // Network administration
    CAP_NET_RAW,                // Raw sockets
    CAP_IPC_LOCK,               // Lock memory
    CAP_IPC_OWNER,              // IPC ownership
    CAP_SYS_MODULE,             // Load kernel modules
    CAP_SYS_RAWIO,              // Raw I/O access
    CAP_SYS_CHROOT,             // Use chroot()
    CAP_SYS_PTRACE,             // Trace processes
    CAP_SYS_PACCT,              // Process accounting
    CAP_SYS_ADMIN,              // System administration
    CAP_SYS_BOOT,               // Reboot system
    CAP_SYS_NICE,               // Set process priority
    CAP_SYS_RESOURCE,           // Resource controls
    CAP_SYS_TIME,               // Set system time
    CAP_SYS_TTY_CONFIG,         // TTY configuration
    CAP_MKNOD,                  // Create device files
    CAP_LEASE,                  // File leases
    CAP_AUDIT_WRITE,            // Write audit log
    CAP_AUDIT_CONTROL,          // Configure audit
    CAP_SETFCAP,                // Set file capabilities
    CAP_MAC_OVERRIDE,           // Override MAC policy
    CAP_MAC_ADMIN,              // MAC administration
    CAP_SYSLOG,                 // Syslog access
    CAP_WAKE_ALARM,             // Wake alarms
    CAP_BLOCK_SUSPEND,          // Block system suspend
    CAP_AUDIT_READ,             // Read audit log
    CAP_PERFMON,                // Performance monitoring
    CAP_BPF,                    // BPF operations
    CAP_CHECKPOINT_RESTORE,     // Checkpoint/restore
    // RaeenOS specific capabilities
    CAP_RAEEN_AI_ACCESS,        // AI system access
    CAP_RAEEN_VM_ADMIN,         // VM administration
    CAP_RAEEN_GPU_ACCESS,       // GPU direct access
    CAP_RAEEN_NPU_ACCESS,       // NPU access
    CAP_RAEEN_CRYPTO_ADMIN,     // Cryptographic administration
    CAP_LAST_CAP = CAP_RAEEN_CRYPTO_ADMIN
} capability_t;

// Security context structure
typedef struct security_context {
    char label[SECURITY_LABEL_MAX];         // Security label
    char context[SECURITY_CONTEXT_MAX];     // Full context string
    uint32_t uid;                           // User ID
    uint32_t gid;                           // Group ID
    uint64_t capabilities;                  // Capability mask
    policy_type_t policy_type;              // Policy type
    security_level_t level;                 // Security level
    uint32_t flags;                         // Context flags
    void* private_data;                     // Policy-specific data
} security_context_t;

// Sandbox profile structure
typedef struct sandbox_profile {
    char name[SECURITY_LABEL_MAX];          // Profile name
    uint32_t version;                       // Profile version
    
    // File system access
    struct {
        char** allowed_paths;               // Allowed file paths
        size_t allowed_count;
        char** denied_paths;                // Denied file paths
        size_t denied_count;
        bool allow_network_fs;              // Network filesystems
        bool allow_device_files;            // Device files
        bool allow_suid_files;              // SUID/SGID files
    } filesystem;
    
    // Network access
    struct {
        bool allow_network;                 // Network access
        bool allow_localhost;               // Localhost only
        bool allow_lan;                     // LAN access
        bool allow_internet;                // Internet access
        uint16_t* allowed_ports;            // Allowed ports
        size_t port_count;
        char** allowed_hosts;               // Allowed hostnames
        size_t host_count;
    } network;
    
    // System call filtering
    struct {
        uint64_t* allowed_syscalls;         // Allowed system calls
        size_t syscall_count;
        bool default_deny;                  // Default deny unknown syscalls
    } syscalls;
    
    // Resource limits
    struct {
        uint64_t max_memory;                // Maximum memory usage
        uint32_t max_processes;             // Maximum processes
        uint32_t max_threads;               // Maximum threads
        uint32_t max_files;                 // Maximum open files
        uint64_t max_cpu_time;              // Maximum CPU time
    } limits;
    
    // Hardware access
    struct {
        bool allow_gpu;                     // GPU access
        bool allow_npu;                     // NPU access
        bool allow_audio;                   // Audio devices
        bool allow_camera;                  // Camera access
        bool allow_microphone;              // Microphone access
        bool allow_usb;                     // USB devices
        bool allow_bluetooth;               // Bluetooth access
    } hardware;
    
    // AI system access  
    struct {
        bool allow_ai_inference;            // AI inference
        bool allow_ai_training;             // AI training
        bool allow_model_loading;           // Load AI models
        char** allowed_models;              // Allowed AI models
        size_t model_count;
    } ai;
    
    uint32_t flags;                         // Profile flags
    void* private_data;                     // Implementation-specific data
} sandbox_profile_t;

// Security policy structure
typedef struct security_policy {
    char name[SECURITY_LABEL_MAX];          // Policy name
    uint32_t version;                       // Policy version
    policy_type_t type;                     // Policy type
    security_level_t level;                 // Required security level
    
    // Policy rules
    struct {
        void* rules;                        // Policy-specific rules
        size_t rule_count;                  // Number of rules
        size_t rule_size;                   // Size of each rule
    } rules;
    
    // Enforcement settings
    struct {
        bool enforcing;                     // Enforcing mode
        bool permissive;                    // Permissive mode
        bool audit_only;                    // Audit-only mode
        bool strict_mode;                   // Strict enforcement
    } enforcement;
    
    // Policy metadata
    struct {
        char author[128];                   // Policy author
        char description[256];              // Policy description
        uint64_t created;                   // Creation timestamp
        uint64_t modified;                  // Last modified
        char checksum[64];                  // Policy checksum
    } metadata;
    
    void* private_data;
} security_policy_t;

// Security event structure
typedef struct security_event {
    uint32_t event_id;                      // Event ID
    uint64_t timestamp;                     // Event timestamp
    uint32_t pid;                           // Process ID
    uint32_t uid;                           // User ID
    
    enum {
        SECURITY_EVENT_ACCESS_DENIED,       // Access denied
        SECURITY_EVENT_POLICY_VIOLATION,    // Policy violation
        SECURITY_EVENT_PRIVILEGE_ESCALATION, // Privilege escalation
        SECURITY_EVENT_SUSPICIOUS_ACTIVITY, // Suspicious activity
        SECURITY_EVENT_CRYPTO_OPERATION,    // Cryptographic operation
        SECURITY_EVENT_LOGIN_ATTEMPT,       // Login attempt
        SECURITY_EVENT_SYSTEM_CALL,         // System call audit
        SECURITY_EVENT_FILE_ACCESS,         // File access audit
        SECURITY_EVENT_NETWORK_ACCESS       // Network access audit
    } type;
    
    char description[256];                  // Event description
    char subject[SECURITY_CONTEXT_MAX];     // Subject (process/user)
    char object[SECURITY_CONTEXT_MAX];      // Object (file/resource)
    char action[64];                        // Action attempted
    
    union {
        struct {
            char path[256];                 // File path
            uint32_t access_mode;           // Access mode
        } file;
        
        struct {
            uint32_t address;               // Network address
            uint16_t port;                  // Port number
            char protocol[16];              // Protocol
        } network;
        
        struct {
            uint32_t syscall_number;        // System call number
            void* args[6];                  // System call arguments
        } syscall;
    } data;
    
    uint32_t severity;                      // Event severity (0-10)
    bool blocked;                           // Whether action was blocked
} security_event_t;

// Cryptographic key structure
typedef struct crypto_key {
    char key_id[SECURITY_KEY_ID_MAX];       // Key identifier
    
    enum {
        CRYPTO_KEY_SYMMETRIC,               // Symmetric key
        CRYPTO_KEY_ASYMMETRIC_PUBLIC,       // Public key
        CRYPTO_KEY_ASYMMETRIC_PRIVATE,      // Private key
        CRYPTO_KEY_HMAC,                    // HMAC key
        CRYPTO_KEY_DERIVED                  // Derived key
    } type;
    
    enum {
        CRYPTO_ALG_AES,                     // AES
        CRYPTO_ALG_RSA,                     // RSA
        CRYPTO_ALG_ECDSA,                   // ECDSA
        CRYPTO_ALG_CHACHA20,                // ChaCha20
        CRYPTO_ALG_POLY1305,                // Poly1305
        CRYPTO_ALG_SHA256,                  // SHA-256
        CRYPTO_ALG_SHA3                     // SHA-3
    } algorithm;
    
    size_t key_length;                      // Key length in bits
    void* key_data;                         // Key material
    uint64_t created;                       // Creation timestamp
    uint64_t expires;                       // Expiration timestamp
    uint32_t usage_flags;                   // Usage flags
    
    security_context_t* owner;              // Key owner
    uint32_t ref_count;                     // Reference count
    
    void* private_data;
} crypto_key_t;

// Security operations structure
typedef struct security_operations {
    // Initialization
    int (*init)(void);
    void (*cleanup)(void);
    
    // Security context management
    int (*create_context)(const char* label, security_context_t** context);
    void (*destroy_context)(security_context_t* context);
    int (*set_context)(process_t* process, security_context_t* context);
    security_context_t* (*get_context)(process_t* process);
    int (*check_context)(security_context_t* context);
    
    // Access control
    int (*check_permission)(security_context_t* subject, security_context_t* object, 
                           const char* action, uint32_t requested_access);
    int (*check_capability)(process_t* process, capability_t capability);
    int (*grant_capability)(process_t* process, capability_t capability);
    int (*revoke_capability)(process_t* process, capability_t capability);
    
    // Sandbox management
    int (*create_sandbox)(const char* name, sandbox_profile_t** profile);
    void (*destroy_sandbox)(sandbox_profile_t* profile);
    int (*apply_sandbox)(process_t* process, sandbox_profile_t* profile);
    int (*check_sandbox_access)(process_t* process, const char* resource, 
                                const char* action);
    
    // Policy management
    int (*load_policy)(const char* policy_path, security_policy_t** policy);
    int (*unload_policy)(security_policy_t* policy);
    int (*set_policy)(security_policy_t* policy);
    security_policy_t* (*get_policy)(void);
    int (*check_policy)(security_context_t* subject, security_context_t* object,
                       const char* action);
    
    // Authentication
    int (*authenticate_user)(const char* username, const char* credential,
                            security_context_t** context);
    int (*verify_signature)(const void* data, size_t data_len, 
                           const void* signature, size_t sig_len,
                           crypto_key_t* key);
    int (*create_session)(security_context_t* context, void** session);
    int (*validate_session)(void* session);
    void (*destroy_session)(void* session);
    
    // Cryptographic operations
    int (*generate_key)(int algorithm, size_t key_length, crypto_key_t** key);
    int (*derive_key)(crypto_key_t* master, const void* info, size_t info_len,
                     crypto_key_t** derived);
    int (*encrypt_data)(crypto_key_t* key, const void* plaintext, size_t len,
                       void** ciphertext, size_t* cipher_len);
    int (*decrypt_data)(crypto_key_t* key, const void* ciphertext, size_t len,
                       void** plaintext, size_t* plain_len);
    int (*sign_data)(crypto_key_t* key, const void* data, size_t data_len,
                    void** signature, size_t* sig_len);
    int (*verify_data)(crypto_key_t* key, const void* data, size_t data_len,
                      const void* signature, size_t sig_len);
    
    // Secure storage
    int (*store_key)(crypto_key_t* key, const char* storage_id);
    int (*retrieve_key)(const char* storage_id, crypto_key_t** key);
    int (*delete_key)(const char* storage_id);
    int (*list_keys)(char*** key_ids, size_t* count);
    
    // Audit and logging
    int (*log_security_event)(security_event_t* event);
    int (*get_security_events)(security_event_t** events, size_t* count,
                              uint64_t since, uint32_t event_type);
    int (*set_audit_policy)(uint32_t event_mask, bool enable);
    
    // Intrusion detection
    int (*register_ids_rule)(const char* rule, void (*callback)(security_event_t*));
    int (*unregister_ids_rule)(const char* rule);
    int (*analyze_behavior)(process_t* process, uint32_t* threat_score);
    
    // Secure boot and integrity
    int (*verify_boot_integrity)(void);
    int (*verify_module_integrity)(const char* module_path);
    int (*verify_file_integrity)(const char* file_path, const char* expected_hash);
    int (*sign_module)(const char* module_path, crypto_key_t* key);
    
    // Hardware security
    int (*init_tpm)(void);
    int (*tpm_seal_data)(const void* data, size_t len, void** sealed, size_t* sealed_len);
    int (*tpm_unseal_data)(const void* sealed, size_t sealed_len, void** data, size_t* len);
    int (*get_hardware_random)(void* buffer, size_t len);
    
    // Secure memory
    void* (*alloc_secure_memory)(size_t size);
    void (*free_secure_memory)(void* ptr, size_t size);
    int (*lock_memory)(void* ptr, size_t size);
    int (*unlock_memory)(void* ptr, size_t size);
    
    // Security information
    int (*get_security_level)(security_level_t* level);
    int (*set_security_level)(security_level_t level);
    int (*get_security_stats)(void* stats);
} security_ops_t;

// Global security operations
extern security_ops_t* security;

// Security API functions

// Initialization
int security_init(void);
void security_cleanup(void);

// Process security
int security_check_permission(const char* action, const char* object);
int security_check_capability(capability_t capability);
int security_apply_sandbox(const char* profile_name);

// Authentication
int security_login(const char* username, const char* password);
int security_logout(void);
bool security_is_authenticated(void);

// Cryptographic functions
int crypto_encrypt(const char* key_id, const void* data, size_t len, 
                  void** encrypted, size_t* encrypted_len);
int crypto_decrypt(const char* key_id, const void* encrypted, size_t len,
                  void** data, size_t* data_len);
int crypto_sign(const char* key_id, const void* data, size_t len,
               void** signature, size_t* sig_len);
int crypto_verify(const char* key_id, const void* data, size_t len,
                 const void* signature, size_t sig_len);

// Secure random number generation
int secure_random(void* buffer, size_t len);

// Security event logging
int security_log_event(uint32_t type, const char* description);

// Utility macros
#define CAPABILITY_SET(caps, cap)    ((caps) |= (1ULL << (cap)))
#define CAPABILITY_CLEAR(caps, cap)  ((caps) &= ~(1ULL << (cap)))
#define CAPABILITY_TEST(caps, cap)   (((caps) & (1ULL << (cap))) != 0)

#define SECURITY_CHECK(action, object) \
    security_check_permission(action, object)

#define CAPABILITY_CHECK(cap) \
    security_check_capability(cap)

// Common capability combinations
#define CAP_ADMIN_SET   ((1ULL << CAP_SYS_ADMIN) | (1ULL << CAP_DAC_OVERRIDE))
#define CAP_NETWORK_SET ((1ULL << CAP_NET_ADMIN) | (1ULL << CAP_NET_BIND_SERVICE))
#define CAP_FILE_SET    ((1ULL << CAP_CHOWN) | (1ULL << CAP_FOWNER))

// Security flags
#define SECURITY_FLAG_ENFORCING     (1 << 0)
#define SECURITY_FLAG_PERMISSIVE    (1 << 1)
#define SECURITY_FLAG_AUDIT         (1 << 2)
#define SECURITY_FLAG_STRICT        (1 << 3)

#ifdef __cplusplus
}
#endif

#endif // SECURITY_INTERFACE_H