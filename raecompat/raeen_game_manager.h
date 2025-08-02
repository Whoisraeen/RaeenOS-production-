#pragma once

/**
 * RaeenGameManager - Native GUI Game Launcher for RaeenOS
 * 
 * A beautiful, intuitive game launcher that integrates seamlessly with
 * the RaeCompat compatibility layer to provide the best gaming experience
 * on RaeenOS.
 * 
 * Features:
 * - Steam-like game library interface
 * - Automatic game detection and installation
 * - Per-game configuration and tweaks
 * - Performance monitoring and overlays
 * - Integration with Steam, Epic, GOG libraries
 * - ProtonDB compatibility database
 * - One-click game installation and launch
 */

#include "raecompat_core.h"
#include "../libs/raeenui/raeenui_core.h"
#include "../libs/raeenui/components.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CORE TYPES
// ============================================================================

typedef struct RaeenGameManager RaeenGameManager;
typedef struct GameEntry GameEntry;
typedef struct GameCategory GameCategory;
typedef struct GameLibrary GameLibrary;

// Game entry information
typedef struct GameEntry {
    char* name;
    char* description;
    char* executable_path;
    char* icon_path;
    char* cover_art_path;
    char* developer;
    char* publisher;
    char* release_date;
    char* genre;
    float rating;
    
    // Compatibility info
    RaeCompatAppConfig compat_config;
    char* protondb_rating;
    char* recommended_proton;
    bool is_installed;
    bool is_favorite;
    
    // Statistics
    int play_count;
    float total_playtime_hours;
    time_t last_played;
    time_t install_date;
    
    // Installation info
    uint64_t install_size_bytes;
    float install_progress;
    bool is_installing;
    bool needs_update;
    
    // Performance data
    float avg_fps;
    float avg_cpu_usage;
    float avg_gpu_usage;
    
    struct GameEntry* next;
} GameEntry;

// Game category for organization
typedef struct GameCategory {
    char* name;
    char* icon;
    RaeenUIColor color;
    GameEntry* games;
    int game_count;
    bool is_expanded;
} GameCategory;

// Game library (Steam, Epic, GOG, etc.)
typedef struct GameLibrary {
    char* name;
    char* icon_path;
    char* install_path;
    bool is_connected;
    bool is_scanning;
    GameEntry* games;
    int game_count;
    time_t last_sync;
} GameLibrary;

// Main game manager structure
typedef struct RaeenGameManager {
    RaeenUIContext* ui_context;
    RaeCompatContext* compat_context;
    
    // UI Components
    RaeenUINode* main_window;
    RaeenUINode* sidebar;
    RaeenUINode* game_grid;
    RaeenUINode* game_details;
    RaeenUINode* settings_panel;
    RaeenUINode* performance_overlay;
    
    // Data
    GameLibrary* libraries;
    int library_count;
    GameCategory* categories;
    int category_count;
    GameEntry* all_games;
    int total_game_count;
    
    // Current state
    GameEntry* selected_game;
    char* current_filter;
    char* search_query;
    bool show_only_installed;
    bool show_only_favorites;
    
    // Settings
    bool auto_detect_games;
    bool show_performance_overlay;
    bool enable_notifications;
    bool minimize_to_tray;
    float game_grid_scale;
    
    // Performance monitoring
    bool monitoring_enabled;
    RaeCompatProcessInfo* current_game_process;
} RaeenGameManager;

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

// Initialization and cleanup
RaeenGameManager* raeen_game_manager_create(void);
void raeen_game_manager_destroy(RaeenGameManager* manager);

// Main application lifecycle
bool raeen_game_manager_initialize(RaeenGameManager* manager);
void raeen_game_manager_run(RaeenGameManager* manager);
void raeen_game_manager_update(RaeenGameManager* manager, float delta_time);
void raeen_game_manager_render(RaeenGameManager* manager);

// ============================================================================
// GAME LIBRARY MANAGEMENT
// ============================================================================

// Library detection and setup
bool raeen_game_manager_add_library(RaeenGameManager* manager, const char* name, const char* path);
bool raeen_game_manager_scan_steam_library(RaeenGameManager* manager);
bool raeen_game_manager_scan_epic_library(RaeenGameManager* manager);
bool raeen_game_manager_scan_gog_library(RaeenGameManager* manager);
bool raeen_game_manager_scan_custom_directory(RaeenGameManager* manager, const char* path);

