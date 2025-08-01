/**
 * @file security_core.c
 * @brief Core Security Framework Implementation for RaeenOS
 * 
 * This module implements the foundational security framework including:
 * - Mandatory Access Control (MAC) framework
 * - Capability-based security model
 * - Security context management
 * - Security policy engine
 * - Defense-in-depth protection mechanisms
 * 
 * RaeenOS Security Architecture:
 * - Zero-trust default policies
 * - Hardware-accelerated security features
 * - Real-time threat detection and prevention
 * - Transparent user control over security decisions
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../include/security_interface.h"
#include "../include/process_interface.h"
#include "../include/memory_interface.h"
#include "../include/hal_interface.h"
#include "../kernel.h"
#include "../memory.h"
#include "../string.h"

// Global security state
static security_framework_t security_framework = {0};
static security_ops_t security_operations;
static bool security_initialized = false;

// Security policy database
static security_policy_t* loaded_policies[MAX_SECURITY_POLICIES];
static size_t policy_count = 0;

// Security context database
static security_context_t* security_contexts[MAX_SECURITY_CONTEXTS];
static size_t context_count = 0;

// Security audit buffer
static security_event_t audit_buffer[SECURITY_AUDIT_BUFFER_SIZE];
static size_t audit_head = 0;
static size_t audit_tail = 0;

// Current security level
static security_level_t current_security_level = SECURITY_LEVEL_ENHANCED;

// MAC enforcement state
static bool mac_enforcing = true;
static bool mac_permissive = false;

/**
 * Initialize the security framework
 */
int security_init(void) {
    if (security_initialized) {
        return -EALREADY;
    }
    
    // Initialize security framework structure
    memset(&security_framework, 0, sizeof(security_framework_t));
    security_framework.version = SECURITY_API_VERSION;
    security_framework.initialized = false;
    
    // Initialize policy database
    memset(loaded_policies, 0, sizeof(loaded_policies));
    policy_count = 0;
    
    // Initialize context database
    memset(security_contexts, 0, sizeof(security_contexts));
    context_count = 0;
    
    // Initialize audit buffer
    memset(audit_buffer, 0, sizeof(audit_buffer));
    audit_head = 0;
    audit_tail = 0;
    
    // Initialize hardware security features
    int ret = security_init_hardware();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize hardware security: %d\n", ret);
        return ret;
    }
    
    // Initialize cryptographic subsystem
    ret = crypto_init();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize crypto subsystem: %d\n", ret);
        return ret;
    }
    
    // Initialize memory protection
    ret = security_init_memory_protection();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize memory protection: %d\n", ret);
        return ret;
    }
    
    // Initialize MAC framework
    ret = mac_init();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize MAC framework: %d\n", ret);
        return ret;
    }
    
    // Initialize capability framework
    ret = capability_init();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize capability framework: %d\n", ret);
        return ret;
    }
    
    // Initialize sandboxing framework
    ret = sandbox_init();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize sandboxing: %d\n", ret);
        return ret;
    }
    
    // Initialize audit framework
    ret = audit_init();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize audit framework: %d\n", ret);
        return ret;
    }
    
    // Initialize intrusion detection
    ret = ids_init();
    if (ret != 0) {
        kernel_printf("Security: Failed to initialize IDS: %d\n", ret);
        return ret;
    }
    
    // Setup security operations
    security_operations.init = security_init;
    security_operations.cleanup = security_cleanup;
    security_operations.create_context = security_create_context;
    security_operations.destroy_context = security_destroy_context;
    security_operations.set_context = security_set_context;
    security_operations.get_context = security_get_context;
    security_operations.check_context = security_check_context;
    security_operations.check_permission = security_check_permission;
    security_operations.check_capability = security_check_capability;
    security_operations.grant_capability = security_grant_capability;
    security_operations.revoke_capability = security_revoke_capability;
    security_operations.create_sandbox = security_create_sandbox;
    security_operations.destroy_sandbox = security_destroy_sandbox;
    security_operations.apply_sandbox = security_apply_sandbox;
    security_operations.check_sandbox_access = security_check_sandbox_access;
    security_operations.load_policy = security_load_policy;
    security_operations.unload_policy = security_unload_policy;
    security_operations.set_policy = security_set_policy;
    security_operations.get_policy = security_get_policy;
    security_operations.check_policy = security_check_policy;
    security_operations.log_security_event = security_log_event;
    security_operations.get_security_events = security_get_events;
    security_operations.set_audit_policy = security_set_audit_policy;
    security_operations.verify_boot_integrity = security_verify_boot_integrity;
    security_operations.init_tpm = security_init_tpm;
    security_operations.get_hardware_random = security_get_hardware_random;
    security_operations.alloc_secure_memory = security_alloc_secure_memory;
    security_operations.free_secure_memory = security_free_secure_memory;
    security_operations.get_security_level = security_get_level;
    security_operations.set_security_level = security_set_level;
    
    // Set global security operations pointer
    security = &security_operations;
    
    // Create default security context for kernel
    security_context_t* kernel_context;
    ret = security_create_context("kernel:kernel:s0", &kernel_context);
    if (ret != 0) {
        kernel_printf("Security: Failed to create kernel context: %d\n", ret);
        return ret;
    }
    
    // Grant all capabilities to kernel context
    kernel_context->capabilities = ~0ULL;
    kernel_context->level = SECURITY_LEVEL_MAXIMUM;
    
    // Load default security policies
    ret = security_load_default_policies();
    if (ret != 0) {
        kernel_printf("Security: Failed to load default policies: %d\n", ret);
        return ret;
    }
    
    // Enable security framework
    security_framework.initialized = true;
    security_initialized = true;
    
    kernel_printf("Security: Framework initialized successfully\n");
    kernel_printf("Security: Level: %s, MAC: %s\n", 
                  security_level_to_string(current_security_level),
                  mac_enforcing ? "Enforcing" : "Permissive");
    
    return 0;
}

