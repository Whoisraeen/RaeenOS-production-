/**
 * RaeCompat Core Implementation
 * Revolutionary Windows Compatibility Layer for RaeenOS
 */

#include "raecompat_core.h"
#include "../kernel/memory.h"
#include "../kernel/string.h"
#include "../userland/include/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

// ============================================================================
// INTERNAL STRUCTURES
// ============================================================================

struct RaeCompatContext {
    // System information
    RaeCompatSystemInfo* system_info;
    
    // Installed Wine versions
    char** wine_versions;
    int wine_version_count;
    
    // Active prefixes
    RaeCompatPrefix** prefixes;
    int prefix_count;
    int prefix_capacity;
    
    // Registered applications
    RaeCompatApplication** applications;
    int app_count;
    int app_capacity;
    
    // Running processes
    RaeCompatProcessInfo** processes;
    int process_count;
    int process_capacity;
    
    // Configuration
    char* config_directory;
    char* wine_directory;
    char* prefix_directory;
    
    // Performance monitoring
    bool performance_monitoring_enabled;
    bool mangohud_enabled;
    bool gamemode_enabled;
    
    // Debug settings
    RaeCompatLogLevel log_level;
    FILE* log_file;
};

struct RaeCompatPrefix {
    RaeCompatPrefixConfig config;
    char* wine_executable;
    char* wineserver_executable;
    char* winetricks_executable;
    bool is_initialized;
    time_t last_used;
};

struct RaeCompatApplication {
    RaeCompatAppConfig config;
    RaeCompatPrefix* prefix;
    RaeCompatProcessInfo* process_info;
    bool is_favorite;
    int launch_count;
    time_t last_launched;
};

// ============================================================================
// CONSTANTS AND DEFAULTS
// ============================================================================

#define RAECOMPAT_CONFIG_DIR "/home/.raecompat"
#define RAECOMPAT_PREFIX_DIR "/home/.raecompat/prefixes"
#define RAECOMPAT_WINE_DIR "/usr/lib/raecompat/wine"
#define RAECOMPAT_LOGS_DIR "/home/.raecompat/logs"

#define RAECOMPAT_DEFAULT_WINDOWS_VERSION "win10"
#define RAECOMPAT_DEFAULT_WINE_VERSION RAECOMPAT_WINE_STAGING

