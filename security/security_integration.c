/**
 * @file security_integration.c
 * @brief Security Framework Integration Module
 * 
 * This module provides integration points between the security framework
 * and other RaeenOS kernel subsystems:
 * - Kernel hook registration and management
 * - VFS security integration
 * - Process lifecycle security hooks
 * - Network stack security integration
 * - Driver framework security validation
 * - AI system security controls
 * - System call security filtering
 * 
 * This module ensures that security policies are enforced consistently
 * across all kernel subsystems.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../memory.h"
#include "../string.h"
#include "../include/driver_framework.h"
#include "../include/ai_interface.h"
#include "../fs/vfs.h"

// Security integration state
static struct {
    bool initialized;
    bool hooks_installed;
    uint32_t hook_count;
    uint64_t security_checks_performed;
    uint64_t security_violations_blocked;
} security_integration_state = {0};

// Security hook types
typedef enum {
    SECURITY_HOOK_PROCESS_CREATE,
    SECURITY_HOOK_PROCESS_EXEC,
    SECURITY_HOOK_PROCESS_EXIT,
    SECURITY_HOOK_FILE_OPEN,
    SECURITY_HOOK_FILE_READ,
    SECURITY_HOOK_FILE_WRITE,
    SECURITY_HOOK_FILE_EXECUTE,
    SECURITY_HOOK_NETWORK_CONNECT,
    SECURITY_HOOK_NETWORK_BIND,
    SECURITY_HOOK_DRIVER_LOAD,
    SECURITY_HOOK_AI_ACCESS,
    SECURITY_HOOK_SYSCALL_ENTER,
    SECURITY_HOOK_SYSCALL_EXIT,
    SECURITY_HOOK_COUNT
} security_hook_type_t;

// Security hook function prototype
typedef int (*security_hook_func_t)(void* data);

// Security hook structure
typedef struct security_hook {
    security_hook_type_t type;
    security_hook_func_t function;
    bool enabled;
    uint32_t call_count;
    struct security_hook* next;
} security_hook_t;

// Security hook chains
static security_hook_t* security_hooks[SECURITY_HOOK_COUNT] = {0};

/**
 * Initialize security integration
 */
int security_init_integration(void) {
    if (security_integration_state.initialized) {
        return 0;
    }
    
    // Initialize hook chains
    memset(security_hooks, 0, sizeof(security_hooks));
    
    // Install default security hooks
    int ret = security_install_default_hooks();
    if (ret != 0) {
        kernel_printf("Security Integration: Failed to install default hooks: %d\n", ret);
        return ret;
    }
    
    security_integration_state.initialized = true;
    security_integration_state.hooks_installed = true;
    security_integration_state.security_checks_performed = 0;
    security_integration_state.security_violations_blocked = 0;
    
    kernel_printf("Security Integration: Framework integrated with kernel subsystems\n");
    kernel_printf("  Security hooks installed: %u\n", security_integration_state.hook_count);
    
    return 0;
}

/**
 * Cleanup security integration
 */
void security_cleanup_integration(void) {
    if (!security_integration_state.initialized) {
        return;
    }
    
    // Remove all security hooks
    for (int i = 0; i < SECURITY_HOOK_COUNT; i++) {
        security_hook_t* hook = security_hooks[i];
        while (hook) {
            security_hook_t* next = hook->next;
            kfree(hook);
            hook = next;
        }
        security_hooks[i] = NULL;
    }
    
    security_integration_state.initialized = false;
    security_integration_state.hooks_installed = false;
    
    kernel_printf("Security Integration: Cleanup complete\n");
    kernel_printf("  Security checks performed: %llu\n", security_integration_state.security_checks_performed);
    kernel_printf("  Security violations blocked: %llu\n", security_integration_state.security_violations_blocked);
}

/**
 * Process creation security hook
 */