/**
 * Cleanup security framework
 */
void security_cleanup(void) {
    if (!security_initialized) {
        return;
    }
    
    // Cleanup loaded policies
    for (size_t i = 0; i < policy_count; i++) {
        if (loaded_policies[i]) {
            security_unload_policy(loaded_policies[i]);
        }
    }
    
    // Cleanup security contexts
    for (size_t i = 0; i < context_count; i++) {
        if (security_contexts[i]) {
            security_destroy_context(security_contexts[i]);
        }
    }
    
    // Cleanup subsystems
    ids_cleanup();
    audit_cleanup();
    sandbox_cleanup();
    capability_cleanup();
    mac_cleanup();
    crypto_cleanup();
    
    security_initialized = false;
    security_framework.initialized = false;
    
    kernel_printf("Security: Framework cleaned up\n");
}

/**
 * Create a new security context
 */
int security_create_context(const char* label, security_context_t** context) {
    if (!security_initialized || !label || !context) {
        return -EINVAL;
    }
    
    if (context_count >= MAX_SECURITY_CONTEXTS) {
        return -ENOMEM;
    }
    
    // Allocate context
    security_context_t* ctx = kmalloc(sizeof(security_context_t));
    if (!ctx) {
        return -ENOMEM;
    }
    
    memset(ctx, 0, sizeof(security_context_t));
    
    // Parse and validate label
    int ret = security_parse_context_label(label, ctx);
    if (ret != 0) {
        kfree(ctx);
        return ret;
    }
    
    // Set default values
    ctx->level = SECURITY_LEVEL_BASIC;
    ctx->capabilities = 0;
    ctx->flags = 0;
    ctx->policy_type = POLICY_TYPE_SELINUX;
    
    // Add to context database
    security_contexts[context_count++] = ctx;
    
    *context = ctx;
    
    // Log context creation
    security_event_t event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 3,
        .blocked = false
    };
    strncpy(event.description, "Security context created", sizeof(event.description) - 1);
    strncpy(event.subject, label, sizeof(event.subject) - 1);
    security_log_event(&event);
    
    return 0;
}

