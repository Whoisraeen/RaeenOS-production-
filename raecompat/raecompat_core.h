#pragma once

/**
 * RaeCompat - Revolutionary Windows Compatibility Layer for RaeenOS
 * 
 * A comprehensive Wine/Proton-based compatibility system that allows
 * Windows applications and games to run seamlessly on RaeenOS with
 * near-native performance and excellent compatibility.
 * 
 * Features:
 * - Wine integration with custom patches for gaming
 * - DXVK/VKD3D-Proton for DirectX → Vulkan translation
 * - Automatic game detection and configuration
 * - Per-application Wine prefixes
 * - Performance optimization and monitoring
 * - Anti-cheat compatibility (EAC, BattlEye where possible)
 * - Steam, Epic, GOG launcher support
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CORE TYPES AND STRUCTURES
// ============================================================================

typedef struct RaeCompatContext RaeCompatContext;
typedef struct RaeCompatPrefix RaeCompatPrefix;
typedef struct RaeCompatApplication RaeCompatApplication;
typedef struct RaeCompatConfig RaeCompatConfig;

// Application types for optimization
typedef enum {
    RAECOMPAT_APP_UNKNOWN,
    RAECOMPAT_APP_GAME,
    RAECOMPAT_APP_LAUNCHER,   // Steam, Epic, etc.
    RAECOMPAT_APP_PRODUCTIVITY,
    RAECOMPAT_APP_UTILITY,
    RAECOMPAT_APP_ANTICHEAT   // Special handling needed
} RaeCompatAppType;

// Wine version management
typedef enum {
    RAECOMPAT_WINE_STABLE,
    RAECOMPAT_WINE_STAGING,
    RAECOMPAT_WINE_PROTON_GE,
    RAECOMPAT_WINE_LUTRIS,
    RAECOMPAT_WINE_CUSTOM
} RaeCompatWineVersion;

// DirectX translation options
typedef enum {
    RAECOMPAT_DX_NATIVE,      // Use native DirectX (slower)
    RAECOMPAT_DX_DXVK,        // DXVK for DX9/10/11
    RAECOMPAT_DX_VKD3D,       // VKD3D-Proton for DX12
    RAECOMPAT_DX_AUTO         // Auto-detect best option
} RaeCompatDXMode;

// Performance optimization flags
typedef struct {
    bool esync_enabled;
    bool fsync_enabled;
    bool large_address_aware;
    bool high_priority;
    bool gpu_scheduling_enabled;
    int cpu_affinity_mask;
    bool memory_optimization;
    bool latency_optimization;
} RaeCompatPerformanceConfig;

// Wine prefix configuration
typedef struct {
    char* name;
    char* path;
    char* windows_version;  // "win10", "win7", etc.
    RaeCompatWineVersion wine_version;
    RaeCompatDXMode dx_mode;
    RaeCompatPerformanceConfig performance;
    
    // DLL overrides
    char** dll_overrides;
    int dll_override_count;
    
    // Registry tweaks
    char** registry_keys;
    char** registry_values;
    int registry_count;
    
    // Winetricks components
    char** winetricks_verbs;
    int winetricks_count;
    
    // Environment variables
    char** env_vars;
    int env_var_count;
} RaeCompatPrefixConfig;

// Application configuration
typedef struct {
    char* name;
    char* executable_path;
    char* working_directory;
    char* arguments;
    RaeCompatAppType type;
    
    // Prefix association
    char* prefix_name;
    
    // Launch options
    bool run_in_terminal;
    bool capture_output;
    bool use_mangohud;
    bool use_gamemode;
    
    // Compatibility flags
    bool requires_anticheat;
    bool requires_admin;
    bool force_windowed;
    bool disable_compositor;
    
    // Performance monitoring
    bool enable_profiling;
    bool log_performance;
} RaeCompatAppConfig;

// System requirements and capabilities
typedef struct {
    bool vulkan_available;
    bool opengl_available;
    bool directx_available;
    char* gpu_vendor;
    char* gpu_model;
    int gpu_memory_mb;
    int system_memory_mb;
    int cpu_cores;
    char* cpu_model;
} RaeCompatSystemInfo;

// ============================================================================
// CORE FRAMEWORK FUNCTIONS
// ============================================================================

// Framework initialization
RaeCompatContext* raecompat_init(void);
void raecompat_shutdown(RaeCompatContext* ctx);

// System detection and setup
RaeCompatSystemInfo* raecompat_detect_system(void);
bool raecompat_setup_environment(RaeCompatContext* ctx);
bool raecompat_install_wine(RaeCompatContext* ctx, RaeCompatWineVersion version);
bool raecompat_install_dxvk(RaeCompatContext* ctx);
bool raecompat_install_vkd3d(RaeCompatContext* ctx);

// ============================================================================
// PREFIX MANAGEMENT
// ============================================================================

// Create and configure Wine prefixes
RaeCompatPrefix* raecompat_create_prefix(RaeCompatContext* ctx, const char* name);
bool raecompat_configure_prefix(RaeCompatPrefix* prefix, RaeCompatPrefixConfig* config);
bool raecompat_delete_prefix(RaeCompatContext* ctx, const char* name);
RaeCompatPrefix** raecompat_list_prefixes(RaeCompatContext* ctx, int* count);

// Winetricks integration
bool raecompat_run_winetricks(RaeCompatPrefix* prefix, const char* verb);
bool raecompat_install_dependencies(RaeCompatPrefix* prefix, const char** deps, int count);
bool raecompat_configure_wine_version(RaeCompatPrefix* prefix, const char* version);

// Registry and DLL management
bool raecompat_set_dll_override(RaeCompatPrefix* prefix, const char* dll, const char* mode);
bool raecompat_set_registry_key(RaeCompatPrefix* prefix, const char* key, const char* value);
bool raecompat_apply_compatibility_fixes(RaeCompatPrefix* prefix, const char* app_name);

// ============================================================================
// APPLICATION MANAGEMENT
// ============================================================================

// Application detection and registration
RaeCompatApplication* raecompat_register_application(RaeCompatContext* ctx, RaeCompatAppConfig* config);
bool raecompat_unregister_application(RaeCompatContext* ctx, const char* name);
RaeCompatApplication** raecompat_list_applications(RaeCompatContext* ctx, int* count);
RaeCompatApplication* raecompat_find_application(RaeCompatContext* ctx, const char* name);

// Auto-detection of installed applications
bool raecompat_scan_for_applications(RaeCompatContext* ctx, const char* directory);
bool raecompat_detect_steam_games(RaeCompatContext* ctx);
bool raecompat_detect_epic_games(RaeCompatContext* ctx);
bool raecompat_detect_gog_games(RaeCompatContext* ctx);

// Application launching
typedef struct {
    pid_t process_id;
    int exit_code;
    bool is_running;
    float cpu_usage;
    float memory_usage_mb;
    float gpu_usage;
    int fps;
} RaeCompatProcessInfo;

RaeCompatProcessInfo* raecompat_launch_application(RaeCompatContext* ctx, const char* app_name);
bool raecompat_terminate_application(RaeCompatContext* ctx, pid_t process_id);
RaeCompatProcessInfo* raecompat_get_process_info(RaeCompatContext* ctx, pid_t process_id);

// ============================================================================
// PERFORMANCE OPTIMIZATION
// ============================================================================

// CPU and memory optimization
bool raecompat_set_cpu_affinity(pid_t process_id, int cpu_mask);
bool raecompat_set_process_priority(pid_t process_id, int priority);
bool raecompat_enable_large_address_aware(RaeCompatPrefix* prefix);
bool raecompat_optimize_memory_usage(RaeCompatPrefix* prefix);

// GPU optimization
bool raecompat_enable_gpu_scheduling(RaeCompatContext* ctx);
bool raecompat_configure_vulkan_layers(RaeCompatContext* ctx);
bool raecompat_setup_dxvk_config(RaeCompatPrefix* prefix);
bool raecompat_setup_vkd3d_config(RaeCompatPrefix* prefix);

// Performance monitoring
typedef struct {
    float fps;
    float frame_time_ms;
    float cpu_usage;
    float gpu_usage;
    float memory_usage_mb;
    float disk_io_mbps;
    float network_io_mbps;
    int active_threads;
} RaeCompatPerformanceStats;

RaeCompatPerformanceStats* raecompat_get_performance_stats(RaeCompatContext* ctx, pid_t process_id);
bool raecompat_enable_mangohud(RaeCompatContext* ctx, bool enable);
bool raecompat_enable_gamemode(RaeCompatContext* ctx, bool enable);

// ============================================================================
// LAUNCHER INTEGRATION
// ============================================================================

// Steam integration
bool raecompat_setup_steam(RaeCompatContext* ctx);
bool raecompat_configure_steam_proton(RaeCompatContext* ctx, const char* proton_version);
bool raecompat_import_steam_library(RaeCompatContext* ctx);

// Epic Games integration
bool raecompat_setup_epic_launcher(RaeCompatContext* ctx);
bool raecompat_configure_epic_wine(RaeCompatContext* ctx);
bool raecompat_import_epic_library(RaeCompatContext* ctx);

// GOG integration
bool raecompat_setup_gog_galaxy(RaeCompatContext* ctx);
bool raecompat_import_gog_library(RaeCompatContext* ctx);

// Generic launcher support
bool raecompat_setup_custom_launcher(RaeCompatContext* ctx, const char* launcher_exe);

// ============================================================================
// ANTI-CHEAT COMPATIBILITY
// ============================================================================

// Anti-cheat system detection
typedef enum {
    RAECOMPAT_ANTICHEAT_NONE,
    RAECOMPAT_ANTICHEAT_EAC,        // Easy Anti-Cheat
    RAECOMPAT_ANTICHEAT_BATTLEYE,   // BattlEye
    RAECOMPAT_ANTICHEAT_VAC,        // Valve Anti-Cheat
    RAECOMPAT_ANTICHEAT_FAIRFIGHT,  // FairFight
    RAECOMPAT_ANTICHEAT_UNKNOWN
} RaeCompatAntiCheatType;

RaeCompatAntiCheatType raecompat_detect_anticheat(const char* executable_path);
bool raecompat_is_anticheat_supported(RaeCompatAntiCheatType type);
bool raecompat_configure_anticheat(RaeCompatPrefix* prefix, RaeCompatAntiCheatType type);

// ============================================================================
// NETWORKING AND ONLINE SUPPORT
// ============================================================================

// Network configuration for online games
bool raecompat_configure_network(RaeCompatPrefix* prefix);
bool raecompat_setup_certificates(RaeCompatPrefix* prefix);
bool raecompat_configure_proxy(RaeCompatPrefix* prefix, const char* proxy_url);

// Online platform authentication
bool raecompat_setup_steam_auth(RaeCompatPrefix* prefix);
bool raecompat_setup_epic_auth(RaeCompatPrefix* prefix);
bool raecompat_setup_xbox_live_auth(RaeCompatPrefix* prefix);

// ============================================================================
// SECURITY AND SANDBOXING
// ============================================================================

// Sandboxing options
typedef struct {
    bool filesystem_isolation;
    bool network_isolation;
    bool device_isolation;
    char** allowed_directories;
    int allowed_dir_count;
    char** blocked_directories;
    int blocked_dir_count;
} RaeCompatSandboxConfig;

bool raecompat_enable_sandbox(RaeCompatPrefix* prefix, RaeCompatSandboxConfig* config);
bool raecompat_disable_sandbox(RaeCompatPrefix* prefix);

// Security validation
bool raecompat_validate_executable(const char* executable_path);
bool raecompat_check_malware(const char* executable_path);

// ============================================================================
// DEBUGGING AND DIAGNOSTICS
// ============================================================================

// Debug output and logging
typedef enum {
    RAECOMPAT_LOG_ERROR,
    RAECOMPAT_LOG_WARNING,
    RAECOMPAT_LOG_INFO,
    RAECOMPAT_LOG_DEBUG,
    RAECOMPAT_LOG_TRACE
} RaeCompatLogLevel;

void raecompat_set_log_level(RaeCompatLogLevel level);
void raecompat_log(RaeCompatLogLevel level, const char* format, ...);

// Wine debug channels
bool raecompat_enable_wine_debug(RaeCompatPrefix* prefix, const char* channels);
bool raecompat_capture_wine_output(RaeCompatPrefix* prefix, const char* output_file);

// System diagnostics
typedef struct {
    bool wine_installed;
    bool dxvk_installed;
    bool vkd3d_installed;
    bool vulkan_working;
    bool opengl_working;
    char* wine_version;
    char* dxvk_version;
    char* vkd3d_version;
    char* graphics_driver_version;
} RaeCompatDiagnostics;

RaeCompatDiagnostics* raecompat_run_diagnostics(RaeCompatContext* ctx);

// ============================================================================
// CONFIGURATION MANAGEMENT
// ============================================================================

// Configuration file management
bool raecompat_save_config(RaeCompatContext* ctx, const char* config_file);
bool raecompat_load_config(RaeCompatContext* ctx, const char* config_file);

// Application-specific configurations
bool raecompat_load_app_config(RaeCompatContext* ctx, const char* app_name);
bool raecompat_save_app_config(RaeCompatContext* ctx, const char* app_name);

// ProtonDB integration
typedef struct {
    char* app_name;
    int steam_app_id;
    char* rating;  // "Platinum", "Gold", "Silver", "Bronze", "Borked"
    char* recommended_proton_version;
    char** required_tweaks;
    int tweak_count;
} RaeCompatProtonDBEntry;

RaeCompatProtonDBEntry* raecompat_query_protondb(const char* app_name);
bool raecompat_apply_protondb_fixes(RaeCompatPrefix* prefix, RaeCompatProtonDBEntry* entry);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Path and file utilities
char* raecompat_get_prefix_path(const char* prefix_name);
char* raecompat_get_wine_executable(RaeCompatWineVersion version);
char* raecompat_resolve_windows_path(RaeCompatPrefix* prefix, const char* windows_path);
char* raecompat_resolve_unix_path(RaeCompatPrefix* prefix, const char* unix_path);

// String utilities
char* raecompat_escape_arguments(const char* args);
char** raecompat_split_string(const char* str, const char* delimiter, int* count);
void raecompat_free_string_array(char** array, int count);

// System utilities
bool raecompat_is_process_running(pid_t pid);
bool raecompat_kill_process_tree(pid_t pid);
uint64_t raecompat_get_available_memory(void);
int raecompat_get_cpu_count(void);

#ifdef __cplusplus
}
#endif