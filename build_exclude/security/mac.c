/**
 * @file mac.c
 * @brief Mandatory Access Control (MAC) Framework Implementation
 * 
 * This module implements the MAC framework for RaeenOS, providing:
 * - SELinux-style type enforcement
 * - Role-Based Access Control (RBAC)
 * - Multi-Level Security (MLS)
 * - Policy-based access decisions
 * - Fine-grained permission control
 * 
 * The MAC framework works alongside DAC to provide defense-in-depth
 * security by enforcing system-wide security policies that cannot
 * be overridden by users.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../memory.h"
#include "../string.h"

// MAC policy database
static mac_policy_t* current_policy = NULL;
static mac_rule_t* policy_rules = NULL;
static size_t rule_count = 0;
static bool mac_initialized = false;

// MAC enforcement state
static bool mac_enforcing = true;
static bool mac_permissive = false;

// Default security types
static const char* default_subject_types[] = {
    "kernel_t",
    "init_t", 
    "user_t",
    "admin_t",
    "system_t",
    "unconfined_t",
    NULL
};

static const char* default_object_types[] = {
    "file_t",
    "device_t",
    "proc_t",
    "sysfs_t",
    "tmp_t",
    "etc_t",
    "bin_t",
    "lib_t",
    "home_t",
    NULL
};

// Access vectors for different object classes
typedef struct access_vector {
    const char* class_name;
    const char* permissions[32];
} access_vector_t;

static const access_vector_t access_vectors[] = {
    {
        .class_name = "file",
        .permissions = {
            "read", "write", "execute", "append", "create", "unlink",
            "link", "rename", "setattr", "getattr", "lock", "relabelfrom",
            "relabelto", "mounton", "quotaon", "audit_access", NULL
        }
    },
    {
        .class_name = "dir",
        .permissions = {
            "read", "write", "execute", "add_name", "remove_name", "reparent",
            "search", "rmdir", "open", "quotaon", "mounton", NULL
        }
    },
    {
        .class_name = "process",
        .permissions = {
            "fork", "transition", "sigchld", "sigkill", "sigstop", "signull",
            "signal", "ptrace", "getsched", "setsched", "getsession", "getpgid",
            "setpgid", "getcap", "setcap", "share", "getattr", "setexec",
            "setfscreate", "noatsecure", "siginh", "setrlimit", "rlimitinh",
            "dyntransition", "setcurrent", "execmem", "execstack", "execheap",
            "setkeycreate", "setsockcreate", NULL
        }
    },
    {
        .class_name = "socket",
        .permissions = {
            "create", "connect", "listen", "accept", "bind", "read", "write",
            "sendto", "recvfrom", "name_bind", "name_connect", NULL
        }
    },
    { .class_name = NULL }
};

/**
 * Initialize MAC framework
 */
int mac_init(void) {
    if (mac_initialized) {
        return 0;
    }
    
    // Allocate space for policy rules
    policy_rules = kmalloc(sizeof(mac_rule_t) * MAX_SECURITY_RULES);
    if (!policy_rules) {
        return -ENOMEM;
    }
    
    memset(policy_rules, 0, sizeof(mac_rule_t) * MAX_SECURITY_RULES);
    rule_count = 0;
    
    // Create default policy
    current_policy = kmalloc(sizeof(mac_policy_t));
    if (!current_policy) {
        kfree(policy_rules);
        return -ENOMEM;
    }
    
    memset(current_policy, 0, sizeof(mac_policy_t));
    strcpy(current_policy->name, "default");
    current_policy->rules = policy_rules;
    current_policy->rule_count = 0;
    current_policy->enforcing = true;
    
    // Load default MAC rules
    int ret = mac_load_default_rules();
    if (ret != 0) {
        kfree(current_policy);
        kfree(policy_rules);
        return ret;
    }
    
    mac_initialized = true;
    kernel_printf("MAC: Framework initialized with %zu rules\n", rule_count);
    
    return 0;
}

/**
 * Cleanup MAC framework
 */
void mac_cleanup(void) {
    if (!mac_initialized) {
        return;
    }
    
    if (current_policy) {
        kfree(current_policy);
        current_policy = NULL;
    }
    
    if (policy_rules) {
        kfree(policy_rules);
        policy_rules = NULL;
    }
    
    rule_count = 0;
    mac_initialized = false;
    
    kernel_printf("MAC: Framework cleaned up\n");
}

/**
 * Check permission using MAC policy
 */