/**
 * Destroy a security context
 */
void security_destroy_context(security_context_t* context) {
    if (!context) {
        return;
    }
    
    // Remove from context database
    for (size_t i = 0; i < context_count; i++) {
        if (security_contexts[i] == context) {
            security_contexts[i] = security_contexts[--context_count];
            security_contexts[context_count] = NULL;
            break;
        }
    }
    
    // Free private data if allocated
    if (context->private_data) {
        kfree(context->private_data);
    }
    
    kfree(context);
}

/**
 * Set security context for a process
 */
int security_set_context(process_t* process, security_context_t* context) {
    if (!process || !context) {
        return -EINVAL;
    }
    
    // Verify caller has permission to set context
    if (!security_check_capability(CAP_MAC_ADMIN)) {
        return -EPERM;
    }
    
    // Set security data in process
    process->security_data = context;
    
    // Update process credentials based on context
    process->creds.uid = context->uid;
    process->creds.gid = context->gid;
    process->creds.capabilities = context->capabilities;
    
    if (process->creds.security_context) {
        kfree(process->creds.security_context);
    }
    
    process->creds.security_context = kmalloc(strlen(context->context) + 1);
    if (process->creds.security_context) {
        strcpy(process->creds.security_context, context->context);
    }
    
    return 0;
}

/**
 * Get security context for a process
 */
security_context_t* security_get_context(process_t* process) {
    if (!process) {
        return NULL;
    }
    
    return (security_context_t*)process->security_data;
}

/**
 * Check if security context is valid
 */
int security_check_context(security_context_t* context) {
    if (!context) {
        return -EINVAL;
    }
    
    // Validate label format
    if (strlen(context->label) == 0 || strlen(context->label) >= SECURITY_LABEL_MAX) {
        return -EINVAL;
    }
    
    // Validate context string
    if (strlen(context->context) >= SECURITY_CONTEXT_MAX) {
        return -EINVAL;
    }
    
    // Validate security level
    if (context->level > SECURITY_LEVEL_MAXIMUM) {
        return -EINVAL;
    }
    
    // Check capabilities against current security level
    if (current_security_level >= SECURITY_LEVEL_HIGH) {
        // In high security mode, validate capability requirements
        uint64_t required_caps = security_get_required_capabilities(context->level);
        if ((context->capabilities & required_caps) != required_caps) {
            return -EPERM;
        }
    }
    
    return 0;
}

/**
 * Check permission for access
 */
int security_check_permission(security_context_t* subject, security_context_t* object,
                             const char* action, uint32_t requested_access) {
    if (!security_initialized || !subject || !object || !action) {
        return -EINVAL;
    }
    
    // If security is disabled, allow all access
    if (current_security_level == SECURITY_LEVEL_NONE) {
        return 0;
    }
    
    int result = 0;
    
    // Check MAC policy first
    if (mac_enforcing) {
        result = mac_check_permission(subject, object, action, requested_access);
        if (result != 0) {
            // Log access denial
            security_event_t event = {
                .event_id = security_generate_event_id(),
                .timestamp = get_system_time(),
                .type = SECURITY_EVENT_ACCESS_DENIED,
                .severity = 5,
                .blocked = true
            };
            strncpy(event.description, "MAC access denied", sizeof(event.description) - 1);
            strncpy(event.subject, subject->context, sizeof(event.subject) - 1);
            strncpy(event.object, object->context, sizeof(event.object) - 1);
            strncpy(event.action, action, sizeof(event.action) - 1);
            security_log_event(&event);
            
            if (!mac_permissive) {
                return result;
            }
        }
    }
    
    // Check capability requirements
    capability_t required_cap = security_action_to_capability(action);
    if (required_cap != CAP_LAST_CAP && !CAPABILITY_TEST(subject->capabilities, required_cap)) {
        // Log capability denial
        security_event_t event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .type = SECURITY_EVENT_ACCESS_DENIED,
            .severity = 4,
            .blocked = true
        };
        strncpy(event.description, "Capability access denied", sizeof(event.description) - 1);
        strncpy(event.subject, subject->context, sizeof(event.subject) - 1);
        strncpy(event.action, action, sizeof(event.action) - 1);
        security_log_event(&event);
        
        return -EPERM;
    }
    
    // Check sandbox restrictions
    process_t* current_proc = current_process();
    if (current_proc && current_proc->security_data) {
        result = security_check_sandbox_access(current_proc, object->context, action);
        if (result != 0) {
            return result;
        }
    }
    
    // All checks passed
    return 0;
}