int security_hook_process_create(process_t* parent, process_t* child) {
    if (!security_integration_state.initialized) {
        return 0;
    }
    
    security_integration_state.security_checks_performed++;
    
    // Check if parent has permission to create child processes
    if (!security_check_capability_process(parent, CAP_FORK)) {
        security_integration_state.security_violations_blocked++;
        return -EPERM;
    }
    
    // Inherit security context from parent
    if (parent->security_data) {
        security_context_t* parent_ctx = (security_context_t*)parent->security_data;
        security_context_t* child_ctx;
        
        int ret = security_create_context(parent_ctx->context, &child_ctx);
        if (ret == 0) {
            child_ctx->uid = parent_ctx->uid;
            child_ctx->gid = parent_ctx->gid;
            child_ctx->capabilities = parent_ctx->capabilities;
            child_ctx->level = parent_ctx->level;
            
            security_set_context(child, child_ctx);
        }
    }
    
    // Apply sandbox profile if parent is sandboxed
    if (parent->security_data) {
        sandbox_profile_t* profile = (sandbox_profile_t*)parent->security_data;
        if (profile) {
            security_apply_sandbox(child, profile);
        }
    }
    
    // Log process creation
    security_event_t create_event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .pid = child->pid,
        .uid = parent->creds.uid,
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 3,
        .blocked = false
    };
    
    snprintf(create_event.description, sizeof(create_event.description),
             "Process created: parent_pid=%u, child_pid=%u", parent->pid, child->pid);
    
    security_log_event(&create_event);
    
    return 0;
}

/**
 * File access security hook
 */
int security_hook_file_access(process_t* process, const char* path, uint32_t access_mode) {
    if (!security_integration_state.initialized || !process || !path) {
        return 0;
    }
    
    security_integration_state.security_checks_performed++;
    
    // Get security contexts
    security_context_t* subject_ctx = security_get_context(process);
    if (!subject_ctx) {
        // No security context - use default
        return 0;
    }
    
    // Create object context for file
    security_context_t object_ctx;
    memset(&object_ctx, 0, sizeof(object_ctx));
    
    // Determine file security context based on path
    if (strncmp(path, "/etc/", 5) == 0) {
        strcpy(object_ctx.context, "system_u:object_r:etc_t:s0");
    } else if (strncmp(path, "/tmp/", 5) == 0) {
        strcpy(object_ctx.context, "system_u:object_r:tmp_t:s0");
    } else if (strncmp(path, "/dev/", 5) == 0) {
        strcpy(object_ctx.context, "system_u:object_r:device_t:s0");
    } else if (strncmp(path, "/home/", 6) == 0) {
        strcpy(object_ctx.context, "system_u:object_r:home_t:s0");
    } else {
        strcpy(object_ctx.context, "system_u:object_r:file_t:s0");
    }
    
    // Determine action based on access mode
    const char* action;
    if (access_mode & 0x01) {
        action = "execute";
    } else if (access_mode & 0x02) {
        action = "write";
    } else {
        action = "read";
    }
    
    // Check MAC policy
    int ret = security_check_permission(subject_ctx, &object_ctx, action, access_mode);
    if (ret != 0) {
        security_integration_state.security_violations_blocked++;
        
        // Log access denial
        security_event_t deny_event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .pid = process->pid,
            .uid = process->creds.uid,
            .type = SECURITY_EVENT_FILE_ACCESS,
            .severity = 5,
            .blocked = true
        };
        
        snprintf(deny_event.description, sizeof(deny_event.description),
                 "File access denied: %s", path);
        strcpy(deny_event.data.file.path, path);
        deny_event.data.file.access_mode = access_mode;
        
        security_log_event(&deny_event);
        
        return ret;
    }
    
    // Check sandbox restrictions
    ret = security_check_sandbox_access(process, path, action);
    if (ret != 0) {
        security_integration_state.security_violations_blocked++;
        return ret;
    }
    
    return 0;
}

/**
 * Network access security hook
 */
int security_hook_network_access(process_t* process, uint32_t addr, uint16_t port, const char* protocol) {
    if (!security_integration_state.initialized || !process) {
        return 0;
    }
    
    security_integration_state.security_checks_performed++;
    
    // Check network access permissions
    int ret = security_check_network_access(process, addr, port, protocol);
    if (ret != 0) {
        security_integration_state.security_violations_blocked++;
        return ret;
    }
    
    return 0;
}