// Wine DLL override modes
static const char* DLL_OVERRIDE_MODES[] = {
    "native",
    "builtin",
    "native,builtin",
    "builtin,native",
    "disabled"
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static void raecompat_create_directory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

static char* raecompat_join_path(const char* base, const char* relative) {
    size_t base_len = strlen(base);
    size_t rel_len = strlen(relative);
    char* result = (char*)malloc(base_len + rel_len + 2);
    
    strcpy(result, base);
    if (base[base_len - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, relative);
    
    return result;
}

static bool raecompat_file_exists(const char* path) {
    return access(path, F_OK) == 0;
}

static bool raecompat_is_executable(const char* path) {
    return access(path, X_OK) == 0;
}

// ============================================================================
// LOGGING SYSTEM
// ============================================================================

static RaeCompatLogLevel global_log_level = RAECOMPAT_LOG_INFO;
static FILE* global_log_file = NULL;

void raecompat_set_log_level(RaeCompatLogLevel level) {
    global_log_level = level;
}

void raecompat_log(RaeCompatLogLevel level, const char* format, ...) {
    if (level > global_log_level) return;
    
    const char* level_str[] = {"ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
    
    va_list args;
    va_start(args, format);
    
    // Log to console
    printf("[RAECOMPAT %s] ", level_str[level]);
    vprintf(format, args);
    printf("\n");
    
    // Log to file if available
    if (global_log_file) {
        fprintf(global_log_file, "[RAECOMPAT %s] ", level_str[level]);
        vfprintf(global_log_file, format, args);
        fprintf(global_log_file, "\n");
        fflush(global_log_file);
    }
    
    va_end(args);
}

// ============================================================================
// CORE FRAMEWORK IMPLEMENTATION
// ============================================================================

RaeCompatContext* raecompat_init(void) {
    RaeCompatContext* ctx = (RaeCompatContext*)malloc(sizeof(RaeCompatContext));
    if (!ctx) return NULL;
    
    // Initialize structure
    memset(ctx, 0, sizeof(RaeCompatContext));
    
    // Set up directories
    ctx->config_directory = strdup(RAECOMPAT_CONFIG_DIR);
    ctx->wine_directory = strdup(RAECOMPAT_WINE_DIR);
    ctx->prefix_directory = strdup(RAECOMPAT_PREFIX_DIR);
    
    // Create directories
    raecompat_create_directory(ctx->config_directory);
    raecompat_create_directory(ctx->prefix_directory);
    raecompat_create_directory(RAECOMPAT_LOGS_DIR);
    
    // Initialize arrays
    ctx->prefix_capacity = 16;
    ctx->prefixes = (RaeCompatPrefix**)malloc(sizeof(RaeCompatPrefix*) * ctx->prefix_capacity);
    
    ctx->app_capacity = 64;
    ctx->applications = (RaeCompatApplication**)malloc(sizeof(RaeCompatApplication*) * ctx->app_capacity);
    
    ctx->process_capacity = 32;
    ctx->processes = (RaeCompatProcessInfo**)malloc(sizeof(RaeCompatProcessInfo*) * ctx->process_capacity);
    
    // Detect system capabilities
    ctx->system_info = raecompat_detect_system();
    
    // Set up logging
    char* log_path = raecompat_join_path(RAECOMPAT_LOGS_DIR, "raecompat.log");
    global_log_file = fopen(log_path, "a");
    free(log_path);
    
    raecompat_log(RAECOMPAT_LOG_INFO, "RaeCompat initialization complete");
    
    return ctx;
}

void raecompat_shutdown(RaeCompatContext* ctx) {
    if (!ctx) return;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Shutting down RaeCompat");
    
    // Cleanup prefixes
    for (int i = 0; i < ctx->prefix_count; i++) {
        // TODO: Cleanup prefix resources
        free(ctx->prefixes[i]);
    }
    free(ctx->prefixes);
    
    // Cleanup applications
    for (int i = 0; i < ctx->app_count; i++) {
        // TODO: Cleanup application resources
        free(ctx->applications[i]);
    }
    free(ctx->applications);
    
    // Cleanup processes
    for (int i = 0; i < ctx->process_count; i++) {
        free(ctx->processes[i]);
    }
    free(ctx->processes);
    
    // Cleanup strings
    free(ctx->config_directory);
    free(ctx->wine_directory);
    free(ctx->prefix_directory);
    
    // Cleanup system info
    if (ctx->system_info) {
        free(ctx->system_info->gpu_vendor);
        free(ctx->system_info->gpu_model);
        free(ctx->system_info->cpu_model);
        free(ctx->system_info);
    }
    
    // Close log file
    if (global_log_file) {
        fclose(global_log_file);
        global_log_file = NULL;
    }
    
    free(ctx);
}

RaeCompatSystemInfo* raecompat_detect_system(void) {
    RaeCompatSystemInfo* info = (RaeCompatSystemInfo*)malloc(sizeof(RaeCompatSystemInfo));
    if (!info) return NULL;
    
    memset(info, 0, sizeof(RaeCompatSystemInfo));
    
    // Detect Vulkan
    info->vulkan_available = raecompat_file_exists("/usr/lib/libvulkan.so") ||
                            raecompat_file_exists("/usr/lib64/libvulkan.so");
    
    // Detect OpenGL
    info->opengl_available = raecompat_file_exists("/usr/lib/libGL.so") ||
                            raecompat_file_exists("/usr/lib64/libGL.so");
    
    // Get CPU information
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strncmp(line, "processor", 9) == 0) {
                info->cpu_cores++;
            } else if (strncmp(line, "model name", 10) == 0) {
                char* colon = strchr(line, ':');
                if (colon && !info->cpu_model) {
                    info->cpu_model = strdup(colon + 2);
                    // Remove newline
                    char* newline = strchr(info->cpu_model, '\n');
                    if (newline) *newline = '\0';
                }
            }
        }
        fclose(cpuinfo);
    }
    
    // Get memory information
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            if (strncmp(line, "MemTotal:", 9) == 0) {
                int mem_kb;
                sscanf(line, "MemTotal: %d kB", &mem_kb);
                info->system_memory_mb = mem_kb / 1024;
                break;
            }
        }
        fclose(meminfo);
    }
    
    // GPU detection (simplified - would need more sophisticated detection)
    // Check for NVIDIA
    if (raecompat_file_exists("/proc/driver/nvidia/version")) {
        info->gpu_vendor = strdup("NVIDIA");
    }
    // Check for AMD
    else if (raecompat_file_exists("/sys/class/drm/card0/device/vendor") &&
             raecompat_file_exists("/sys/class/drm/card0/device/device")) {
        FILE* vendor_file = fopen("/sys/class/drm/card0/device/vendor", "r");
        if (vendor_file) {
            char vendor_id[16];
            fgets(vendor_id, sizeof(vendor_id), vendor_file);
            fclose(vendor_file);
            
            if (strstr(vendor_id, "0x1002")) {
                info->gpu_vendor = strdup("AMD");
            } else if (strstr(vendor_id, "0x8086")) {
                info->gpu_vendor = strdup("Intel");
            }
        }
    }
    
    if (!info->gpu_vendor) {
        info->gpu_vendor = strdup("Unknown");
    }
    if (!info->gpu_model) {
        info->gpu_model = strdup("Unknown");
    }
    
    raecompat_log(RAECOMPAT_LOG_INFO, "System detected: %s %s, %d cores, %dMB RAM, Vulkan: %s, OpenGL: %s",
                  info->gpu_vendor, info->gpu_model, info->cpu_cores, info->system_memory_mb,
                  info->vulkan_available ? "yes" : "no", info->opengl_available ? "yes" : "no");
    
    return info;
}

