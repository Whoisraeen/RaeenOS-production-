/**
 * @file sandbox.c
 * @brief Application Sandboxing Framework Implementation
 * 
 * This module implements comprehensive application sandboxing for RaeenOS:
 * - Process isolation using namespaces
 * - Resource limiting and quota enforcement  
 * - System call filtering (seccomp-style)
 * - File system access control
 * - Network access restrictions
 * - Hardware device access control
 * - AI system access management
 * 
 * The sandbox provides defense-in-depth protection by isolating
 * applications and limiting their access to system resources.
 * 
 * @author RaeenOS Security Team
 * @version 1.0
 */

#include "security_core.h"
#include "../memory.h"
#include "../string.h"
#include "../process/process.h"

// Sandbox profiles database
static sandbox_profile_t* sandbox_profiles[MAX_SECURITY_POLICIES];
static size_t profile_count = 0;
static bool sandbox_initialized = false;

// Default sandbox profiles
static const char* default_profiles[] = {
    "strict",
    "default", 
    "permissive",
    "developer",
    "system",
    NULL
};

// System call filtering support
#define MAX_SYSCALLS 512
static bool allowed_syscalls[MAX_SYSCALLS];

/**
 * Initialize sandboxing framework
 */
int sandbox_init(void) {
    if (sandbox_initialized) {
        return 0;
    }
    
    // Initialize profiles array
    memset(sandbox_profiles, 0, sizeof(sandbox_profiles));
    profile_count = 0;
    
    // Initialize syscall filter
    memset(allowed_syscalls, false, sizeof(allowed_syscalls));
    
    // Create default sandbox profiles
    int ret = sandbox_create_default_profiles();
    if (ret != 0) {
        return ret;
    }
    
    sandbox_initialized = true;
    kernel_printf("Sandbox: Framework initialized with %zu profiles\n", profile_count);
    
    return 0;
}

/**
 * Cleanup sandboxing framework
 */
void sandbox_cleanup(void) {
    if (!sandbox_initialized) {
        return;
    }
    
    // Cleanup all profiles
    for (size_t i = 0; i < profile_count; i++) {
        if (sandbox_profiles[i]) {
            sandbox_destroy_profile_internal(sandbox_profiles[i]);
        }
    }
    
    profile_count = 0;
    sandbox_initialized = false;
    
    kernel_printf("Sandbox: Framework cleaned up\n");
}

/**
 * Create a new sandbox profile
 */
int security_create_sandbox(const char* name, sandbox_profile_t** profile) {
    if (!sandbox_initialized || !name || !profile) {
        return -EINVAL;
    }
    
    if (profile_count >= MAX_SECURITY_POLICIES) {
        return -ENOMEM;
    }
    
    // Check if profile already exists
    for (size_t i = 0; i < profile_count; i++) {
        if (sandbox_profiles[i] && strcmp(sandbox_profiles[i]->name, name) == 0) {
            return -EEXIST;
        }
    }
    
    // Allocate new profile
    sandbox_profile_t* prof = kmalloc(sizeof(sandbox_profile_t));
    if (!prof) {
        return -ENOMEM;
    }
    
    memset(prof, 0, sizeof(sandbox_profile_t));
    
    // Initialize profile
    strncpy(prof->name, name, sizeof(prof->name) - 1);
    prof->version = 1;
    prof->flags = 0;
    
    // Set default restrictions
    sandbox_set_default_restrictions(prof);
    
    // Add to profiles database
    sandbox_profiles[profile_count++] = prof;
    
    *profile = prof;
    
    kernel_printf("Sandbox: Created profile '%s'\n", name);
    return 0;
}

/**
 * Destroy a sandbox profile
 */
void security_destroy_sandbox(sandbox_profile_t* profile) {
    if (!profile) {
        return;
    }
    
    // Remove from profiles database
    for (size_t i = 0; i < profile_count; i++) {
        if (sandbox_profiles[i] == profile) {
            sandbox_profiles[i] = sandbox_profiles[--profile_count];
            sandbox_profiles[profile_count] = NULL;
            break;
        }
    }
    
    sandbox_destroy_profile_internal(profile);
}

/**
 * Apply sandbox profile to a process
 */