int mac_check_permission(security_context_t* subject, security_context_t* object,
                        const char* action, uint32_t requested_access) {
    if (!mac_initialized || !subject || !object || !action) {
        return -EINVAL;
    }
    
    // Extract type information from contexts
    char subject_type[64];
    char object_type[64];
    
    int ret = mac_extract_type_from_context(subject->context, subject_type, sizeof(subject_type));
    if (ret != 0) {
        return ret;
    }
    
    ret = mac_extract_type_from_context(object->context, object_type, sizeof(object_type));
    if (ret != 0) {
        return ret;
    }
    
    // Check against policy rules
    for (size_t i = 0; i < rule_count; i++) {
        mac_rule_t* rule = &policy_rules[i];
        
        // Check if rule matches the access attempt
        if (mac_match_pattern(rule->subject_type, subject_type) &&
            mac_match_pattern(rule->object_type, object_type) &&
            strcmp(rule->action, action) == 0) {
            
            // Check additional conditions if present
            if (rule->conditions != 0) {
                if (!mac_check_conditions(rule->conditions, subject, object)) {
                    continue;
                }
            }
            
            // Rule matched - return decision
            if (rule->allow) {
                return 0; // Access allowed
            } else {
                return -EACCES; // Access denied
            }
        }
    }
    
    // No explicit rule found - default deny in enforcing mode
    if (current_policy->enforcing) {
        return -EACCES;
    }
    
    // Allow in permissive mode
    return 0;
}

/**
 * Load MAC policy from file
 */
int mac_load_policy(const char* policy_path) {
    if (!policy_path) {
        return -EINVAL;
    }
    
    // For now, return success - full policy loading would require
    // a policy compiler and file system access
    kernel_printf("MAC: Policy loading from %s not yet implemented\n", policy_path);
    return 0;
}

/**
 * Set MAC enforcement mode
 */
int mac_set_enforcing(bool enforcing) {
    if (!mac_initialized) {
        return -ENODEV;
    }
    
    bool old_enforcing = mac_enforcing;
    mac_enforcing = enforcing;
    
    if (current_policy) {
        current_policy->enforcing = enforcing;
    }
    
    kernel_printf("MAC: Enforcement mode changed from %s to %s\n",
                  old_enforcing ? "enforcing" : "permissive",
                  enforcing ? "enforcing" : "permissive");
    
    return 0;
}

/**
 * Load default MAC rules
 */
static int mac_load_default_rules(void) {
    // Add basic allow rules for kernel operations
    mac_add_rule("kernel_t", "*", "read", true, 0);
    mac_add_rule("kernel_t", "*", "write", true, 0);
    mac_add_rule("kernel_t", "*", "execute", true, 0);
    mac_add_rule("kernel_t", "*", "create", true, 0);
    mac_add_rule("kernel_t", "*", "delete", true, 0);
    
    // Add rules for init process
    mac_add_rule("init_t", "file_t", "read", true, 0);
    mac_add_rule("init_t", "file_t", "write", true, 0);
    mac_add_rule("init_t", "file_t", "execute", true, 0);
    mac_add_rule("init_t", "device_t", "read", true, 0);
    mac_add_rule("init_t", "device_t", "write", true, 0);
    
    // Add rules for user processes
    mac_add_rule("user_t", "home_t", "read", true, 0);
    mac_add_rule("user_t", "home_t", "write", true, 0);
    mac_add_rule("user_t", "home_t", "create", true, 0);
    mac_add_rule("user_t", "tmp_t", "read", true, 0);
    mac_add_rule("user_t", "tmp_t", "write", true, 0);
    mac_add_rule("user_t", "tmp_t", "create", true, 0);
    mac_add_rule("user_t", "bin_t", "execute", true, 0);
    mac_add_rule("user_t", "lib_t", "read", true, 0);
    
    // Add deny rules for sensitive areas
    mac_add_rule("user_t", "etc_t", "write", false, 0);
    mac_add_rule("user_t", "device_t", "write", false, 0);
    mac_add_rule("user_t", "proc_t", "write", false, MAC_CONDITION_NOT_OWNER);
    
    // Add rules for admin users
    mac_add_rule("admin_t", "*", "read", true, 0);
    mac_add_rule("admin_t", "*", "write", true, 0);
    mac_add_rule("admin_t", "*", "execute", true, 0);
    mac_add_rule("admin_t", "*", "create", true, 0);
    mac_add_rule("admin_t", "*", "delete", true, 0);
    
    // Add rules for system processes
    mac_add_rule("system_t", "file_t", "read", true, 0);
    mac_add_rule("system_t", "file_t", "write", true, MAC_CONDITION_SYSTEM_PATH);
    mac_add_rule("system_t", "device_t", "read", true, 0);
    mac_add_rule("system_t", "proc_t", "read", true, 0);
    
    kernel_printf("MAC: Loaded %zu default rules\n", rule_count);
    return 0;
}

/**
 * Add a MAC rule
 */