bool raecompat_setup_environment(RaeCompatContext* ctx) {
    if (!ctx) return false;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Setting up RaeCompat environment");
    
    // Create necessary directories
    raecompat_create_directory(ctx->wine_directory);
    
    // Set environment variables
    setenv("RAECOMPAT_PREFIX_DIR", ctx->prefix_directory, 1);
    setenv("RAECOMPAT_WINE_DIR", ctx->wine_directory, 1);
    
    // Configure Wine environment
    setenv("WINEARCH", "win64", 1);
    setenv("WINEPREFIX", ctx->prefix_directory, 1);
    
    // Performance optimizations
    if (ctx->system_info->vulkan_available) {
        setenv("VKD3D_CONFIG", "dxr", 1);
        setenv("DXVK_HUD", "fps,memory,gpuload", 1);
    }
    
    return true;
}

bool raecompat_install_wine(RaeCompatContext* ctx, RaeCompatWineVersion version) {
    if (!ctx) return false;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Installing Wine version %d", version);
    
    // TODO: Implement Wine installation
    // This would typically involve:
    // 1. Downloading Wine binaries or compiling from source
    // 2. Installing to ctx->wine_directory
    // 3. Setting up wine prefix
    // 4. Registering the installation
    
    return true; // Placeholder
}

bool raecompat_install_dxvk(RaeCompatContext* ctx) {
    if (!ctx || !ctx->system_info->vulkan_available) {
        raecompat_log(RAECOMPAT_LOG_ERROR, "DXVK requires Vulkan support");
        return false;
    }
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Installing DXVK");
    
    // TODO: Implement DXVK installation
    // 1. Download latest DXVK release
    // 2. Extract to wine directory
    // 3. Set up DLL overrides
    
    return true; // Placeholder
}

bool raecompat_install_vkd3d(RaeCompatContext* ctx) {
    if (!ctx || !ctx->system_info->vulkan_available) {
        raecompat_log(RAECOMPAT_LOG_ERROR, "VKD3D requires Vulkan support");
        return false;
    }
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Installing VKD3D-Proton");
    
    // TODO: Implement VKD3D installation similar to DXVK
    
    return true; // Placeholder
}

// ============================================================================
// PREFIX MANAGEMENT
// ============================================================================

