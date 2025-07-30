#ifndef SYSTEM_SERVICES_INTERFACE_H
#define SYSTEM_SERVICES_INTERFACE_H

/**
 * @file system_services_interface.h
 * @brief Comprehensive System Services Interface for RaeenOS
 * 
 * This interface defines the core system services and daemon APIs
 * that provide essential functionality for the RaeenOS ecosystem.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "process_interface.h"
#include "security_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// System services API version
#define SYSTEM_SERVICES_API_VERSION 1

// Service limits and constants
#define SERVICE_NAME_MAX        64
#define SERVICE_DESC_MAX        256
#define SERVICE_COMMAND_MAX     512
#define SERVICE_ENV_MAX         32
#define SERVICE_DEPENDENCY_MAX  16
#define MAX_SYSTEM_SERVICES     256
#define MAX_SERVICE_INSTANCES   64

// Service states
typedef enum {
    SERVICE_STATE_UNKNOWN,          // Unknown state
    SERVICE_STATE_STOPPED,          // Service is stopped
    SERVICE_STATE_STARTING,         // Service is starting
    SERVICE_STATE_RUNNING,          // Service is running
    SERVICE_STATE_STOPPING,         // Service is stopping
    SERVICE_STATE_FAILED,           // Service failed to start/run
    SERVICE_STATE_RESTARTING,       // Service is restarting
    SERVICE_STATE_DISABLED          // Service is disabled
} service_state_t;

// Service types
typedef enum {
    SERVICE_TYPE_SYSTEM,            // System service (kernel space)
    SERVICE_TYPE_USER,              // User service (user space)
    SERVICE_TYPE_DAEMON,            // Background daemon
    SERVICE_TYPE_ONESHOT,           // One-time execution
    SERVICE_TYPE_FORKING,           // Forking service
    SERVICE_TYPE_NOTIFY,            // Notification service
    SERVICE_TYPE_IDLE,              // Idle service
    SERVICE_TYPE_SOCKET,            // Socket-activated service
    SERVICE_TYPE_TIMER              // Timer-activated service
} service_type_t;

// Service start policies
typedef enum {
    SERVICE_START_AUTO,             // Start automatically at boot
    SERVICE_START_MANUAL,           // Start manually
    SERVICE_START_DEMAND,           // Start on demand
    SERVICE_START_DISABLED          // Never start
} service_start_policy_t;

// Service restart policies
typedef enum {
    SERVICE_RESTART_NO,             // Never restart
    SERVICE_RESTART_ALWAYS,         // Always restart
    SERVICE_RESTART_ON_SUCCESS,     // Restart on successful exit
    SERVICE_RESTART_ON_FAILURE,     // Restart on failure
    SERVICE_RESTART_ON_ABNORMAL     // Restart on abnormal exit
} service_restart_policy_t;

// Service priority levels
typedef enum {
    SERVICE_PRIORITY_CRITICAL,      // Critical system services
    SERVICE_PRIORITY_HIGH,          // High priority services
    SERVICE_PRIORITY_NORMAL,        // Normal priority services
    SERVICE_PRIORITY_LOW,           // Low priority services
    SERVICE_PRIORITY_IDLE           // Idle priority services
} service_priority_t;

// Forward declarations
typedef struct system_service system_service_t;
typedef struct service_manager service_manager_t;
typedef struct service_dependency service_dependency_t;

// Service configuration structure
typedef struct service_config {
    char name[SERVICE_NAME_MAX];        // Service name
    char description[SERVICE_DESC_MAX]; // Service description
    service_type_t type;                // Service type
    service_start_policy_t start_policy; // Start policy
    service_restart_policy_t restart_policy; // Restart policy
    service_priority_t priority;        // Service priority
    
    // Execution configuration
    char executable[SERVICE_COMMAND_MAX]; // Executable path
    char* arguments[16];                // Command line arguments
    char* environment[SERVICE_ENV_MAX]; // Environment variables
    char working_directory[256];        // Working directory
    
    // User and permissions
    uint32_t user_id;                   // Run as user ID
    uint32_t group_id;                  // Run as group ID
    security_context_t* security_context; // Security context
    
    // Resource limits
    struct {
        uint64_t memory_limit;          // Memory limit (bytes)
        uint32_t cpu_limit;             // CPU limit (percentage)
        uint32_t file_limit;            // File descriptor limit
        uint64_t timeout_start;         // Start timeout (ms)
        uint64_t timeout_stop;          // Stop timeout (ms)
        uint64_t timeout_restart;       // Restart timeout (ms)
    } limits;
    
    // Dependencies
    service_dependency_t* dependencies; // Service dependencies
    size_t dependency_count;            // Number of dependencies
    
    // Socket configuration (for socket services)
    struct {
        char socket_path[256];          // Socket path
        uint16_t port;                  // Network port
        bool tcp;                       // TCP socket
        bool udp;                       // UDP socket
        bool unix_socket;               // Unix domain socket
    } socket;
    
    // Timer configuration (for timer services)
    struct {
        uint64_t interval_ms;           // Timer interval
        uint64_t delay_ms;              // Initial delay
        bool repeat;                    // Repeat timer
    } timer;
    
    uint32_t flags;                     // Service flags
} service_config_t;

// Service dependency structure
struct service_dependency {
    char name[SERVICE_NAME_MAX];        // Dependency service name
    enum {
        DEPENDENCY_REQUIRES,            // Hard dependency
        DEPENDENCY_WANTS,               // Soft dependency
        DEPENDENCY_CONFLICTS,           // Conflicting service
        DEPENDENCY_BEFORE,              // Must start before
        DEPENDENCY_AFTER                // Must start after
    } type;
    bool optional;                      // Optional dependency
};

// Service statistics
typedef struct service_stats {
    uint64_t start_time;                // Service start time
    uint64_t uptime;                    // Service uptime
    uint32_t restart_count;             // Number of restarts
    uint32_t failure_count;             // Number of failures
    
    // Resource usage
    uint64_t memory_usage;              // Current memory usage
    float cpu_usage;                    // Current CPU usage
    uint32_t file_descriptors;          // Open file descriptors
    uint64_t network_bytes_in;          // Network bytes received
    uint64_t network_bytes_out;         // Network bytes sent
    
    // Performance metrics
    uint64_t requests_handled;          // Requests handled
    uint64_t average_response_time;     // Average response time
    uint32_t error_count;               // Error count
    uint32_t warning_count;             // Warning count
} service_stats_t;

// System service structure
struct system_service {
    // Service identification
    char name[SERVICE_NAME_MAX];        // Service name
    char description[SERVICE_DESC_MAX]; // Service description
    uint32_t service_id;                // Unique service ID
    
    // Configuration
    service_config_t* config;           // Service configuration
    
    // Current state
    service_state_t state;              // Current state
    service_state_t target_state;       // Target state
    int exit_code;                      // Last exit code
    char status_message[256];           // Status message
    
    // Process information
    process_t* main_process;            // Main service process
    uint32_t pid;                       // Process ID
    uint32_t ppid;                      // Parent process ID
    
    // Dependencies
    system_service_t** dependencies;   // Dependency services
    size_t dependency_count;            // Number of dependencies
    system_service_t** dependents;     // Dependent services
    size_t dependent_count;             // Number of dependents
    
    // Statistics
    service_stats_t stats;              // Service statistics
    
    // Control operations
    struct service_operations* ops;     // Service operations
    
    // Synchronization
    void* lock;                         // Service lock
    
    // Private data
    void* private_data;                 // Service-specific data
};

// Service operations structure
typedef struct service_operations {
    // Lifecycle operations
    int (*start)(system_service_t* service);
    int (*stop)(system_service_t* service);
    int (*restart)(system_service_t* service);
    int (*reload)(system_service_t* service);
    int (*status)(system_service_t* service, char* buffer, size_t size);
    
    // Configuration operations
    int (*configure)(system_service_t* service, service_config_t* config);
    int (*validate_config)(service_config_t* config);
    
    // Health monitoring
    int (*health_check)(system_service_t* service);
    int (*get_metrics)(system_service_t* service, service_stats_t* stats);
    
    // Event handling
    int (*on_process_exit)(system_service_t* service, int exit_code);
    int (*on_signal)(system_service_t* service, int signal);
    int (*on_timeout)(system_service_t* service, int timeout_type);
    
    // Custom operations
    int (*custom_command)(system_service_t* service, const char* command, 
                         const char* args, char* response, size_t response_size);
} service_ops_t;

// Service manager structure
struct service_manager {
    // Service registry
    system_service_t* services[MAX_SYSTEM_SERVICES]; // Registered services
    size_t service_count;               // Number of services
    
    // State management
    enum {
        SERVICE_MGR_INITIALIZING,       // Manager initializing
        SERVICE_MGR_RUNNING,            // Manager running
        SERVICE_MGR_SHUTTING_DOWN       // Manager shutting down
    } state;
    
    // Configuration
    struct {
        uint32_t max_parallel_starts;   // Max parallel service starts
        uint64_t default_timeout;       // Default operation timeout
        bool auto_restart_failed;       // Auto restart failed services
        uint32_t restart_delay_ms;      // Delay between restarts
        uint32_t max_restart_attempts;  // Max restart attempts
    } config;
    
    // Statistics
    struct {
        uint32_t services_running;      // Currently running services
        uint32_t services_failed;       // Failed services
        uint32_t total_starts;          // Total service starts
        uint32_t total_stops;           // Total service stops
        uint32_t total_restarts;        // Total service restarts
        uint64_t uptime;                // Manager uptime
    } stats;
    
    // Event handling
    void (*on_service_state_change)(system_service_t* service, 
                                   service_state_t old_state, 
                                   service_state_t new_state);
    
    // Synchronization
    void* lock;                         // Manager lock
    
    void* private_data;
};

// System services operations
typedef struct system_services_operations {
    // Initialization
    int (*init)(void);
    void (*cleanup)(void);
    
    // Service management
    int (*register_service)(service_config_t* config, system_service_t** service);
    int (*unregister_service)(const char* name);
    system_service_t* (*find_service)(const char* name);
    int (*list_services)(system_service_t*** services, size_t* count);
    
    // Service control
    int (*start_service)(const char* name);
    int (*stop_service)(const char* name);
    int (*restart_service)(const char* name);
    int (*reload_service)(const char* name);
    int (*enable_service)(const char* name);
    int (*disable_service)(const char* name);
    
    // Service status
    service_state_t (*get_service_state)(const char* name);
    int (*get_service_status)(const char* name, char* buffer, size_t size);
    int (*get_service_stats)(const char* name, service_stats_t* stats);
    
    // Bulk operations
    int (*start_all_services)(void);
    int (*stop_all_services)(void);
    int (*restart_failed_services)(void);
    
    // Configuration management
    int (*load_service_config)(const char* config_file, service_config_t** config);
    int (*save_service_config)(const char* config_file, service_config_t* config);
    int (*reload_service_configs)(void);
    
    // Dependency management
    int (*add_dependency)(const char* service, const char* dependency, int type);
    int (*remove_dependency)(const char* service, const char* dependency);
    int (*resolve_dependencies)(const char* service, char*** ordered_list, size_t* count);
    
    // Health monitoring
    int (*health_check_all)(void);
    int (*get_system_health)(float* health_score);
    int (*register_health_monitor)(void (*callback)(system_service_t*, int));
    
    // Event system
    int (*register_event_handler)(const char* event_type, 
                                 void (*handler)(const char*, void*));
    int (*unregister_event_handler)(const char* event_type);
    int (*emit_event)(const char* event_type, void* data);
    
    // System management
    int (*system_shutdown)(int timeout_seconds);
    int (*system_reboot)(int timeout_seconds);
    int (*system_suspend)(void);
    int (*system_hibernate)(void);
    
    // Performance monitoring
    int (*get_performance_metrics)(void* metrics);
    int (*start_performance_monitoring)(const char* service_name);
    int (*stop_performance_monitoring)(const char* service_name);
    
    // Logging and debugging
    int (*set_log_level)(const char* service_name, int log_level);
    int (*get_service_logs)(const char* service_name, char** logs, size_t* size);
    int (*enable_debug_mode)(const char* service_name);
    int (*disable_debug_mode)(const char* service_name);
} system_services_ops_t;

// Global system services operations
extern system_services_ops_t* system_services;

// System Services API Functions

// Initialization
int system_services_init(void);
void system_services_cleanup(void);

// Service management
int service_register(service_config_t* config);
int service_unregister(const char* name);
int service_start(const char* name);
int service_stop(const char* name);
int service_restart(const char* name);
int service_reload(const char* name);

// Service status queries
service_state_t service_get_state(const char* name);
bool service_is_running(const char* name);
bool service_is_enabled(const char* name);
int service_get_info(const char* name, service_stats_t* stats);

// Configuration helpers
service_config_t* service_config_create(const char* name, const char* executable);
void service_config_destroy(service_config_t* config);
int service_config_set_user(service_config_t* config, const char* username);
int service_config_add_dependency(service_config_t* config, const char* dep_name, int type);
int service_config_set_environment(service_config_t* config, const char* key, const char* value);

// System control
int system_shutdown_services(void);
int system_restart_services(void);
int system_emergency_stop(void);

// Built-in system services
int service_init_system(void);          // System initialization service
int service_init_network(void);         // Network service
int service_init_audio(void);           // Audio service  
int service_init_graphics(void);        // Graphics service
int service_init_ai(void);              // AI service
int service_init_security(void);        // Security service
int service_init_package_manager(void); // Package manager service
int service_init_update_manager(void);  // Update manager service
int service_init_power_manager(void);   // Power management service
int service_init_thermal_manager(void); // Thermal management service

// Service helper macros
#define SERVICE_DEFINE(name, desc, exec) \
    static service_config_t name##_config = { \
        .name = #name, \
        .description = desc, \
        .executable = exec, \
        .type = SERVICE_TYPE_DAEMON, \
        .start_policy = SERVICE_START_AUTO, \
        .restart_policy = SERVICE_RESTART_ON_FAILURE, \
        .priority = SERVICE_PRIORITY_NORMAL \
    }

#define SERVICE_REGISTER(name) \
    service_register(&name##_config)

#define SERVICE_START_AUTO(service) \
    do { \
        (service)->config->start_policy = SERVICE_START_AUTO; \
    } while(0)

#define SERVICE_SET_PRIORITY(service, prio) \
    do { \
        (service)->config->priority = prio; \
    } while(0)

// Service state check macros
#define SERVICE_IS_RUNNING(state) \
    ((state) == SERVICE_STATE_RUNNING)

#define SERVICE_IS_STOPPED(state) \
    ((state) == SERVICE_STATE_STOPPED || (state) == SERVICE_STATE_FAILED)

#define SERVICE_IS_TRANSITIONING(state) \
    ((state) == SERVICE_STATE_STARTING || (state) == SERVICE_STATE_STOPPING || \
     (state) == SERVICE_STATE_RESTARTING)

// Service flags
#define SERVICE_FLAG_AUTO_START         (1 << 0)  // Auto start at boot
#define SERVICE_FLAG_RESTART_ON_FAILURE (1 << 1)  // Restart on failure
#define SERVICE_FLAG_CRITICAL           (1 << 2)  // Critical service
#define SERVICE_FLAG_NO_KILL            (1 << 3)  // Don't kill on shutdown
#define SERVICE_FLAG_SOCKET_ACTIVATED   (1 << 4)  // Socket activated
#define SERVICE_FLAG_TIMER_ACTIVATED    (1 << 5)  // Timer activated
#define SERVICE_FLAG_ONE_SHOT           (1 << 6)  // One-time execution
#define SERVICE_FLAG_PRIVILEGED         (1 << 7)  // Requires privileges

// Error codes
#define SERVICE_SUCCESS                 0
#define SERVICE_ERR_NOT_FOUND          -4001
#define SERVICE_ERR_ALREADY_EXISTS     -4002
#define SERVICE_ERR_INVALID_CONFIG     -4003
#define SERVICE_ERR_DEPENDENCY_FAILED  -4004
#define SERVICE_ERR_START_FAILED       -4005
#define SERVICE_ERR_STOP_FAILED        -4006
#define SERVICE_ERR_TIMEOUT            -4007
#define SERVICE_ERR_PERMISSION_DENIED  -4008
#define SERVICE_ERR_RESOURCE_LIMIT     -4009
#define SERVICE_ERR_INVALID_STATE      -4010

// Default service configuration
#define DEFAULT_SERVICE_TIMEOUT_START   30000    // 30 seconds
#define DEFAULT_SERVICE_TIMEOUT_STOP    10000    // 10 seconds  
#define DEFAULT_SERVICE_TIMEOUT_RESTART 5000     // 5 seconds
#define DEFAULT_MAX_RESTART_ATTEMPTS    3
#define DEFAULT_RESTART_DELAY_MS        1000     // 1 second

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_SERVICES_INTERFACE_H