/**
 * Check if process has capability
 */
int security_check_capability(capability_t capability) {
    process_t* proc = current_process();
    if (!proc) {
        return -ESRCH;
    }
    
    return security_check_capability_process(proc, capability);
}

/**
 * Check if specific process has capability
 */
int security_check_capability_process(process_t* process, capability_t capability) {
    if (!process) {
        return -EINVAL;
    }
    
    if (capability > CAP_LAST_CAP) {
        return -EINVAL;
    }
    
    // Check if capability is set in process capabilities
    if (!CAPABILITY_TEST(process->creds.capabilities, capability)) {
        return -EPERM;
    }
    
    // In high security mode, check additional restrictions
    if (current_security_level >= SECURITY_LEVEL_HIGH) {
        // Some capabilities require additional verification in high security mode
        switch (capability) {
            case CAP_SYS_ADMIN:
            case CAP_SYS_MODULE:
            case CAP_MAC_ADMIN:
                // Require additional authentication or restrictions
                if (!security_verify_elevated_capability(process, capability)) {
                    return -EPERM;
                }
                break;
            default:
                break;
        }
    }
    
    return 0;
}

/**
 * Grant capability to process
 */
int security_grant_capability(process_t* process, capability_t capability) {
    if (!process) {
        return -EINVAL;
    }
    
    if (capability > CAP_LAST_CAP) {
        return -EINVAL;
    }
    
    // Check if caller has permission to grant capabilities
    if (!security_check_capability(CAP_SETPCAP)) {
        return -EPERM;
    }
    
    // Grant capability
    CAPABILITY_SET(process->creds.capabilities, capability);
    
    // Log capability grant
    security_event_t event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .pid = process->pid,
        .uid = process->creds.uid,
        .type = SECURITY_EVENT_PRIVILEGE_ESCALATION,
        .severity = 6,
        .blocked = false
    };
    snprintf(event.description, sizeof(event.description), 
             "Capability %d granted to process %d", capability, process->pid);
    security_log_event(&event);
    
    return 0;
}

/**
 * Revoke capability from process
 */
int security_revoke_capability(process_t* process, capability_t capability) {
    if (!process) {
        return -EINVAL;
    }
    
    if (capability > CAP_LAST_CAP) {
        return -EINVAL;
    }
    
    // Check if caller has permission to revoke capabilities
    if (!security_check_capability(CAP_SETPCAP)) {
        return -EPERM;
    }
    
    // Revoke capability
    CAPABILITY_CLEAR(process->creds.capabilities, capability);
    
    // Log capability revocation
    security_event_t event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .pid = process->pid,
        .uid = process->creds.uid,
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 4,
        .blocked = false
    };
    snprintf(event.description, sizeof(event.description),
             "Capability %d revoked from process %d", capability, process->pid);
    security_log_event(&event);
    
    return 0;
}

/**
 * Get current security level
 */
int security_get_level(security_level_t* level) {
    if (!level) {
        return -EINVAL;
    }
    
    *level = current_security_level;
    return 0;
}

/**
 * Set system security level
 */
