/**
 * RaeenOS Out-of-Box Experience (OOBE)
 * First-time setup wizard for new installations
 */

#ifndef OOBE_H
#define OOBE_H

#include <stdint.h>
#include <stdbool.h>
#include "../ui/raeenui.h"

// OOBE stages
typedef enum {
    OOBE_STAGE_WELCOME = 0,
    OOBE_STAGE_LANGUAGE,
    OOBE_STAGE_LICENSE,
    OOBE_STAGE_NETWORK,
    OOBE_STAGE_GPU_DRIVERS,
    OOBE_STAGE_ACCOUNT,
    OOBE_STAGE_PRIVACY,
    OOBE_STAGE_GAMING,
    OOBE_STAGE_MIGRATION,
    OOBE_STAGE_FINALIZE,
    OOBE_STAGE_COMPLETE
} oobe_stage_t;

// Language selection
typedef struct {
    const char* code;        // ISO 639-1 code (e.g., "en", "es", "fr")
    const char* name;        // Native name (e.g., "English", "Español")
    const char* region;      // Region code (e.g., "US", "ES", "FR")
    bool rtl;               // Right-to-left text
} oobe_language_t;

// Network configuration
typedef struct {
    char ssid[64];
    char password[128];
    bool use_ethernet;
    bool skip_network;
    bool proxy_enabled;
    char proxy_host[256];
    uint16_t proxy_port;
} oobe_network_config_t;

// User account information
typedef struct {
    char username[64];
    char full_name[128];
    char password[128];
    char password_confirm[128];
    char security_question[256];
    char security_answer[128];
    bool use_biometric;
    bool use_pin;
    char pin[16];
    uint8_t* profile_picture;
    uint32_t picture_size;
} oobe_account_t;

// Privacy settings
typedef struct {
    bool telemetry_enabled;
    bool crash_reports;
    bool usage_analytics;
    bool location_services;
    bool advertising_id;
    bool diagnostic_data;
    bool cloud_sync;
} oobe_privacy_t;

// Gaming configuration
typedef struct {
    bool install_steam;
    bool install_epic;
    bool install_battlenet;
    bool install_gog;
    bool install_proton;
    bool install_dxvk;
    bool install_winetricks;
    bool enable_game_mode;
    bool install_discord;
    char steam_library_path[512];
} oobe_gaming_t;

// Data migration settings
typedef struct {
    bool migrate_documents;
    bool migrate_pictures;
    bool migrate_music;
    bool migrate_videos;
    bool migrate_downloads;
    bool migrate_desktop;
    bool migrate_bookmarks;
    bool migrate_steam_games;
    char windows_drive[8];
    char migration_path[512];
} oobe_migration_t;

// OOBE context
typedef struct {
    oobe_stage_t current_stage;
    bool completed;
    bool skipped;
    
    // UI components
    RaeenUIWindow* window;
    RaeenUIView* main_container;
    RaeenUIView* header;
    RaeenUIView* content;
    RaeenUIView* footer;
    RaeenUIView* progress_bar;
    
    // Configuration data
    oobe_language_t selected_language;
    oobe_network_config_t network_config;
    oobe_account_t user_account;
    oobe_privacy_t privacy_settings;
    oobe_gaming_t gaming_config;
    oobe_migration_t migration_settings;
    
    // State
    bool license_accepted;
    bool network_configured;
    bool account_created;
    uint32_t start_time;
    uint32_t stage_times[OOBE_STAGE_COMPLETE + 1];
} oobe_context_t;

// Function prototypes

// Core OOBE functions
bool oobe_init(void);
void oobe_shutdown(void);
bool oobe_is_required(void);
bool oobe_is_completed(void);
void oobe_run(void);

// Stage management
void oobe_set_stage(oobe_stage_t stage);
oobe_stage_t oobe_get_stage(void);
void oobe_next_stage(void);
void oobe_previous_stage(void);
bool oobe_can_skip_stage(oobe_stage_t stage);

// UI creation functions
void oobe_create_ui(void);
void oobe_create_welcome_stage(void);
void oobe_create_language_stage(void);
void oobe_create_license_stage(void);
void oobe_create_network_stage(void);
void oobe_create_gpu_stage(void);
void oobe_create_account_stage(void);
void oobe_create_privacy_stage(void);
void oobe_create_gaming_stage(void);
void oobe_create_migration_stage(void);
void oobe_create_finalize_stage(void);