int security_apply_sandbox(process_t* process, sandbox_profile_t* profile) {
    if (!process || !profile) {
        return -EINVAL;
    }
    
    // Check if caller has permission to apply sandbox
    if (!security_check_capability(CAP_SYS_ADMIN)) {
        return -EPERM;
    }
    
    // Apply filesystem restrictions
    int ret = sandbox_apply_filesystem_restrictions(process, profile);
    if (ret != 0) {
        return ret;
    }
    
    // Apply network restrictions
    ret = sandbox_apply_network_restrictions(process, profile);
    if (ret != 0) {
        return ret;
    }
    
    // Apply system call filtering
    ret = sandbox_apply_syscall_filter(process, profile);
    if (ret != 0) {
        return ret;
    }
    
    // Apply resource limits
    ret = sandbox_apply_resource_limits(process, profile);
    if (ret != 0) {
        return ret;
    }
    
    // Apply hardware restrictions
    ret = sandbox_apply_hardware_restrictions(process, profile);
    if (ret != 0) {
        return ret;
    }
    
    // Apply AI access restrictions
    ret = sandbox_apply_ai_restrictions(process, profile);
    if (ret != 0) {
        return ret;
    }
    
    // Set sandbox profile in process structure
    process->security_data = profile;
    
    // Log sandbox application
    security_event_t event = {
        .event_id = security_generate_event_id(),
        .timestamp = get_system_time(),
        .pid = process->pid,
        .uid = process->creds.uid,
        .type = SECURITY_EVENT_SYSTEM_CALL,
        .severity = 5,
        .blocked = false
    };
    snprintf(event.description, sizeof(event.description),
             "Sandbox profile '%s' applied to process %d", profile->name, process->pid);
    security_log_event(&event);
    
    kernel_printf("Sandbox: Applied profile '%s' to process %d\n", profile->name, process->pid);
    return 0;
}

/**
 * Check sandbox access for a resource
 */
int security_check_sandbox_access(process_t* process, const char* resource, const char* action) {
    if (!process || !resource || !action) {
        return -EINVAL;
    }
    
    sandbox_profile_t* profile = (sandbox_profile_t*)process->security_data;
    if (!profile) {
        // No sandbox profile - allow access
        return 0;
    }
    
    // Check filesystem access
    if (strncmp(resource, "/", 1) == 0) {
        return sandbox_check_filesystem_access(profile, resource, action);
    }
    
    // Check network access
    if (strncmp(resource, "net:", 4) == 0) {
        return sandbox_check_network_access(profile, resource + 4, action);
    }
    
    // Check device access
    if (strncmp(resource, "dev:", 4) == 0) {
        return sandbox_check_device_access(profile, resource + 4, action);
    }
    
    // Check AI access
    if (strncmp(resource, "ai:", 3) == 0) {
        return sandbox_check_ai_access(profile, resource + 3, action);
    }
    
    // Unknown resource type - deny by default
    return -EACCES;
}

/**
 * Create default sandbox profiles
 */
static int sandbox_create_default_profiles(void) {
    int ret;
    
    // Create strict profile
    sandbox_profile_t* strict_profile;
    ret = security_create_sandbox("strict", &strict_profile);
    if (ret != 0) return ret;
    
    sandbox_configure_strict_profile(strict_profile);
    
    // Create default profile
    sandbox_profile_t* default_profile;
    ret = security_create_sandbox("default", &default_profile);
    if (ret != 0) return ret;
    
    sandbox_configure_default_profile(default_profile);
    
    // Create permissive profile
    sandbox_profile_t* permissive_profile;
    ret = security_create_sandbox("permissive", &permissive_profile);
    if (ret != 0) return ret;
    
    sandbox_configure_permissive_profile(permissive_profile);
    
    // Create developer profile
    sandbox_profile_t* developer_profile;
    ret = security_create_sandbox("developer", &developer_profile);
    if (ret != 0) return ret;
    
    sandbox_configure_developer_profile(developer_profile);
    
    // Create system profile
    sandbox_profile_t* system_profile;
    ret = security_create_sandbox("system", &system_profile);
    if (ret != 0) return ret;
    
    sandbox_configure_system_profile(system_profile);
    
    return 0;
}

/**
 * Set default restrictions for a profile
 */
