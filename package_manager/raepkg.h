/**
 * RaeenOS Package Manager (RaePkg)
 * Modern, secure package management with atomic updates and dependency resolution
 */

#ifndef RAEPKG_H
#define RAEPKG_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

// Package format types
typedef enum {
    PKG_FORMAT_RAEPKG,      // Native RaeenOS package format
    PKG_FORMAT_FLATPAK,     // Flatpak compatibility
    PKG_FORMAT_APPIMAGE,    // AppImage compatibility
    PKG_FORMAT_SNAP,        // Snap compatibility
    PKG_FORMAT_DEB,         // Debian package compatibility
    PKG_FORMAT_RPM,         // RPM package compatibility
    PKG_FORMAT_TAR_XZ       // Simple tarball format
} PackageFormat;

// Package categories
typedef enum {
    PKG_CATEGORY_SYSTEM,
    PKG_CATEGORY_DEVELOPMENT,
    PKG_CATEGORY_GAMES,
    PKG_CATEGORY_MULTIMEDIA,
    PKG_CATEGORY_PRODUCTIVITY,
    PKG_CATEGORY_INTERNET,
    PKG_CATEGORY_GRAPHICS,
    PKG_CATEGORY_EDUCATION,
    PKG_CATEGORY_UTILITIES,
    PKG_CATEGORY_SECURITY
} PackageCategory;

// Package architecture
typedef enum {
    PKG_ARCH_X86_64,
    PKG_ARCH_ARM64,
    PKG_ARCH_X86,
    PKG_ARCH_UNIVERSAL
} PackageArchitecture;

// Installation status
typedef enum {
    PKG_STATUS_NOT_INSTALLED,
    PKG_STATUS_INSTALLED,
    PKG_STATUS_PENDING_INSTALL,
    PKG_STATUS_PENDING_UPDATE,
    PKG_STATUS_PENDING_REMOVAL,
    PKG_STATUS_BROKEN,
    PKG_STATUS_HELD
} PackageStatus;

// Security verification levels
typedef enum {
    PKG_SECURITY_NONE,
    PKG_SECURITY_CHECKSUM,
    PKG_SECURITY_SIGNED,
    PKG_SECURITY_VERIFIED_PUBLISHER
} PackageSecurityLevel;

// Dependency types
typedef enum {
    DEP_TYPE_REQUIRED,
    DEP_TYPE_OPTIONAL,
    DEP_TYPE_CONFLICTS,
    DEP_TYPE_PROVIDES,
    DEP_TYPE_REPLACES
} DependencyType;

// Version comparison operators
typedef enum {
    VERSION_OP_EQ,  // ==
    VERSION_OP_NE,  // !=
    VERSION_OP_LT,  // <
    VERSION_OP_LE,  // <=
    VERSION_OP_GT,  // >
    VERSION_OP_GE   // >=
} VersionOperator;

// Package version
typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t build;
    char pre_release[32];   // e.g., "alpha", "beta", "rc1"
    char build_metadata[64]; // e.g., git commit hash
} PackageVersion;

// Package dependency
typedef struct {
    char name[256];
    PackageVersion min_version;
    PackageVersion max_version;
    VersionOperator version_op;
    DependencyType type;
    bool is_optional;
} PackageDependency;

// Package metadata
typedef struct {
    char name[256];
    char display_name[256];
    char description[1024];
    char summary[256];
    char homepage[512];
    char license[128];
    char maintainer[256];
    char maintainer_email[256];
    PackageVersion version;
    PackageCategory category;
    PackageArchitecture architecture;
    PackageFormat format;
    
    // Dependencies
    PackageDependency* dependencies;
    uint32_t dependency_count;
    
    // File information
    uint64_t installed_size;
    uint64_t download_size;
    char* file_list;
    uint32_t file_count;
    
    // Security
    PackageSecurityLevel security_level;
    char signature[512];
    char checksum_sha256[65];
    char publisher_id[128];
    
    // Installation metadata
    PackageStatus status;
    time_t install_time;
    time_t last_update;
    char install_path[512];
    
    // Repository information
    char repository_url[512];
    char repository_name[128];
    uint32_t repository_priority;
} PackageMetadata;

