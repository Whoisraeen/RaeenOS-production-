/**
 * RaeenOS Security Framework - Access Control System
 * Integrates policy engine with sandbox and capability management
 */

#include "raesec.h"
#include "security_core.h"
#include "../kernel/process_advanced.h"
#include "../kernel/filesystem_advanced.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

// Access control enforcement modes
typedef enum {
    ENFORCE_MODE_DISABLED = 0,
    ENFORCE_MODE_LEARNING = 1,
    ENFORCE_MODE_ENFORCING = 2,
    ENFORCE_MODE_PARANOID = 3
} EnforcementMode;

// Global access control state
static EnforcementMode g_enforcement_mode = ENFORCE_MODE_ENFORCING;
static bool g_access_control_initialized = false;
static uint64_t g_access_requests = 0;
static uint64_t g_access_granted = 0;
static uint64_t g_access_denied = 0;
static uint64_t g_access_prompted = 0;

// Internal function declarations
static bool validate_filesystem_access(SecurityFramework* framework, SecurityContext* context, const char* path, CapabilityType capability);
static bool validate_network_access(SecurityFramework* framework, SecurityContext* context, const char* resource, CapabilityType capability);
static bool validate_device_access(SecurityFramework* framework, SecurityContext* context, const char* device, CapabilityType capability);
static bool validate_process_access(SecurityFramework* framework, SecurityContext* context, uint32_t target_pid, CapabilityType capability);
static bool handle_policy_prompt(SecurityContext* context, const char* resource, CapabilityType capability);
static bool log_access_attempt(SecurityContext* context, const char* resource, CapabilityType capability, PolicyAction action, bool granted);
static bool apply_quarantine(SecurityFramework* framework, SecurityContext* context);

/**
 * Initialize access control system
 */
bool raesec_init_access_control(SecurityFramework* framework) {
    if (!framework) return false;
    
    if (g_access_control_initialized) {
        return true;
    }
    
    printf("Initializing RaeenOS Access Control System\n");
    
    // Set default enforcement mode from environment
    const char* mode_env = getenv("RAESEC_ENFORCEMENT_MODE");
    if (mode_env) {
        if (strcmp(mode_env, "disabled") == 0) {
            g_enforcement_mode = ENFORCE_MODE_DISABLED;
        } else if (strcmp(mode_env, "learning") == 0) {
            g_enforcement_mode = ENFORCE_MODE_LEARNING;
        } else if (strcmp(mode_env, "enforcing") == 0) {
            g_enforcement_mode = ENFORCE_MODE_ENFORCING;
        } else if (strcmp(mode_env, "paranoid") == 0) {
            g_enforcement_mode = ENFORCE_MODE_PARANOID;
        }
    }
    
    printf("Access control enforcement mode: %d\n", g_enforcement_mode);
    
    g_access_control_initialized = true;
    return true;
}

/**
 * Main access control enforcement function
 */
bool raesec_enforce_access_control(SecurityFramework* framework, SecurityContext* context, const char* resource_path, CapabilityType capability) {
    if (!framework || !context || !resource_path) {
        return false;
    }
    
    g_access_requests++;
    
    // Skip enforcement if disabled
    if (g_enforcement_mode == ENFORCE_MODE_DISABLED) {
        g_access_granted++;
        return true;
    }
    
    // System context always allowed
    if (context->type == SEC_CONTEXT_SYSTEM) {
        g_access_granted++;
        return true;
    }
    
    // Evaluate security policy
    PolicyAction action;
    raesec_evaluate_policy(framework, context, resource_path, capability, &action);
    
    bool access_granted = false;
    
    switch (action) {
        case POLICY_ACTION_ALLOW:
            access_granted = true;
            g_access_granted++;
            break;
            
        case POLICY_ACTION_DENY:
            access_granted = false;
            g_access_denied++;
            break;
            
        case POLICY_ACTION_LOG:
            access_granted = true;
            g_access_granted++;
            log_access_attempt(context, resource_path, capability, action, true);
            break;
            
        case POLICY_ACTION_PROMPT:
            if (g_enforcement_mode == ENFORCE_MODE_LEARNING) {
                access_granted = true;
                g_access_granted++;
            } else {
                access_granted = handle_policy_prompt(context, resource_path, capability);
                if (access_granted) {
                    g_access_granted++;
                } else {
                    g_access_denied++;
                }
            }
            g_access_prompted++;
            break;
            
        case POLICY_ACTION_QUARANTINE:
            access_granted = false;
            g_access_denied++;
            apply_quarantine(framework, context);
            break;
    }
    
    // Additional validation based on resource type
    if (access_granted) {
        if (strncmp(resource_path, "/dev/", 5) == 0) {
            access_granted = validate_device_access(framework, context, resource_path, capability);
        } else if (strncmp(resource_path, "network:", 8) == 0) {
            access_granted = validate_network_access(framework, context, resource_path + 8, capability);
        } else if (strncmp(resource_path, "process:", 8) == 0) {
            uint32_t target_pid = atoi(resource_path + 8);
            access_granted = validate_process_access(framework, context, target_pid, capability);
        } else {
            access_granted = validate_filesystem_access(framework, context, resource_path, capability);
        }
    }
    
    // Log the access attempt
    log_access_attempt(context, resource_path, capability, action, access_granted);
    
    // In paranoid mode, deny access to untrusted contexts by default
    if (g_enforcement_mode == ENFORCE_MODE_PARANOID && context->type == SEC_CONTEXT_UNTRUSTED) {
        access_granted = false;
        g_access_denied++;
    }
    
    return access_granted;
}