RaeCompatPrefix* raecompat_create_prefix(RaeCompatContext* ctx, const char* name) {
    if (!ctx || !name) return NULL;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Creating Wine prefix: %s", name);
    
    // Check if prefix already exists
    for (int i = 0; i < ctx->prefix_count; i++) {
        if (strcmp(ctx->prefixes[i]->config.name, name) == 0) {
            raecompat_log(RAECOMPAT_LOG_WARNING, "Prefix %s already exists", name);
            return ctx->prefixes[i];
        }
    }
    
    // Create prefix structure
    RaeCompatPrefix* prefix = (RaeCompatPrefix*)malloc(sizeof(RaeCompatPrefix));
    if (!prefix) return NULL;
    
    memset(prefix, 0, sizeof(RaeCompatPrefix));
    
    // Initialize configuration
    prefix->config.name = strdup(name);
    prefix->config.path = raecompat_join_path(ctx->prefix_directory, name);
    prefix->config.windows_version = strdup(RAECOMPAT_DEFAULT_WINDOWS_VERSION);
    prefix->config.wine_version = RAECOMPAT_DEFAULT_WINE_VERSION;
    prefix->config.dx_mode = RAECOMPAT_DX_AUTO;
    
    // Create prefix directory
    raecompat_create_directory(prefix->config.path);
    
    // Add to context
    if (ctx->prefix_count >= ctx->prefix_capacity) {
        ctx->prefix_capacity *= 2;
        ctx->prefixes = (RaeCompatPrefix**)realloc(ctx->prefixes, 
            sizeof(RaeCompatPrefix*) * ctx->prefix_capacity);
    }
    
    ctx->prefixes[ctx->prefix_count++] = prefix;
    
    return prefix;
}

bool raecompat_configure_prefix(RaeCompatPrefix* prefix, RaeCompatPrefixConfig* config) {
    if (!prefix || !config) return false;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Configuring prefix: %s", prefix->config.name);
    
    // Update configuration
    if (config->windows_version) {
        free(prefix->config.windows_version);
        prefix->config.windows_version = strdup(config->windows_version);
    }
    
    prefix->config.wine_version = config->wine_version;
    prefix->config.dx_mode = config->dx_mode;
    prefix->config.performance = config->performance;
    
    // Apply DLL overrides
    for (int i = 0; i < config->dll_override_count; i++) {
        // TODO: Apply DLL override using winecfg or registry
    }
    
    // Apply registry settings
    for (int i = 0; i < config->registry_count; i++) {
        // TODO: Apply registry setting using regedit
    }
    
    // Run winetricks
    for (int i = 0; i < config->winetricks_count; i++) {
        raecompat_run_winetricks(prefix, config->winetricks_verbs[i]);
    }
    
    return true;
}

bool raecompat_run_winetricks(RaeCompatPrefix* prefix, const char* verb) {
    if (!prefix || !verb) return false;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Running winetricks %s for prefix %s", verb, prefix->config.name);
    
    // Build winetricks command
    char command[1024];
    snprintf(command, sizeof(command), 
        "WINEPREFIX=%s winetricks -q %s", prefix->config.path, verb);
    
    // Execute command
    int result = system(command);
    
    if (result == 0) {
        raecompat_log(RAECOMPAT_LOG_INFO, "Winetricks %s completed successfully", verb);
        return true;
    } else {
        raecompat_log(RAECOMPAT_LOG_ERROR, "Winetricks %s failed with code %d", verb, result);
        return false;
    }
}

bool raecompat_set_dll_override(RaeCompatPrefix* prefix, const char* dll, const char* mode) {
    if (!prefix || !dll || !mode) return false;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Setting DLL override: %s = %s", dll, mode);
    
    // Use winecfg to set DLL override
    char command[512];
    snprintf(command, sizeof(command),
        "WINEPREFIX=%s winecfg /v %s=%s", prefix->config.path, dll, mode);
    
    return system(command) == 0;
}

// ============================================================================
// APPLICATION MANAGEMENT
// ============================================================================

RaeCompatApplication* raecompat_register_application(RaeCompatContext* ctx, RaeCompatAppConfig* config) {
    if (!ctx || !config) return NULL;
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Registering application: %s", config->name);
    
    // Create application structure
    RaeCompatApplication* app = (RaeCompatApplication*)malloc(sizeof(RaeCompatApplication));
    if (!app) return NULL;
    
    memset(app, 0, sizeof(RaeCompatApplication));
    app->config = *config;
    
    // Find associated prefix
    for (int i = 0; i < ctx->prefix_count; i++) {
        if (strcmp(ctx->prefixes[i]->config.name, config->prefix_name) == 0) {
            app->prefix = ctx->prefixes[i];
            break;
        }
    }
    
    // Add to context
    if (ctx->app_count >= ctx->app_capacity) {
        ctx->app_capacity *= 2;
        ctx->applications = (RaeCompatApplication**)realloc(ctx->applications,
            sizeof(RaeCompatApplication*) * ctx->app_capacity);
    }
    
    ctx->applications[ctx->app_count++] = app;
    
    return app;
}