// Repository configuration
typedef struct {
    char name[128];
    char url[512];
    char description[256];
    bool enabled;
    bool trusted;
    uint32_t priority;
    char gpg_key_id[64];
    char mirror_urls[10][512];
    uint32_t mirror_count;
    time_t last_sync;
    uint64_t total_packages;
} Repository;

// Package database entry
typedef struct {
    PackageMetadata metadata;
    char local_path[512];
    bool is_cached;
    time_t cache_time;
    struct PackageDBEntry* next;
} PackageDBEntry;

// Transaction operation
typedef struct {
    enum {
        TRANSACTION_INSTALL,
        TRANSACTION_UPDATE,
        TRANSACTION_REMOVE,
        TRANSACTION_DOWNGRADE
    } operation;
    PackageMetadata* package;
    char reason[256];
} TransactionOperation;

// Package transaction
typedef struct {
    uint64_t transaction_id;
    TransactionOperation* operations;
    uint32_t operation_count;
    uint32_t operation_capacity;
    
    // Transaction state
    bool is_prepared;
    bool is_committed;
    bool can_rollback;
    time_t start_time;
    time_t commit_time;
    
    // Rollback information
    char snapshot_id[64];
    char* rollback_script;
    
    // Progress tracking
    uint32_t current_operation;
    uint64_t bytes_downloaded;
    uint64_t total_bytes;
    double progress_percent;
} PackageTransaction;

// Download progress callback
typedef void (*DownloadProgressCallback)(const char* package_name, uint64_t downloaded, uint64_t total, void* user_data);

// Installation progress callback
typedef void (*InstallProgressCallback)(const char* package_name, const char* current_file, uint32_t files_processed, uint32_t total_files, void* user_data);

// Package manager context
typedef struct {
    // Configuration
    char config_dir[512];
    char cache_dir[512];
    char install_root[512];
    char database_path[512];
    
    // Repositories
    Repository* repositories;
    uint32_t repository_count;
    uint32_t repository_capacity;
    
    // Package database
    PackageDBEntry* package_db;
    uint32_t package_count;
    pthread_mutex_t db_mutex;
    
    // Active transactions
    PackageTransaction* transactions;
    uint32_t transaction_count;
    uint32_t transaction_capacity;
    uint64_t next_transaction_id;
    pthread_mutex_t transaction_mutex;
    
    // Configuration options
    bool auto_resolve_dependencies;
    bool allow_downgrades;
    bool verify_signatures;
    bool use_delta_updates;
    uint32_t max_parallel_downloads;
    uint32_t cache_retention_days;
    uint64_t max_cache_size;
    
    // Callbacks
    DownloadProgressCallback download_callback;
    InstallProgressCallback install_callback;
    void* callback_user_data;
    
    // Statistics
    uint64_t packages_installed;
    uint64_t packages_updated;
    uint64_t packages_removed;
    uint64_t total_downloads;
    uint64_t total_download_size;
    time_t last_update_check;
    
    // Synchronization
    pthread_mutex_t manager_mutex;
    bool is_initialized;
} PackageManager;

// Search filters
typedef struct {
    char name_pattern[256];
    char description_pattern[256];
    PackageCategory category;
    PackageArchitecture architecture;
    PackageStatus status;
    bool installed_only;
    bool available_only;
} SearchFilter;

// Update information
typedef struct {
    PackageMetadata* current_package;
    PackageMetadata* new_package;
    bool security_update;
    bool breaking_changes;
    char changelog[2048];
} UpdateInfo;

// Function declarations

// Core package manager functions
PackageManager* raepkg_init(const char* config_dir);
void raepkg_shutdown(PackageManager* pm);
bool raepkg_load_config(PackageManager* pm, const char* config_file);
bool raepkg_save_config(PackageManager* pm, const char* config_file);

// Repository management
bool raepkg_add_repository(PackageManager* pm, const char* name, const char* url, bool trusted);
bool raepkg_remove_repository(PackageManager* pm, const char* name);
bool raepkg_enable_repository(PackageManager* pm, const char* name, bool enabled);
bool raepkg_sync_repositories(PackageManager* pm);
bool raepkg_sync_repository(PackageManager* pm, const char* name);