// Game detection and management
GameEntry* raeen_game_manager_add_game(RaeenGameManager* manager, const char* executable_path);
bool raeen_game_manager_remove_game(RaeenGameManager* manager, GameEntry* game);
bool raeen_game_manager_update_game_info(RaeenGameManager* manager, GameEntry* game);

// Game launching and management
bool raeen_game_manager_launch_game(RaeenGameManager* manager, GameEntry* game);
bool raeen_game_manager_install_game(RaeenGameManager* manager, GameEntry* game);
bool raeen_game_manager_uninstall_game(RaeenGameManager* manager, GameEntry* game);
bool raeen_game_manager_update_game(RaeenGameManager* manager, GameEntry* game);

// ============================================================================
// USER INTERFACE
// ============================================================================

// Main UI creation
void raeen_game_manager_create_main_window(RaeenGameManager* manager);
void raeen_game_manager_create_sidebar(RaeenGameManager* manager);
void raeen_game_manager_create_game_grid(RaeenGameManager* manager);
void raeen_game_manager_create_game_details_panel(RaeenGameManager* manager);
void raeen_game_manager_create_settings_panel(RaeenGameManager* manager);

// UI updates
void raeen_game_manager_update_game_grid(RaeenGameManager* manager);
void raeen_game_manager_update_game_details(RaeenGameManager* manager, GameEntry* game);
void raeen_game_manager_update_library_status(RaeenGameManager* manager);

// Event handlers
bool raeen_game_manager_on_game_selected(RaeenUIEvent* event, void* user_data);
bool raeen_game_manager_on_game_launched(RaeenUIEvent* event, void* user_data);
bool raeen_game_manager_on_search_changed(RaeenUIEvent* event, void* user_data);
bool raeen_game_manager_on_filter_changed(RaeenUIEvent* event, void* user_data);
bool raeen_game_manager_on_settings_changed(RaeenUIEvent* event, void* user_data);

// ============================================================================
// GAME CONFIGURATION
// ============================================================================

// Per-game configuration
typedef struct {
    char* wine_version;
    RaeCompatDXMode dx_mode;
    bool enable_dxvk;
    bool enable_vkd3d;
    bool enable_esync;
    bool enable_fsync;
    bool windowed_mode;
    int resolution_width;
    int resolution_height;
    char** launch_arguments;
    int argument_count;
    char** dll_overrides;
    int dll_override_count;
    char** env_variables;
    int env_var_count;
} GameConfiguration;

bool raeen_game_manager_configure_game(RaeenGameManager* manager, GameEntry* game, GameConfiguration* config);
GameConfiguration* raeen_game_manager_get_game_config(RaeenGameManager* manager, GameEntry* game);
bool raeen_game_manager_save_game_config(RaeenGameManager* manager, GameEntry* game);
bool raeen_game_manager_load_game_config(RaeenGameManager* manager, GameEntry* game);

// ProtonDB integration
typedef struct {
    char* game_name;
    char* rating;
    char* recommended_proton;
    char** tweaks;
    int tweak_count;
    char* notes;
} ProtonDBInfo;

ProtonDBInfo* raeen_game_manager_query_protondb(const char* game_name);
bool raeen_game_manager_apply_protondb_fixes(RaeenGameManager* manager, GameEntry* game);

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

// Performance tracking
typedef struct {
    float fps;
    float frame_time_ms;
    float cpu_usage;
    float gpu_usage;
    float memory_usage_mb;
    float disk_usage_mbps;
    float network_usage_mbps;
    int temperature_cpu;
    int temperature_gpu;
} GamePerformanceStats;

GamePerformanceStats* raeen_game_manager_get_performance_stats(RaeenGameManager* manager);
void raeen_game_manager_update_performance_overlay(RaeenGameManager* manager);
bool raeen_game_manager_enable_performance_monitoring(RaeenGameManager* manager, bool enable);

// Performance profiles
typedef struct {
    char* name;
    bool prioritize_fps;
    bool prioritize_quality;
    bool prioritize_power_saving;
    RaeCompatPerformanceConfig compat_settings;
} PerformanceProfile;

bool raeen_game_manager_apply_performance_profile(RaeenGameManager* manager, GameEntry* game, PerformanceProfile* profile);
PerformanceProfile* raeen_game_manager_get_optimal_profile(RaeenGameManager* manager, GameEntry* game);