RaeCompatProcessInfo* raecompat_launch_application(RaeCompatContext* ctx, const char* app_name) {
    if (!ctx || !app_name) return NULL;
    
    // Find application
    RaeCompatApplication* app = raecompat_find_application(ctx, app_name);
    if (!app) {
        raecompat_log(RAECOMPAT_LOG_ERROR, "Application not found: %s", app_name);
        return NULL;
    }
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Launching application: %s", app_name);
    
    // Build wine command
    char command[2048];
    snprintf(command, sizeof(command), 
        "WINEPREFIX=%s wine %s %s",
        app->prefix->config.path,
        app->config.executable_path,
        app->config.arguments ? app->config.arguments : "");
    
    // Create process info
    RaeCompatProcessInfo* process = (RaeCompatProcessInfo*)malloc(sizeof(RaeCompatProcessInfo));
    if (!process) return NULL;
    
    memset(process, 0, sizeof(RaeCompatProcessInfo));
    
    // Fork and execute
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (app->config.working_directory) {
            chdir(app->config.working_directory);
        }
        
        // Execute wine
        execl("/bin/sh", "sh", "-c", command, (char*)NULL);
        exit(1); // Should not reach here
    } else if (pid > 0) {
        // Parent process
        process->process_id = pid;
        process->is_running = true;
        
        // Add to process list
        if (ctx->process_count >= ctx->process_capacity) {
            ctx->process_capacity *= 2;
            ctx->processes = (RaeCompatProcessInfo**)realloc(ctx->processes,
                sizeof(RaeCompatProcessInfo*) * ctx->process_capacity);
        }
        
        ctx->processes[ctx->process_count++] = process;
        app->process_info = process;
        app->launch_count++;
        app->last_launched = time(NULL);
        
        raecompat_log(RAECOMPAT_LOG_INFO, "Application launched with PID: %d", pid);
        return process;
    } else {
        // Fork failed
        raecompat_log(RAECOMPAT_LOG_ERROR, "Failed to launch application: %s", strerror(errno));
        free(process);
        return NULL;
    }
}

RaeCompatApplication* raecompat_find_application(RaeCompatContext* ctx, const char* name) {
    if (!ctx || !name) return NULL;
    
    for (int i = 0; i < ctx->app_count; i++) {
        if (strcmp(ctx->applications[i]->config.name, name) == 0) {
            return ctx->applications[i];
        }
    }
    
    return NULL;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

RaeCompatPerformanceStats* raecompat_get_performance_stats(RaeCompatContext* ctx, pid_t process_id) {
    if (!ctx) return NULL;
    
    RaeCompatPerformanceStats* stats = (RaeCompatPerformanceStats*)malloc(sizeof(RaeCompatPerformanceStats));
    if (!stats) return NULL;
    
    memset(stats, 0, sizeof(RaeCompatPerformanceStats));
    
    // Read /proc/[pid]/stat for CPU usage
    char stat_path[64];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", process_id);
    
    FILE* stat_file = fopen(stat_path, "r");
    if (stat_file) {
        // Parse CPU times (simplified)
        unsigned long utime, stime;
        fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
        
        // Calculate CPU usage (would need more sophisticated calculation)
        stats->cpu_usage = (float)(utime + stime) / 100.0f; // Rough estimate
        
        fclose(stat_file);
    }
    
    // Read /proc/[pid]/status for memory usage
    char status_path[64];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", process_id);
    
    FILE* status_file = fopen(status_path, "r");
    if (status_file) {
        char line[256];
        while (fgets(line, sizeof(line), status_file)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                int memory_kb;
                sscanf(line, "VmRSS: %d kB", &memory_kb);
                stats->memory_usage_mb = (float)memory_kb / 1024.0f;
                break;
            }
        }
        fclose(status_file);
    }
    
    // TODO: Implement GPU usage monitoring
    // TODO: Implement FPS monitoring
    // TODO: Implement disk/network I/O monitoring
    
    return stats;
}