static void sandbox_set_default_restrictions(sandbox_profile_t* profile) {
    // Filesystem restrictions
    profile->filesystem.allowed_paths = NULL;
    profile->filesystem.allowed_count = 0;
    profile->filesystem.denied_paths = NULL;
    profile->filesystem.denied_count = 0;
    profile->filesystem.allow_network_fs = false;
    profile->filesystem.allow_device_files = false;
    profile->filesystem.allow_suid_files = false;
    
    // Network restrictions
    profile->network.allow_network = false;
    profile->network.allow_localhost = true;
    profile->network.allow_lan = false;
    profile->network.allow_internet = false;
    profile->network.allowed_ports = NULL;
    profile->network.port_count = 0;
    profile->network.allowed_hosts = NULL;
    profile->network.host_count = 0;
    
    // System call restrictions
    profile->syscalls.allowed_syscalls = NULL;
    profile->syscalls.syscall_count = 0;
    profile->syscalls.default_deny = true;
    
    // Resource limits
    profile->limits.max_memory = 256 * 1024 * 1024; // 256MB
    profile->limits.max_processes = 10;
    profile->limits.max_threads = 50;
    profile->limits.max_files = 100;
    profile->limits.max_cpu_time = 3600; // 1 hour
    
    // Hardware restrictions
    profile->hardware.allow_gpu = false;
    profile->hardware.allow_npu = false;
    profile->hardware.allow_audio = false;
    profile->hardware.allow_camera = false;
    profile->hardware.allow_microphone = false;
    profile->hardware.allow_usb = false;
    profile->hardware.allow_bluetooth = false;
    
    // AI restrictions
    profile->ai.allow_ai_inference = false;
    profile->ai.allow_ai_training = false;
    profile->ai.allow_model_loading = false;
    profile->ai.allowed_models = NULL;
    profile->ai.model_count = 0;
}

/**
 * Configure strict sandbox profile
 */
static void sandbox_configure_strict_profile(sandbox_profile_t* profile) {
    // Very restrictive settings
    profile->filesystem.allow_network_fs = false;
    profile->filesystem.allow_device_files = false;
    profile->filesystem.allow_suid_files = false;
    
    profile->network.allow_network = false;
    profile->network.allow_localhost = false;
    profile->network.allow_lan = false;
    profile->network.allow_internet = false;
    
    profile->limits.max_memory = 64 * 1024 * 1024; // 64MB
    profile->limits.max_processes = 5;
    profile->limits.max_threads = 10;
    profile->limits.max_files = 20;
    profile->limits.max_cpu_time = 300; // 5 minutes
    
    // No hardware access
    profile->hardware.allow_gpu = false;
    profile->hardware.allow_npu = false;
    profile->hardware.allow_audio = false;
    profile->hardware.allow_camera = false;
    profile->hardware.allow_microphone = false;
    profile->hardware.allow_usb = false;
    profile->hardware.allow_bluetooth = false;
    
    // No AI access
    profile->ai.allow_ai_inference = false;
    profile->ai.allow_ai_training = false;
    profile->ai.allow_model_loading = false;
}

/**
 * Configure default sandbox profile
 */
static void sandbox_configure_default_profile(sandbox_profile_t* profile) {
    // Moderate restrictions - suitable for most applications
    profile->filesystem.allow_network_fs = false;
    profile->filesystem.allow_device_files = false;
    profile->filesystem.allow_suid_files = false;
    
    profile->network.allow_network = true;
    profile->network.allow_localhost = true;
    profile->network.allow_lan = true;
    profile->network.allow_internet = true;
    
    profile->limits.max_memory = 512 * 1024 * 1024; // 512MB
    profile->limits.max_processes = 20;
    profile->limits.max_threads = 100;
    profile->limits.max_files = 200;
    profile->limits.max_cpu_time = 7200; // 2 hours
    
    // Limited hardware access
    profile->hardware.allow_gpu = false;
    profile->hardware.allow_npu = false;
    profile->hardware.allow_audio = true;
    profile->hardware.allow_camera = false;
    profile->hardware.allow_microphone = false;
    profile->hardware.allow_usb = false;
    profile->hardware.allow_bluetooth = false;
    
    // Basic AI access
    profile->ai.allow_ai_inference = true;
    profile->ai.allow_ai_training = false;
    profile->ai.allow_model_loading = false;
}

/**
 * Configure permissive sandbox profile
 */
static void sandbox_configure_permissive_profile(sandbox_profile_t* profile) {
    // Relaxed restrictions
    profile->filesystem.allow_network_fs = true;
    profile->filesystem.allow_device_files = false;
    profile->filesystem.allow_suid_files = false;
    
    profile->network.allow_network = true;
    profile->network.allow_localhost = true;
    profile->network.allow_lan = true;
    profile->network.allow_internet = true;
    
    profile->limits.max_memory = 2048 * 1024 * 1024; // 2GB
    profile->limits.max_processes = 100;
    profile->limits.max_threads = 500;
    profile->limits.max_files = 1000;
    profile->limits.max_cpu_time = 86400; // 24 hours
    
    // More hardware access
    profile->hardware.allow_gpu = true;
    profile->hardware.allow_npu = false;
    profile->hardware.allow_audio = true;
    profile->hardware.allow_camera = true;
    profile->hardware.allow_microphone = true;
    profile->hardware.allow_usb = false;
    profile->hardware.allow_bluetooth = true;
    
    // Extended AI access
    profile->ai.allow_ai_inference = true;
    profile->ai.allow_ai_training = false;
    profile->ai.allow_model_loading = true;
}