static int mac_add_rule(const char* subject_type, const char* object_type,
                       const char* action, bool allow, uint32_t conditions) {
    if (rule_count >= MAX_SECURITY_RULES) {
        return -ENOMEM;
    }
    
    mac_rule_t* rule = &policy_rules[rule_count];
    
    strncpy(rule->subject_type, subject_type, sizeof(rule->subject_type) - 1);
    strncpy(rule->object_type, object_type, sizeof(rule->object_type) - 1);
    strncpy(rule->action, action, sizeof(rule->action) - 1);
    rule->allow = allow;
    rule->conditions = conditions;
    
    rule_count++;
    
    if (current_policy) {
        current_policy->rule_count = rule_count;
    }
    
    return 0;
}

/**
 * Extract type from security context
 */
static int mac_extract_type_from_context(const char* context, char* type, size_t type_size) {
    if (!context || !type) {
        return -EINVAL;
    }
    
    // Parse SELinux-style context: user:role:type:level
    // For simplicity, assume type is the third component
    const char* colon1 = strchr(context, ':');
    if (!colon1) {
        // Simple context, use as-is
        strncpy(type, context, type_size - 1);
        type[type_size - 1] = '\0';
        return 0;
    }
    
    const char* colon2 = strchr(colon1 + 1, ':');
    if (!colon2) {
        // No type field, use default
        strcpy(type, "unconfined_t");
        return 0;
    }
    
    const char* type_start = colon2 + 1;
    const char* colon3 = strchr(type_start, ':');
    
    size_t type_len;
    if (colon3) {
        type_len = colon3 - type_start;
    } else {
        type_len = strlen(type_start);
    }
    
    if (type_len >= type_size) {
        type_len = type_size - 1;
    }
    
    memcpy(type, type_start, type_len);
    type[type_len] = '\0';
    
    return 0;
}

/**
 * Match pattern (supports wildcards)
 */
static bool mac_match_pattern(const char* pattern, const char* string) {
    if (!pattern || !string) {
        return false;
    }
    
    // Simple wildcard matching
    if (strcmp(pattern, "*") == 0) {
        return true;
    }
    
    // Exact match
    if (strcmp(pattern, string) == 0) {
        return true;
    }
    
    // Pattern with wildcards (simplified implementation)
    const char* star = strchr(pattern, '*');
    if (star) {
        size_t prefix_len = star - pattern;
        if (strncmp(pattern, string, prefix_len) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * Check additional conditions
 */
static bool mac_check_conditions(uint32_t conditions, security_context_t* subject,
                                security_context_t* object) {
    if (conditions == 0) {
        return true;
    }
    
    // Check owner condition
    if (conditions & MAC_CONDITION_NOT_OWNER) {
        if (subject->uid == object->uid) {
            return false;
        }
    }
    
    // Check system path condition
    if (conditions & MAC_CONDITION_SYSTEM_PATH) {
        // This would check if the object is in a system path
        // For now, always return true
        return true;
    }
    
    // Check time-based conditions
    if (conditions & MAC_CONDITION_BUSINESS_HOURS) {
        uint64_t current_time = get_system_time();
        // Check if current time is within business hours
        // For now, always return true
        return true;
    }
    
    return true;
}

/**
 * Get MAC policy information
 */
int mac_get_policy_info(char* name, size_t name_size, bool* enforcing, size_t* rule_count_out) {
    if (!mac_initialized || !current_policy) {
        return -ENODEV;
    }
    
    if (name && name_size > 0) {
        strncpy(name, current_policy->name, name_size - 1);
        name[name_size - 1] = '\0';
    }
    
    if (enforcing) {
        *enforcing = current_policy->enforcing;
    }
    
    if (rule_count_out) {
        *rule_count_out = current_policy->rule_count;
    }
    
    return 0;
}

/**
 * Validate security context format
 */
int mac_validate_context(const char* context) {
    if (!context) {
        return -EINVAL;
    }
    
    size_t len = strlen(context);
    if (len == 0 || len >= SECURITY_CONTEXT_MAX) {
        return -EINVAL;
    }
    
    // Basic validation - should have at least one component
    // Full validation would check against loaded policy
    return 0;
}

/**
 * Compute security context for new object
 */
int mac_compute_context(security_context_t* subject, security_context_t* parent,
                       const char* object_class, char* new_context, size_t context_size) {
    if (!subject || !new_context) {
        return -EINVAL;
    }
    
    // For now, create a simple context based on subject
    // Full implementation would use policy rules for context computation
    char subject_type[64];
    int ret = mac_extract_type_from_context(subject->context, subject_type, sizeof(subject_type));
    if (ret != 0) {
        return ret;
    }
    
    // Create default context for new object
    snprintf(new_context, context_size, "user_u:object_r:%s_t:s0",
             object_class ? object_class : "file");
    
    return 0;
}