/**
 * Check if a process has specific capability
 */
bool raesec_check_capability_access(SecurityFramework* framework, uint32_t process_id, CapabilityType capability) {
    if (!framework) return false;
    
    SecurityContext* context = raesec_get_context_by_process(framework, process_id);
    if (!context) {
        return false;
    }
    
    return raesec_check_capability(framework, context, capability);
}

/**
 * Validate filesystem access with sandbox integration
 */
static bool validate_filesystem_access(SecurityFramework* framework, SecurityContext* context, const char* path, CapabilityType capability) {
    // Check if process is sandboxed
    if (raesec_is_process_sandboxed(context->process_id)) {
        return raesec_validate_sandbox_access(context->process_id, path, capability);
    }
    
    // Check basic filesystem permissions
    struct stat st;
    if (stat(path, &st) != 0) {
        return false; // Path doesn't exist
    }
    
    // Check capability-specific access
    switch (capability) {
        case CAP_FILESYSTEM_READ:
            return (st.st_mode & S_IRUSR) || context->type == SEC_CONTEXT_ADMIN;
            
        case CAP_FILESYSTEM_WRITE:
            return (st.st_mode & S_IWUSR) || context->type == SEC_CONTEXT_ADMIN;
            
        case CAP_FILESYSTEM_EXECUTE:
            return (st.st_mode & S_IXUSR) || context->type == SEC_CONTEXT_ADMIN;
            
        case CAP_FILESYSTEM_DELETE:
            return context->type == SEC_CONTEXT_ADMIN || context->type == SEC_CONTEXT_USER;
            
        default:
            return false;
    }
}

/**
 * Validate network access
 */
static bool validate_network_access(SecurityFramework* framework, SecurityContext* context, const char* resource, CapabilityType capability) {
    // Check if process is sandboxed with network restrictions
    if (raesec_is_process_sandboxed(context->process_id)) {
        SandboxConfig* config = raesec_get_sandbox_config(context->process_id);
        if (config && !config->allow_network) {
            return false;
        }
    }
    
    // Check network capability
    if (!raesec_check_capability(framework, context, CAP_NETWORK_ACCESS)) {
        return false;
    }
    
    // Additional network-specific checks can be added here
    return true;
}

/**
 * Validate device access
 */
static bool validate_device_access(SecurityFramework* framework, SecurityContext* context, const char* device, CapabilityType capability) {
    // Check if process is sandboxed
    if (raesec_is_process_sandboxed(context->process_id)) {
        // Most devices are denied in sandbox by default
        if (strstr(device, "/dev/null") || strstr(device, "/dev/zero") || strstr(device, "/dev/random")) {
            return true; // Allow safe devices
        }
        return false;
    }
    
    // Check device capability
    if (!raesec_check_capability(framework, context, CAP_DEVICE_ACCESS)) {
        return false;
    }
    
    return true;
}

/**
 * Validate process access
 */