bool raecompat_enable_mangohud(RaeCompatContext* ctx, bool enable) {
    if (!ctx) return false;
    
    ctx->mangohud_enabled = enable;
    
    if (enable) {
        setenv("MANGOHUD", "1", 1);
        setenv("MANGOHUD_CONFIG", "fps,frametime,cpu_temp,gpu_temp", 1);
        raecompat_log(RAECOMPAT_LOG_INFO, "MangoHUD enabled");
    } else {
        unsetenv("MANGOHUD");
        raecompat_log(RAECOMPAT_LOG_INFO, "MangoHUD disabled");
    }
    
    return true;
}

bool raecompat_enable_gamemode(RaeCompatContext* ctx, bool enable) {
    if (!ctx) return false;
    
    ctx->gamemode_enabled = enable;
    
    if (enable) {
        // Check if gamemode is available
        if (raecompat_is_executable("/usr/bin/gamemoderun")) {
            raecompat_log(RAECOMPAT_LOG_INFO, "GameMode enabled");
            return true;
        } else {
            raecompat_log(RAECOMPAT_LOG_WARNING, "GameMode not available");
            return false;
        }
    } else {
        raecompat_log(RAECOMPAT_LOG_INFO, "GameMode disabled");
        return true;
    }
}

// ============================================================================
// DIAGNOSTICS
// ============================================================================

RaeCompatDiagnostics* raecompat_run_diagnostics(RaeCompatContext* ctx) {
    if (!ctx) return NULL;
    
    RaeCompatDiagnostics* diag = (RaeCompatDiagnostics*)malloc(sizeof(RaeCompatDiagnostics));
    if (!diag) return NULL;
    
    memset(diag, 0, sizeof(RaeCompatDiagnostics));
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Running system diagnostics");
    
    // Check Wine installation
    diag->wine_installed = raecompat_is_executable("/usr/bin/wine") ||
                          raecompat_is_executable("/usr/local/bin/wine");
    
    // Check DXVK installation
    diag->dxvk_installed = raecompat_file_exists("/usr/lib/wine/dxvk") ||
                          raecompat_file_exists("/usr/local/lib/wine/dxvk");
    
    // Check VKD3D installation
    diag->vkd3d_installed = raecompat_file_exists("/usr/lib/wine/vkd3d") ||
                           raecompat_file_exists("/usr/local/lib/wine/vkd3d");
    
    // Check Vulkan
    diag->vulkan_working = ctx->system_info->vulkan_available;
    
    // Check OpenGL
    diag->opengl_working = ctx->system_info->opengl_available;
    
    // Get versions (simplified)
    if (diag->wine_installed) {
        FILE* wine_version = popen("wine --version 2>/dev/null", "r");
        if (wine_version) {
            char version[64];
            if (fgets(version, sizeof(version), wine_version)) {
                // Remove newline
                char* newline = strchr(version, '\n');
                if (newline) *newline = '\0';
                diag->wine_version = strdup(version);
            }
            pclose(wine_version);
        }
    }
    
    raecompat_log(RAECOMPAT_LOG_INFO, "Diagnostics complete - Wine: %s, DXVK: %s, VKD3D: %s",
                  diag->wine_installed ? "OK" : "MISSING",
                  diag->dxvk_installed ? "OK" : "MISSING",
                  diag->vkd3d_installed ? "OK" : "MISSING");
    
    return diag;
}

// ============================================================================
// UTILITY IMPLEMENTATIONS
// ============================================================================

char* raecompat_get_prefix_path(const char* prefix_name) {
    return raecompat_join_path(RAECOMPAT_PREFIX_DIR, prefix_name);
}

bool raecompat_is_process_running(pid_t pid) {
    return kill(pid, 0) == 0;
}

bool raecompat_kill_process_tree(pid_t pid) {
    // Kill process and all children
    char command[64];
    snprintf(command, sizeof(command), "pkill -TERM -P %d", pid);
    system(command);
    
    // Kill the main process
    return kill(pid, SIGTERM) == 0;
}

uint64_t raecompat_get_available_memory(void) {
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo) return 0;
    
    uint64_t available = 0;
    char line[256];
    while (fgets(line, sizeof(line), meminfo)) {
        if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %lu kB", &available);
            break;
        }
    }
    fclose(meminfo);
    
    return available * 1024; // Convert to bytes
}

int raecompat_get_cpu_count(void) {
    return sysconf(_SC_NPROCESSORS_ONLN);
}