/**
 * Driver loading security hook
 */
int security_hook_driver_load(const char* driver_path) {
    if (!security_integration_state.initialized || !driver_path) {
        return 0;
    }
    
    security_integration_state.security_checks_performed++;
    
    // Check if caller has permission to load drivers
    if (!security_check_capability(CAP_SYS_MODULE)) {
        security_integration_state.security_violations_blocked++;
        
        // Log driver load attempt
        security_event_t load_event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 7,
            .blocked = true
        };
        
        snprintf(load_event.description, sizeof(load_event.description),
                 "Unauthorized driver load attempt: %s", driver_path);
        
        security_log_event(&load_event);
        
        return -EPERM;
    }
    
    // Verify driver signature
    int ret = security_verify_module_integrity(driver_path);
    if (ret != 0) {
        security_integration_state.security_violations_blocked++;
        
        // Log integrity failure
        security_event_t integrity_event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .type = SECURITY_EVENT_SUSPICIOUS_ACTIVITY,
            .severity = 8,
            .blocked = true
        };
        
        snprintf(integrity_event.description, sizeof(integrity_event.description),
                 "Driver integrity verification failed: %s", driver_path);
        
        security_log_event(&integrity_event);
        
        return ret;
    }
    
    return 0;
}

/**
 * AI system access security hook
 */
int security_hook_ai_access(process_t* process, const char* ai_resource, const char* action) {
    if (!security_integration_state.initialized || !process || !ai_resource || !action) {
        return 0;
    }
    
    security_integration_state.security_checks_performed++;
    
    // Check AI access permissions through sandbox
    char resource_name[256];
    snprintf(resource_name, sizeof(resource_name), "ai:%s", ai_resource);
    
    int ret = security_check_sandbox_access(process, resource_name, action);
    if (ret != 0) {
        security_integration_state.security_violations_blocked++;
        
        // Log AI access denial
        security_event_t ai_event = {
            .event_id = security_generate_event_id(),
            .timestamp = get_system_time(),
            .pid = process->pid,
            .uid = process->creds.uid,
            .type = SECURITY_EVENT_ACCESS_DENIED,
            .severity = 6,
            .blocked = true
        };
        
        snprintf(ai_event.description, sizeof(ai_event.description),
                 "AI access denied: resource=%s, action=%s", ai_resource, action);
        
        security_log_event(&ai_event);
        
        return ret;
    }
    
    // Check if process has AI access capability
    if (!security_check_capability_process(process, CAP_RAEEN_AI_ACCESS)) {
        security_integration_state.security_violations_blocked++;
        return -EPERM;
    }
    
    return 0;
}

/**
 * System call security hook
 */
int security_hook_syscall_enter(process_t* process, uint32_t syscall_number, void* args) {
    if (!security_integration_state.initialized || !process) {
        return 0;
    }
    
    security_integration_state.security_checks_performed++;
    
    // Check if system call is allowed by sandbox
    if (process->security_data) {
        sandbox_profile_t* profile = (sandbox_profile_t*)process->security_data;
        
        if (profile->syscalls.default_deny) {
            // Check if syscall is in allowed list
            bool allowed = false;
            for (size_t i = 0; i < profile->syscalls.syscall_count; i++) {
                if (profile->syscalls.allowed_syscalls[i] == syscall_number) {
                    allowed = true;
                    break;
                }
            }
            
            if (!allowed) {
                security_integration_state.security_violations_blocked++;
                
                // Log syscall denial
                security_event_t syscall_event = {
                    .event_id = security_generate_event_id(),
                    .timestamp = get_system_time(),
                    .pid = process->pid,
                    .uid = process->creds.uid,
                    .type = SECURITY_EVENT_SYSTEM_CALL,
                    .severity = 4,
                    .blocked = true
                };
                
                snprintf(syscall_event.description, sizeof(syscall_event.description),
                         "System call blocked by sandbox: %u", syscall_number);
                syscall_event.data.syscall.syscall_number = syscall_number;
                
                security_log_event(&syscall_event);
                
                return -EPERM;
            }
        }
    }
    
    // Check for potentially dangerous system calls
    switch (syscall_number) {
        case 1: // sys_exit
        case 2: // sys_fork
        case 11: // sys_execve
        case 23: // sys_setuid
        case 46: // sys_setgid
            // These require special capabilities - check in syscall handler
            break;
            
        case 169: // sys_reboot
            if (!security_check_capability_process(process, CAP_SYS_BOOT)) {
                security_integration_state.security_violations_blocked++;
                return -EPERM;
            }
            break;
            
        case 175: // sys_init_module
            if (!security_check_capability_process(process, CAP_SYS_MODULE)) {
                security_integration_state.security_violations_blocked++;
                return -EPERM;
            }
            break;
    }
    
    return 0;
}

