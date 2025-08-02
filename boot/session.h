/**
 * RaeenOS Session Management System
 * Handles desktop shell launch, fast resume, and user session lifecycle
 */

#ifndef SESSION_H
#define SESSION_H

#include <stdint.h>
#include <stdbool.h>
#include "../ui/raeenui.h"

// Session states
typedef enum {
    SESSION_STATE_NONE = 0,
    SESSION_STATE_INITIALIZING,
    SESSION_STATE_LOGIN_SCREEN,
    SESSION_STATE_AUTHENTICATING,
    SESSION_STATE_LOADING,
    SESSION_STATE_ACTIVE,
    SESSION_STATE_LOCKED,
    SESSION_STATE_SUSPENDING,
    SESSION_STATE_SUSPENDED,
    SESSION_STATE_RESUMING,
    SESSION_STATE_TERMINATING
} session_state_t;

// Authentication methods
typedef enum {
    AUTH_METHOD_PASSWORD = 0,
    AUTH_METHOD_PIN,
    AUTH_METHOD_BIOMETRIC,
    AUTH_METHOD_SMART_CARD,
    AUTH_METHOD_AUTO_LOGIN
} auth_method_t;

// User session information
typedef struct {
    uint32_t user_id;
    char username[64];
    char full_name[128];
    char home_directory[512];
    char profile_picture[512];
    uint32_t login_time;
    uint32_t last_activity;
    bool is_admin;
    bool auto_login_enabled;
    auth_method_t preferred_auth;
} user_session_t;

// Desktop environment configuration
typedef struct {
    char theme_name[64];
    char wallpaper_path[512];
    uint32_t accent_color;
    bool dark_mode;
    bool animations_enabled;
    bool transparency_enabled;
    uint32_t animation_speed;
    bool game_mode_enabled;
    bool ai_assistant_enabled;
} desktop_config_t;

// Session manager context
typedef struct {
    session_state_t state;
    user_session_t current_user;
    desktop_config_t desktop_config;
    
    // UI components
    RaeenUIWindow* login_window;
    RaeenUIWindow* lock_screen;
    RaeenUIView* desktop_shell;
    
    // Authentication
    char auth_input[256];
    uint32_t auth_attempts;
    uint32_t auth_lockout_time;
    bool biometric_available;
    
    // Fast resume
    bool fast_resume_enabled;
    void* gpu_context_backup;
    uint32_t context_backup_size;
    uint32_t suspend_time;
    
    // Background services
    bool services_started;
    uint32_t service_count;
    char** running_services;
    
    // Performance monitoring
    uint32_t boot_time_ms;
    uint32_t login_time_ms;
    uint32_t desktop_load_time_ms;
    uint32_t resume_time_ms;
} session_manager_t;

// Background service definition
typedef struct {
    const char* name;
    const char* description;
    bool (*init_func)(void);
    void (*shutdown_func)(void);
    bool (*health_check)(void);
    bool essential;
    uint32_t startup_delay_ms;
} background_service_t;

// Function prototypes

// Core session management
bool session_manager_init(void);
void session_manager_shutdown(void);
session_state_t session_get_state(void);
void session_set_state(session_state_t state);

// Boot sequence integration
void session_handle_boot_complete(void);
void session_show_login_screen(void);
void session_start_desktop_shell(void);
void session_complete_startup(void);

// Authentication
bool session_authenticate_user(const char* username, const char* credential, auth_method_t method);
bool session_validate_password(const char* username, const char* password);
bool session_validate_pin(const char* username, const char* pin);
bool session_validate_biometric(const char* username, const void* biometric_data);
void session_handle_auth_failure(void);
void session_reset_auth_attempts(void);

// User session lifecycle
bool session_create_user_session(const char* username);
void session_destroy_user_session(void);
bool session_switch_user(const char* username);
void session_lock_session(void);
bool session_unlock_session(const char* credential, auth_method_t method);

// Desktop shell management
bool session_load_desktop_shell(void);
void session_unload_desktop_shell(void);
bool session_restart_desktop_shell(void);
void session_apply_desktop_config(const desktop_config_t* config);