int security_set_level(security_level_t level) {
    if (level > SECURITY_LEVEL_MAXIMUM) {
        return -EINVAL;
    }
    
    // Check if caller has permission to change security level
    if (!security_check_capability(CAP_SYS_ADMIN)) {
        return -EPERM;
    }
    
    security_level_t old_level = current_security_level;
    current_security_level = level;
    
    // Adjust enforcement based on security level
    switch (level) {
        case SECURITY_LEVEL_NONE:
            mac_enforcing = false;
            mac_permissive = false;
            break;
        case SECURITY_LEVEL_BASIC:
            mac_enforcing = false;
            mac_permissive = true;
            break;
        case SECURITY_LEVEL_ENHANCED:
            mac_enforcing = true;
            mac_permissive = false;
            break;
        case SECURITY_LEVEL_HIGH:
        case SECURITY_LEVEL_MAXIMUM:
            mac_enforcing = true;
            mac_permissive = false;
            // Enable additional security features
            security_enable_enhanced_features();
            break;
    }
    
    // Log security level change
    security_event_t event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 8,
        .blocked = false
    };
    snprintf(event.description, sizeof(event.description),
             "Security level changed from %s to %s", 
             security_level_to_string(old_level),
             security_level_to_string(level));
    security_log_event(&event);
    
    kernel_printf("Security: Level changed to %s\n", security_level_to_string(level));
    
    return 0;
}

/**
 * Log security event
 */
int security_log_event(security_event_t* event) {
    if (!event) {
        return -EINVAL;
    }
    
    // Add to audit buffer (circular buffer)
    size_t next_head = (audit_head + 1) % SECURITY_AUDIT_BUFFER_SIZE;
    if (next_head == audit_tail) {
        // Buffer full, drop oldest event
        audit_tail = (audit_tail + 1) % SECURITY_AUDIT_BUFFER_SIZE;
    }
    
    audit_buffer[audit_head] = *event;
    audit_head = next_head;
    
    // Forward to audit subsystem for persistent logging
    audit_log_event(event);
    
    // Check for intrusion detection patterns
    ids_analyze_event(event);
    
    return 0;
}

// Helper functions

/**
 * Convert security level to string
 */
static const char* security_level_to_string(security_level_t level) {
    switch (level) {
        case SECURITY_LEVEL_NONE: return "None";
        case SECURITY_LEVEL_BASIC: return "Basic";
        case SECURITY_LEVEL_ENHANCED: return "Enhanced";
        case SECURITY_LEVEL_HIGH: return "High";
        case SECURITY_LEVEL_MAXIMUM: return "Maximum";
        default: return "Unknown";
    }
}

/**
 * Generate unique event ID
 */
static uint32_t security_generate_event_id(void) {
    static uint32_t event_counter = 1;
    return event_counter++;
}

/**
 * Map action to required capability
 */
static capability_t security_action_to_capability(const char* action) {
    if (!action) {
        return CAP_LAST_CAP;
    }
    
    // Map common actions to capabilities
    if (strcmp(action, "read") == 0) return CAP_DAC_READ_SEARCH;
    if (strcmp(action, "write") == 0) return CAP_DAC_OVERRIDE;
    if (strcmp(action, "execute") == 0) return CAP_DAC_OVERRIDE;
    if (strcmp(action, "create") == 0) return CAP_DAC_OVERRIDE;
    if (strcmp(action, "delete") == 0) return CAP_DAC_OVERRIDE;
    if (strcmp(action, "chown") == 0) return CAP_CHOWN;
    if (strcmp(action, "chmod") == 0) return CAP_FOWNER;
    if (strcmp(action, "mount") == 0) return CAP_SYS_ADMIN;
    if (strcmp(action, "module_load") == 0) return CAP_SYS_MODULE;
    if (strcmp(action, "ptrace") == 0) return CAP_SYS_PTRACE;
    if (strcmp(action, "network_bind") == 0) return CAP_NET_BIND_SERVICE;
    if (strcmp(action, "network_raw") == 0) return CAP_NET_RAW;
    if (strcmp(action, "network_admin") == 0) return CAP_NET_ADMIN;
    
    return CAP_LAST_CAP; // No specific capability required
}

// Global security operations pointer
security_ops_t* security = NULL;