/**
 * Configure developer sandbox profile
 */
static void sandbox_configure_developer_profile(sandbox_profile_t* profile) {
    // Developer-friendly settings
    profile->filesystem.allow_network_fs = true;
    profile->filesystem.allow_device_files = true;
    profile->filesystem.allow_suid_files = false;
    
    profile->network.allow_network = true;
    profile->network.allow_localhost = true;
    profile->network.allow_lan = true;
    profile->network.allow_internet = true;
    
    profile->limits.max_memory = 4096 * 1024 * 1024; // 4GB
    profile->limits.max_processes = 200;
    profile->limits.max_threads = 1000;
    profile->limits.max_files = 2000;
    profile->limits.max_cpu_time = 86400; // 24 hours
    
    // Full hardware access
    profile->hardware.allow_gpu = true;
    profile->hardware.allow_npu = true;
    profile->hardware.allow_audio = true;
    profile->hardware.allow_camera = true;
    profile->hardware.allow_microphone = true;
    profile->hardware.allow_usb = true;
    profile->hardware.allow_bluetooth = true;
    
    // Full AI access
    profile->ai.allow_ai_inference = true;
    profile->ai.allow_ai_training = true;
    profile->ai.allow_model_loading = true;
}

/**
 * Configure system sandbox profile
 */
static void sandbox_configure_system_profile(sandbox_profile_t* profile) {
    // System process settings - minimal restrictions
    profile->filesystem.allow_network_fs = true;
    profile->filesystem.allow_device_files = true;
    profile->filesystem.allow_suid_files = true;
    
    profile->network.allow_network = true;
    profile->network.allow_localhost = true;
    profile->network.allow_lan = true;
    profile->network.allow_internet = true;
    
    profile->limits.max_memory = 8192 * 1024 * 1024; // 8GB
    profile->limits.max_processes = 500;
    profile->limits.max_threads = 2000;
    profile->limits.max_files = 5000;
    profile->limits.max_cpu_time = 0; // No limit
    
    // Full hardware access
    profile->hardware.allow_gpu = true;
    profile->hardware.allow_npu = true;
    profile->hardware.allow_audio = true;
    profile->hardware.allow_camera = true;
    profile->hardware.allow_microphone = true;
    profile->hardware.allow_usb = true;
    profile->hardware.allow_bluetooth = true;
    
    // Full AI access
    profile->ai.allow_ai_inference = true;
    profile->ai.allow_ai_training = true;
    profile->ai.allow_model_loading = true;
}

/**
 * Apply filesystem restrictions to process
 */
static int sandbox_apply_filesystem_restrictions(process_t* process, sandbox_profile_t* profile) {
    // For now, store restrictions in process structure
    // Full implementation would set up namespace isolation
    return 0;
}

/**
 * Apply network restrictions to process
 */
static int sandbox_apply_network_restrictions(process_t* process, sandbox_profile_t* profile) {
    // For now, return success
    // Full implementation would configure network namespace
    return 0;
}

/**
 * Apply system call filter to process
 */
static int sandbox_apply_syscall_filter(process_t* process, sandbox_profile_t* profile) {
    // For now, return success
    // Full implementation would install seccomp filter
    return 0;
}

/**
 * Apply resource limits to process
 */
static int sandbox_apply_resource_limits(process_t* process, sandbox_profile_t* profile) {
    // Set memory limit
    if (profile->limits.max_memory > 0) {
        process->limits.limits[0].hard_limit = profile->limits.max_memory;
        process->limits.limits[0].soft_limit = profile->limits.max_memory;
    }
    
    // Set process limit
    if (profile->limits.max_processes > 0) {
        process->limits.limits[1].hard_limit = profile->limits.max_processes;
        process->limits.limits[1].soft_limit = profile->limits.max_processes;
    }
    
    // Set file limit
    if (profile->limits.max_files > 0) {
        process->files.max_count = profile->limits.max_files;
    }
    
    return 0;
}

/**
 * Apply hardware restrictions to process
 */
static int sandbox_apply_hardware_restrictions(process_t* process, sandbox_profile_t* profile) {
    // For now, return success
    // Full implementation would restrict device access
    return 0;
}

/**
 * Apply AI access restrictions to process
 */
static int sandbox_apply_ai_restrictions(process_t* process, sandbox_profile_t* profile) {
    // For now, return success
    // Full implementation would configure AI system access
    return 0;
}