// Fast resume functionality
bool session_enable_fast_resume(void);
void session_disable_fast_resume(void);
bool session_suspend_session(void);
bool session_resume_session(void);
void session_backup_gpu_context(void);
void session_restore_gpu_context(void);

// Background services
bool session_start_background_services(void);
void session_stop_background_services(void);
bool session_start_service(const char* service_name);
bool session_stop_service(const char* service_name);
bool session_restart_service(const char* service_name);
bool session_is_service_running(const char* service_name);

// UI creation
void session_create_login_ui(void);
void session_create_lock_screen_ui(void);
void session_update_login_progress(uint32_t percent, const char* message);
void session_show_auth_error(const char* message);

// Event handlers
void session_handle_login_submit(RaeenUIView* view, void* user_data);
void session_handle_auth_method_change(RaeenUIView* view, void* user_data);
void session_handle_unlock_submit(RaeenUIView* view, void* user_data);
void session_handle_user_activity(void);
void session_handle_idle_timeout(void);

// Configuration management
bool session_load_user_config(const char* username);
bool session_save_user_config(const char* username);
bool session_load_desktop_config(desktop_config_t* config);
bool session_save_desktop_config(const desktop_config_t* config);

// Performance monitoring
void session_record_boot_time(uint32_t time_ms);
void session_record_login_time(uint32_t time_ms);
void session_record_desktop_load_time(uint32_t time_ms);
void session_record_resume_time(uint32_t time_ms);
void session_get_performance_stats(uint32_t* boot_time, uint32_t* login_time, 
                                  uint32_t* desktop_time, uint32_t* resume_time);

// Security functions
bool session_check_user_permissions(const char* username, const char* permission);
void session_log_security_event(const char* event, const char* details);
bool session_is_session_secure(void);
void session_enforce_security_policy(void);

// Utility functions
const char* session_state_to_string(session_state_t state);
const char* session_auth_method_to_string(auth_method_t method);
bool session_is_user_logged_in(void);
uint32_t session_get_idle_time(void);
void session_update_last_activity(void);

// Available background services
extern const background_service_t SESSION_BACKGROUND_SERVICES[];
extern const uint32_t SESSION_SERVICE_COUNT;

// Default configurations
extern const desktop_config_t SESSION_DESKTOP_CONFIG_DEFAULT;
extern const desktop_config_t SESSION_DESKTOP_CONFIG_GAMING;
extern const desktop_config_t SESSION_DESKTOP_CONFIG_MINIMAL;

// Constants
#define SESSION_MAX_AUTH_ATTEMPTS       3
#define SESSION_AUTH_LOCKOUT_TIME_MS    300000  // 5 minutes
#define SESSION_IDLE_TIMEOUT_MS         1800000 // 30 minutes
#define SESSION_AUTO_LOCK_TIMEOUT_MS    600000  // 10 minutes
#define SESSION_FAST_RESUME_TIMEOUT_MS  5000    // 5 seconds max resume time

// File paths
#define SESSION_CONFIG_DIR              "/etc/raeenos/session"
#define SESSION_USER_CONFIG_DIR         "/home/%s/.config/raeenos"
#define SESSION_DESKTOP_CONFIG_FILE     "desktop.conf"
#define SESSION_AUTH_LOG_FILE           "/var/log/raeenos/auth.log"
#define SESSION_PERFORMANCE_LOG_FILE    "/var/log/raeenos/performance.log"

// UI dimensions
#define SESSION_LOGIN_WINDOW_WIDTH      400
#define SESSION_LOGIN_WINDOW_HEIGHT     500
#define SESSION_LOCK_SCREEN_FULLSCREEN  true

// Colors
#define SESSION_COLOR_LOGIN_BG          0xFF1E293B
#define SESSION_COLOR_LOGIN_SURFACE     0xFF334155
#define SESSION_COLOR_LOGIN_PRIMARY     0xFF6B46C1
#define SESSION_COLOR_LOGIN_TEXT        0xFFFFFFFF
#define SESSION_COLOR_LOGIN_ERROR       0xFFEF4444
#define SESSION_COLOR_LOGIN_SUCCESS     0xFF10B981

#endif // SESSION_H