static bool validate_process_access(SecurityFramework* framework, SecurityContext* context, uint32_t target_pid, CapabilityType capability) {
    // Get target process context
    SecurityContext* target_context = raesec_get_context_by_process(framework, target_pid);
    if (!target_context) {
        return false;
    }
    
    // Same process always allowed
    if (context->process_id == target_pid) {
        return true;
    }
    
    // Admin can access any process
    if (context->type == SEC_CONTEXT_ADMIN) {
        return true;
    }
    
    // Same user can access their own processes
    if (context->user_id == target_context->user_id) {
        return raesec_check_capability(framework, context, CAP_PROCESS_CONTROL);
    }
    
    // Cross-user access requires admin privileges
    return false;
}

/**
 * Handle policy prompt (simplified implementation)
 */
static bool handle_policy_prompt(SecurityContext* context, const char* resource, CapabilityType capability) {
    // In a real implementation, this would show a user prompt
    // For now, we'll default to deny for security
    printf("SECURITY PROMPT: Process %u requesting %s access to %s\n", 
           context->process_id, raesec_capability_to_string(capability), resource);
    
    // Auto-deny for untrusted contexts
    if (context->type == SEC_CONTEXT_UNTRUSTED) {
        return false;
    }
    
    // Auto-allow for user contexts (would be prompted in real implementation)
    return context->type == SEC_CONTEXT_USER;
}

/**
 * Log access attempt
 */
static bool log_access_attempt(SecurityContext* context, const char* resource, CapabilityType capability, PolicyAction action, bool granted) {
    SecurityEvent event;
    memset(&event, 0, sizeof(event));
    
    event.event_type = granted ? SEC_EVENT_ACCESS_GRANTED : SEC_EVENT_ACCESS_DENIED;
    event.timestamp = time(NULL);
    event.process_id = context->process_id;
    event.user_id = context->user_id;
    event.severity = granted ? SEC_SEVERITY_INFO : SEC_SEVERITY_WARNING;
    
    snprintf(event.description, sizeof(event.description), 
             "%s access to %s (%s) - %s", 
             raesec_capability_to_string(capability),
             resource,
             raesec_policy_action_to_string(action),
             granted ? "GRANTED" : "DENIED");
    
    return raesec_log_security_event(NULL, &event);
}

/**
 * Apply quarantine to a process
 */
static bool apply_quarantine(SecurityFramework* framework, SecurityContext* context) {
    printf("QUARANTINE: Applying quarantine to process %u\n", context->process_id);
    
    // Create strict sandbox for quarantined process
    SandboxConfig* quarantine_config = raesec_create_strict_sandbox_config("quarantine", "/tmp/quarantine");
    if (!quarantine_config) {
        return false;
    }
    
    // Apply even stricter restrictions
    quarantine_config->allow_network = false;
    quarantine_config->max_memory = 64 * 1024 * 1024; // 64MB
    quarantine_config->max_processes = 1;
    quarantine_config->readonly_root = true;
    
    // Apply quarantine sandbox
    bool result = raesec_create_sandbox(framework, context, quarantine_config);
    
    // Log quarantine event
    SecurityEvent event;
    memset(&event, 0, sizeof(event));
    event.event_type = SEC_EVENT_QUARANTINE_APPLIED;
    event.timestamp = time(NULL);
    event.process_id = context->process_id;
    event.user_id = context->user_id;
    event.severity = SEC_SEVERITY_CRITICAL;
    snprintf(event.description, sizeof(event.description), 
             "Process quarantined due to security policy violation");
    
    raesec_log_security_event(framework, &event);
    
    return result;
}

/**
 * Set enforcement mode
 */
bool raesec_set_enforcement_mode(int mode) {
    if (mode < ENFORCE_MODE_DISABLED || mode > ENFORCE_MODE_PARANOID) {
        return false;
    }
    
    g_enforcement_mode = mode;
    printf("Access control enforcement mode set to: %d\n", mode);
    return true;
}

/**
 * Get enforcement mode
 */
int raesec_get_enforcement_mode(void) {
    return g_enforcement_mode;
}

/**
 * Get access control statistics
 */
void raesec_get_access_statistics(uint64_t* total_requests, uint64_t* granted, uint64_t* denied, uint64_t* prompted) {
    if (total_requests) *total_requests = g_access_requests;
    if (granted) *granted = g_access_granted;
    if (denied) *denied = g_access_denied;
    if (prompted) *prompted = g_access_prompted;
}

/**
 * Reset access control statistics
 */
void raesec_reset_access_statistics(void) {
    g_access_requests = 0;
    g_access_granted = 0;
    g_access_denied = 0;
    g_access_prompted = 0;
    printf("Access control statistics reset\n");
}