// ============================================================================
// INSTALLATION AND UPDATES
// ============================================================================

// Game installation
typedef struct {
    GameEntry* game;
    char* installer_path;
    char* install_directory;
    uint64_t total_size;
    uint64_t downloaded_size;
    float progress;
    bool is_active;
    bool is_paused;
    char* current_step;
} GameInstallation;

GameInstallation* raeen_game_manager_start_installation(RaeenGameManager* manager, GameEntry* game, const char* installer_path);
bool raeen_game_manager_pause_installation(RaeenGameManager* manager, GameInstallation* install);
bool raeen_game_manager_resume_installation(RaeenGameManager* manager, GameInstallation* install);
bool raeen_game_manager_cancel_installation(RaeenGameManager* manager, GameInstallation* install);

// Auto-updater
bool raeen_game_manager_check_for_updates(RaeenGameManager* manager);
bool raeen_game_manager_update_proton_database(RaeenGameManager* manager);
bool raeen_game_manager_update_wine_versions(RaeenGameManager* manager);

// ============================================================================
// SOCIAL FEATURES
// ============================================================================

// Game sharing and recommendations
typedef struct {
    GameEntry* game;
    char* screenshot_path;
    char* comment;
    float rating;
    time_t timestamp;
} GameReview;

bool raeen_game_manager_share_game(RaeenGameManager* manager, GameEntry* game, const char* comment);
bool raeen_game_manager_rate_game(RaeenGameManager* manager, GameEntry* game, float rating);
GameReview** raeen_game_manager_get_game_reviews(RaeenGameManager* manager, GameEntry* game, int* count);

// Screenshots and recordings
bool raeen_game_manager_take_screenshot(RaeenGameManager* manager);
bool raeen_game_manager_start_recording(RaeenGameManager* manager);
bool raeen_game_manager_stop_recording(RaeenGameManager* manager);

// ============================================================================
// THEMING AND CUSTOMIZATION
// ============================================================================

// Theme management
typedef struct {
    char* name;
    RaeenUIColor primary_color;
    RaeenUIColor secondary_color;
    RaeenUIColor accent_color;
    RaeenUIColor background_color;
    RaeenUIColor text_color;
    char* background_image;
    float blur_intensity;
    float transparency;
} GameManagerTheme;

bool raeen_game_manager_apply_theme(RaeenGameManager* manager, GameManagerTheme* theme);
GameManagerTheme** raeen_game_manager_get_available_themes(RaeenGameManager* manager, int* count);
bool raeen_game_manager_create_custom_theme(RaeenGameManager* manager, GameManagerTheme* theme);

// Layout customization
typedef enum {
    GAME_VIEW_GRID,
    GAME_VIEW_LIST,
    GAME_VIEW_COVERS,
    GAME_VIEW_COMPACT
} GameViewMode;

bool raeen_game_manager_set_view_mode(RaeenGameManager* manager, GameViewMode mode);
bool raeen_game_manager_set_grid_size(RaeenGameManager* manager, float scale);
bool raeen_game_manager_customize_sidebar(RaeenGameManager* manager, bool show_categories, bool show_stats);

// ============================================================================
// SETTINGS AND CONFIGURATION
// ============================================================================

// Application settings
typedef struct {
    // General
    bool auto_start_with_system;
    bool minimize_to_tray;
    bool close_to_tray;
    bool check_updates_automatically;
    
    // Gaming
    bool auto_detect_games;
    bool show_non_steam_games;
    bool enable_game_overlay;
    bool enable_performance_monitoring;
    
    // Compatibility
    char* default_wine_version;
    RaeCompatDXMode default_dx_mode;
    bool enable_esync_by_default;
    bool enable_fsync_by_default;
    
    // UI
    GameViewMode default_view_mode;
    float game_grid_scale;
    char* theme_name;
    bool enable_animations;
    bool show_background_video;
    
    // Performance
    bool enable_game_mode;
    bool enable_mango_hud;
    bool prioritize_performance;
    int fps_limit;
} GameManagerSettings;

bool raeen_game_manager_load_settings(RaeenGameManager* manager);
bool raeen_game_manager_save_settings(RaeenGameManager* manager);
bool raeen_game_manager_reset_settings(RaeenGameManager* manager);

#ifdef __cplusplus
}
#endif