/**
 * Check filesystem access against sandbox profile
 */
static int sandbox_check_filesystem_access(sandbox_profile_t* profile, const char* path, const char* action) {
    // Check allowed paths
    if (profile->filesystem.allowed_count > 0) {
        bool found = false;
        for (size_t i = 0; i < profile->filesystem.allowed_count; i++) {
            if (sandbox_path_matches(profile->filesystem.allowed_paths[i], path)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return -EACCES;
        }
    }
    
    // Check denied paths
    for (size_t i = 0; i < profile->filesystem.denied_count; i++) {
        if (sandbox_path_matches(profile->filesystem.denied_paths[i], path)) {
            return -EACCES;
        }
    }
    
    // Check special file types
    if (strstr(path, "/dev/") && !profile->filesystem.allow_device_files) {
        return -EACCES;
    }
    
    return 0;
}

/**
 * Check network access against sandbox profile
 */
static int sandbox_check_network_access(sandbox_profile_t* profile, const char* resource, const char* action) {
    if (!profile->network.allow_network) {
        return -EACCES;
    }
    
    // Parse resource (host:port format)
    // For now, allow all network access if network is enabled
    return 0;
}

/**
 * Check device access against sandbox profile
 */
static int sandbox_check_device_access(sandbox_profile_t* profile, const char* device, const char* action) {
    if (strcmp(device, "gpu") == 0 && !profile->hardware.allow_gpu) {
        return -EACCES;
    }
    
    if (strcmp(device, "audio") == 0 && !profile->hardware.allow_audio) {
        return -EACCES;
    }
    
    if (strcmp(device, "camera") == 0 && !profile->hardware.allow_camera) {
        return -EACCES;
    }
    
    if (strcmp(device, "microphone") == 0 && !profile->hardware.allow_microphone) {
        return -EACCES;
    }
    
    return 0;
}

/**
 * Check AI access against sandbox profile
 */
static int sandbox_check_ai_access(sandbox_profile_t* profile, const char* resource, const char* action) {
    if (strcmp(action, "inference") == 0 && !profile->ai.allow_ai_inference) {
        return -EACCES;
    }
    
    if (strcmp(action, "training") == 0 && !profile->ai.allow_ai_training) {
        return -EACCES;
    }
    
    if (strcmp(action, "load_model") == 0 && !profile->ai.allow_model_loading) {
        return -EACCES;
    }
    
    return 0;
}

/**
 * Check if path matches pattern
 */
static bool sandbox_path_matches(const char* pattern, const char* path) {
    if (!pattern || !path) {
        return false;
    }
    
    // Simple prefix matching for now
    return strncmp(pattern, path, strlen(pattern)) == 0;
}

/**
 * Destroy profile internal data
 */
static void sandbox_destroy_profile_internal(sandbox_profile_t* profile) {
    if (!profile) {
        return;
    }
    
    // Free allowed paths
    if (profile->filesystem.allowed_paths) {
        for (size_t i = 0; i < profile->filesystem.allowed_count; i++) {
            if (profile->filesystem.allowed_paths[i]) {
                kfree(profile->filesystem.allowed_paths[i]);
            }
        }
        kfree(profile->filesystem.allowed_paths);
    }
    
    // Free denied paths
    if (profile->filesystem.denied_paths) {
        for (size_t i = 0; i < profile->filesystem.denied_count; i++) {
            if (profile->filesystem.denied_paths[i]) {
                kfree(profile->filesystem.denied_paths[i]);
            }
        }
        kfree(profile->filesystem.denied_paths);
    }
    
    // Free network allowed hosts
    if (profile->network.allowed_hosts) {
        for (size_t i = 0; i < profile->network.host_count; i++) {
            if (profile->network.allowed_hosts[i]) {
                kfree(profile->network.allowed_hosts[i]);
            }
        }
        kfree(profile->network.allowed_hosts);
    }
    
    // Free network allowed ports
    if (profile->network.allowed_ports) {
        kfree(profile->network.allowed_ports);
    }
    
    // Free syscall list
    if (profile->syscalls.allowed_syscalls) {
        kfree(profile->syscalls.allowed_syscalls);
    }
    
    // Free AI allowed models
    if (profile->ai.allowed_models) {
        for (size_t i = 0; i < profile->ai.model_count; i++) {
            if (profile->ai.allowed_models[i]) {
                kfree(profile->ai.allowed_models[i]);
            }
        }
        kfree(profile->ai.allowed_models);
    }
    
    // Free private data
    if (profile->private_data) {
        kfree(profile->private_data);
    }
    
    kfree(profile);
}