// Event handlers
void oobe_handle_next_button(RaeenUIView* view, void* user_data);
void oobe_handle_back_button(RaeenUIView* view, void* user_data);
void oobe_handle_skip_button(RaeenUIView* view, void* user_data);
void oobe_handle_language_select(RaeenUIView* view, void* user_data);
void oobe_handle_license_accept(RaeenUIView* view, void* user_data);
void oobe_handle_network_connect(RaeenUIView* view, void* user_data);
void oobe_handle_account_create(RaeenUIView* view, void* user_data);

// Configuration functions
bool oobe_detect_windows_partition(void);
bool oobe_scan_wifi_networks(void);
bool oobe_test_network_connection(void);
bool oobe_download_gpu_drivers(void);
bool oobe_create_user_account(const oobe_account_t* account);
bool oobe_apply_privacy_settings(const oobe_privacy_t* privacy);
bool oobe_install_gaming_software(const oobe_gaming_t* gaming);
bool oobe_migrate_user_data(const oobe_migration_t* migration);

// Validation functions
bool oobe_validate_username(const char* username);
bool oobe_validate_password(const char* password);
bool oobe_validate_email(const char* email);
bool oobe_validate_network_config(const oobe_network_config_t* config);

// Progress tracking
void oobe_update_progress(void);
uint32_t oobe_get_progress_percent(void);
const char* oobe_get_stage_title(oobe_stage_t stage);
const char* oobe_get_stage_description(oobe_stage_t stage);

// Localization
void oobe_set_language(const oobe_language_t* language);
const char* oobe_get_text(const char* key);
void oobe_load_language_pack(const char* language_code);

// System integration
void oobe_save_configuration(void);
void oobe_load_configuration(void);
void oobe_mark_completed(void);
void oobe_create_user_profile(void);
void oobe_setup_desktop_environment(void);

// Utility functions
void oobe_show_error(const char* message);
void oobe_show_warning(const char* message);
void oobe_show_info(const char* message);
bool oobe_confirm_action(const char* message);

// Available languages
extern const oobe_language_t OOBE_LANGUAGES[];
extern const uint32_t OOBE_LANGUAGE_COUNT;

// Stage titles and descriptions
extern const char* OOBE_STAGE_TITLES[];
extern const char* OOBE_STAGE_DESCRIPTIONS[];

// Default configurations
extern const oobe_privacy_t OOBE_PRIVACY_DEFAULTS;
extern const oobe_gaming_t OOBE_GAMING_DEFAULTS;
extern const oobe_migration_t OOBE_MIGRATION_DEFAULTS;

// Constants
#define OOBE_MAX_USERNAME_LENGTH    32
#define OOBE_MAX_PASSWORD_LENGTH    128
#define OOBE_MAX_SSID_LENGTH        64
#define OOBE_MIN_PASSWORD_LENGTH    8
#define OOBE_CONFIG_FILE           "/etc/raeenos/oobe.conf"
#define OOBE_COMPLETION_FLAG       "/var/lib/raeenos/oobe_completed"

// UI dimensions
#define OOBE_WINDOW_WIDTH          1000
#define OOBE_WINDOW_HEIGHT         700
#define OOBE_CONTENT_PADDING       40
#define OOBE_BUTTON_HEIGHT         40
#define OOBE_INPUT_HEIGHT          35

// Colors
#define OOBE_COLOR_PRIMARY         0xFF6B46C1
#define OOBE_COLOR_SECONDARY       0xFF8B5CF6
#define OOBE_COLOR_BACKGROUND      0xFFF8FAFC
#define OOBE_COLOR_SURFACE         0xFFFFFFFF
#define OOBE_COLOR_TEXT            0xFF1E293B
#define OOBE_COLOR_TEXT_SECONDARY  0xFF64748B
#define OOBE_COLOR_SUCCESS         0xFF10B981
#define OOBE_COLOR_WARNING         0xFFF59E0B
#define OOBE_COLOR_ERROR           0xFFEF4444

#endif // OOBE_H