/**
 * Install default security hooks
 */
static int security_install_default_hooks(void) {
    // Install process lifecycle hooks
    security_register_hook(SECURITY_HOOK_PROCESS_CREATE, (security_hook_func_t)security_hook_process_create);
    security_register_hook(SECURITY_HOOK_FILE_OPEN, (security_hook_func_t)security_hook_file_access);
    security_register_hook(SECURITY_HOOK_NETWORK_CONNECT, (security_hook_func_t)security_hook_network_access);
    security_register_hook(SECURITY_HOOK_DRIVER_LOAD, (security_hook_func_t)security_hook_driver_load);
    security_register_hook(SECURITY_HOOK_AI_ACCESS, (security_hook_func_t)security_hook_ai_access);
    security_register_hook(SECURITY_HOOK_SYSCALL_ENTER, (security_hook_func_t)security_hook_syscall_enter);
    
    return 0;
}

/**
 * Register security hook
 */
int security_register_hook(security_hook_type_t type, security_hook_func_t function) {
    if (type >= SECURITY_HOOK_COUNT || !function) {
        return -EINVAL;
    }
    
    security_hook_t* hook = kmalloc(sizeof(security_hook_t));
    if (!hook) {
        return -ENOMEM;
    }
    
    hook->type = type;
    hook->function = function;
    hook->enabled = true;
    hook->call_count = 0;
    hook->next = security_hooks[type];
    
    security_hooks[type] = hook;
    security_integration_state.hook_count++;
    
    return 0;
}

/**
 * Call security hooks for specific type
 */
int security_call_hooks(security_hook_type_t type, void* data) {
    if (type >= SECURITY_HOOK_COUNT || !security_integration_state.hooks_installed) {
        return 0;
    }
    
    security_hook_t* hook = security_hooks[type];
    while (hook) {
        if (hook->enabled) {
            hook->call_count++;
            int ret = hook->function(data);
            if (ret != 0) {
                return ret; // Security hook blocked the operation
            }
        }
        hook = hook->next;
    }
    
    return 0;
}

/**
 * Get security integration statistics
 */
int security_get_integration_stats(security_integration_stats_t* stats) {
    if (!stats) {
        return -EINVAL;
    }
    
    memset(stats, 0, sizeof(security_integration_stats_t));
    
    stats->initialized = security_integration_state.initialized;
    stats->hooks_installed = security_integration_state.hooks_installed;
    stats->hook_count = security_integration_state.hook_count;
    stats->security_checks_performed = security_integration_state.security_checks_performed;
    stats->security_violations_blocked = security_integration_state.security_violations_blocked;
    
    // Count hooks by type
    for (int i = 0; i < SECURITY_HOOK_COUNT; i++) {
        security_hook_t* hook = security_hooks[i];
        while (hook) {
            stats->total_hook_calls += hook->call_count;
            hook = hook->next;
        }
    }
    
    return 0;
}

// Security integration statistics structure
typedef struct security_integration_stats {
    bool initialized;
    bool hooks_installed;
    uint32_t hook_count;
    uint64_t security_checks_performed;
    uint64_t security_violations_blocked;
    uint64_t total_hook_calls;
} security_integration_stats_t;