// Package database operations
bool raepkg_update_database(PackageManager* pm);
PackageMetadata* raepkg_find_package(PackageManager* pm, const char* name);
PackageMetadata** raepkg_search_packages(PackageManager* pm, const SearchFilter* filter, uint32_t* result_count);
PackageMetadata** raepkg_list_installed_packages(PackageManager* pm, uint32_t* count);
PackageMetadata** raepkg_list_available_packages(PackageManager* pm, uint32_t* count);

// Dependency resolution
bool raepkg_resolve_dependencies(PackageManager* pm, PackageMetadata* package, PackageMetadata*** dependencies, uint32_t* dep_count);
bool raepkg_check_conflicts(PackageManager* pm, PackageMetadata* package);
bool raepkg_verify_dependencies(PackageManager* pm, PackageMetadata* package);

// Transaction management
PackageTransaction* raepkg_create_transaction(PackageManager* pm);
bool raepkg_transaction_add_install(PackageTransaction* transaction, const char* package_name);
bool raepkg_transaction_add_update(PackageTransaction* transaction, const char* package_name);
bool raepkg_transaction_add_remove(PackageTransaction* transaction, const char* package_name);
bool raepkg_transaction_prepare(PackageManager* pm, PackageTransaction* transaction);
bool raepkg_transaction_commit(PackageManager* pm, PackageTransaction* transaction);
bool raepkg_transaction_rollback(PackageManager* pm, PackageTransaction* transaction);
void raepkg_transaction_destroy(PackageTransaction* transaction);

// Package operations
bool raepkg_install_package(PackageManager* pm, const char* package_name);
bool raepkg_update_package(PackageManager* pm, const char* package_name);
bool raepkg_remove_package(PackageManager* pm, const char* package_name);
bool raepkg_download_package(PackageManager* pm, const char* package_name, const char* destination);

// System operations
bool raepkg_update_system(PackageManager* pm);
UpdateInfo** raepkg_check_updates(PackageManager* pm, uint32_t* update_count);
bool raepkg_clean_cache(PackageManager* pm);
bool raepkg_autoremove_orphans(PackageManager* pm);

// Package verification
bool raepkg_verify_package(PackageManager* pm, PackageMetadata* package);
bool raepkg_verify_signature(PackageManager* pm, PackageMetadata* package);
bool raepkg_verify_checksum(PackageManager* pm, const char* file_path, const char* expected_checksum);

// Package information
bool raepkg_get_package_info(PackageManager* pm, const char* package_name, PackageMetadata** info);
char** raepkg_get_package_files(PackageManager* pm, const char* package_name, uint32_t* file_count);
PackageDependency** raepkg_get_package_dependencies(PackageManager* pm, const char* package_name, uint32_t* dep_count);
PackageMetadata** raepkg_get_reverse_dependencies(PackageManager* pm, const char* package_name, uint32_t* dep_count);

// Compatibility layer functions
bool raepkg_install_flatpak(PackageManager* pm, const char* flatpak_id);
bool raepkg_install_appimage(PackageManager* pm, const char* appimage_url);
bool raepkg_install_deb(PackageManager* pm, const char* deb_file);
bool raepkg_install_rpm(PackageManager* pm, const char* rpm_file);

// Utility functions
int raepkg_compare_versions(const PackageVersion* v1, const PackageVersion* v2);
bool raepkg_version_satisfies(const PackageVersion* version, const PackageVersion* requirement, VersionOperator op);
char* raepkg_version_to_string(const PackageVersion* version);
bool raepkg_parse_version(const char* version_str, PackageVersion* version);
const char* raepkg_status_to_string(PackageStatus status);
const char* raepkg_category_to_string(PackageCategory category);

// Progress and callback functions
void raepkg_set_download_callback(PackageManager* pm, DownloadProgressCallback callback, void* user_data);
void raepkg_set_install_callback(PackageManager* pm, InstallProgressCallback callback, void* user_data);

// Statistics and monitoring
void raepkg_get_statistics(PackageManager* pm, uint64_t* installed, uint64_t* available, uint64_t* updates);
void raepkg_print_statistics(PackageManager* pm);
bool raepkg_export_package_list(PackageManager* pm, const char* file_path);
bool raepkg_import_package_list(PackageManager* pm, const char* file_path);

#endif // RAEPKG_H
