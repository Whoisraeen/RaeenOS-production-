/**
 * RaeenOS Package Manager CLI (raepkg)
 * Command-line interface for the RaePkg package manager
 */

#include "raepkg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

// CLI command structure
typedef struct {
    const char* name;
    const char* description;
    int (*handler)(int argc, char* argv[]);
    bool requires_root;
} CLICommand;

// Global variables
static PackageManager* g_pm = NULL;
static bool g_verbose = false;
static bool g_quiet = false;
static bool g_assume_yes = false;

// Function declarations
static int cmd_install(int argc, char* argv[]);
static int cmd_remove(int argc, char* argv[]);
static int cmd_update(int argc, char* argv[]);
static int cmd_upgrade(int argc, char* argv[]);
static int cmd_search(int argc, char* argv[]);
static int cmd_info(int argc, char* argv[]);
static int cmd_list(int argc, char* argv[]);
static int cmd_clean(int argc, char* argv[]);
static int cmd_repo(int argc, char* argv[]);
static int cmd_verify(int argc, char* argv[]);
static int cmd_stats(int argc, char* argv[]);
static int cmd_help(int argc, char* argv[]);

static void print_usage(void);
static void print_version(void);
static void setup_signal_handlers(void);
static void signal_handler(int sig);
static void download_progress_callback(const char* package_name, uint64_t downloaded, uint64_t total, void* user_data);
static void install_progress_callback(const char* package_name, const char* current_file, uint32_t files_processed, uint32_t total_files, void* user_data);
static bool confirm_action(const char* message);
static void print_package_info(PackageMetadata* package);
static void print_package_list(PackageMetadata** packages, uint32_t count);

// Command table
static CLICommand commands[] = {
    {"install", "Install packages", cmd_install, true},
    {"remove", "Remove packages", cmd_remove, true},
    {"update", "Update package database", cmd_update, false},
    {"upgrade", "Upgrade installed packages", cmd_upgrade, true},
    {"search", "Search for packages", cmd_search, false},
    {"info", "Show package information", cmd_info, false},
    {"list", "List packages", cmd_list, false},
    {"clean", "Clean package cache", cmd_clean, false},
    {"repo", "Manage repositories", cmd_repo, true},
    {"verify", "Verify package integrity", cmd_verify, false},
    {"stats", "Show package statistics", cmd_stats, false},
    {"help", "Show help information", cmd_help, false},
    {NULL, NULL, NULL, false}
};

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    int opt;
    const char* config_dir = NULL;
    
    // Parse global options
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"quiet", no_argument, 0, 'q'},
        {"yes", no_argument, 0, 'y'},
        {"config", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "vqyc:hV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                g_verbose = true;
                break;
            case 'q':
                g_quiet = true;
                break;
            case 'y':
                g_assume_yes = true;
                break;
            case 'c':
                config_dir = optarg;
                break;
            case 'h':
                print_usage();
                return 0;
            case 'V':
                print_version();
                return 0;
            default:
                print_usage();
                return 1;
        }
    }
    
    // Check if command was provided
    if (optind >= argc) {
        if (!g_quiet) {
            printf("Error: No command specified\n\n");
            print_usage();
        }
        return 1;
    }
    
    const char* command = argv[optind];
    
    // Find and execute command
    CLICommand* cmd = NULL;
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(command, commands[i].name) == 0) {
            cmd = &commands[i];
            break;
        }
    }
    
    if (!cmd) {
        if (!g_quiet) {
            printf("Error: Unknown command '%s'\n\n", command);
            print_usage();
        }
        return 1;
    }
    
    // Check root privileges if required
    if (cmd->requires_root && getuid() != 0) {
        printf("Error: Command '%s' requires root privileges\n", command);
        return 1;
    }
    
    // Initialize package manager
    g_pm = raepkg_init(config_dir);
    if (!g_pm) {
        printf("Error: Failed to initialize package manager\n");
        return 1;
    }
    
    // Set up progress callbacks
    raepkg_set_download_callback(g_pm, download_progress_callback, NULL);
    raepkg_set_install_callback(g_pm, install_progress_callback, NULL);
    
    // Set up signal handlers
    setup_signal_handlers();
    
    // Execute command
    int result = cmd->handler(argc - optind, argv + optind);
    
    // Cleanup
    raepkg_shutdown(g_pm);
    
    return result;
}

/**
 * Install command
 */
static int cmd_install(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: raepkg install <package1> [package2] ...\n");
        return 1;
    }
    
    bool all_success = true;
    
    for (int i = 1; i < argc; i++) {
        const char* package_name = argv[i];
        
        if (g_verbose) {
            printf("Installing package: %s\n", package_name);
        }
        
        if (!raepkg_install_package(g_pm, package_name)) {
            printf("Failed to install package: %s\n", package_name);
            all_success = false;
        }
    }
    
    return all_success ? 0 : 1;
}

/**
 * Remove command
 */
static int cmd_remove(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: raepkg remove <package1> [package2] ...\n");
        return 1;
    }
    
    // Confirm removal unless -y flag is used
    if (!g_assume_yes) {
        printf("The following packages will be removed:\n");
        for (int i = 1; i < argc; i++) {
            printf("  %s\n", argv[i]);
        }
        
        if (!confirm_action("Do you want to continue?")) {
            printf("Operation cancelled.\n");
            return 0;
        }
    }
    
    bool all_success = true;
    
    for (int i = 1; i < argc; i++) {
        const char* package_name = argv[i];
        
        if (g_verbose) {
            printf("Removing package: %s\n", package_name);
        }
        
        if (!raepkg_remove_package(g_pm, package_name)) {
            printf("Failed to remove package: %s\n", package_name);
            all_success = false;
        }
    }
    
    return all_success ? 0 : 1;
}

/**
 * Update command
 */
static int cmd_update(int argc, char* argv[]) {
    printf("Updating package database...\n");
    
    if (!raepkg_sync_repositories(g_pm)) {
        printf("Failed to update package database\n");
        return 1;
    }
    
    printf("Package database updated successfully\n");
    return 0;
}

/**
 * Upgrade command
 */
static int cmd_upgrade(int argc, char* argv[]) {
    printf("Checking for package updates...\n");
    
    uint32_t update_count;
    UpdateInfo** updates = raepkg_check_updates(g_pm, &update_count);
    
    if (update_count == 0) {
        printf("All packages are up to date\n");
        return 0;
    }
    
    printf("The following packages will be upgraded:\n");
    for (uint32_t i = 0; i < update_count; i++) {
        printf("  %s (%s -> %s)\n", 
               updates[i]->current_package->name,
               raepkg_version_to_string(&updates[i]->current_package->version),
               raepkg_version_to_string(&updates[i]->new_package->version));
    }
    
    if (!g_assume_yes && !confirm_action("Do you want to continue?")) {
        printf("Operation cancelled.\n");
        return 0;
    }
    
    if (!raepkg_update_system(g_pm)) {
        printf("Failed to upgrade packages\n");
        return 1;
    }
    
    printf("System upgraded successfully\n");
    return 0;
}

/**
 * Search command
 */
static int cmd_search(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: raepkg search <pattern>\n");
        return 1;
    }
    
    const char* pattern = argv[1];
    
    SearchFilter filter = {0};
    strncpy(filter.name_pattern, pattern, sizeof(filter.name_pattern) - 1);
    strncpy(filter.description_pattern, pattern, sizeof(filter.description_pattern) - 1);
    
    uint32_t result_count;
    PackageMetadata** results = raepkg_search_packages(g_pm, &filter, &result_count);
    
    if (result_count == 0) {
        printf("No packages found matching '%s'\n", pattern);
        return 0;
    }
    
    printf("Found %d package(s) matching '%s':\n\n", result_count, pattern);
    print_package_list(results, result_count);
    
    free(results);
    return 0;
}

/**
 * Info command
 */
static int cmd_info(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: raepkg info <package>\n");
        return 1;
    }
    
    const char* package_name = argv[1];
    PackageMetadata* package = raepkg_find_package(g_pm, package_name);
    
    if (!package) {
        printf("Package '%s' not found\n", package_name);
        return 1;
    }
    
    print_package_info(package);
    free(package);
    return 0;
}

/**
 * List command
 */
static int cmd_list(int argc, char* argv[]) {
    bool installed_only = false;
    
    // Parse options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--installed") == 0) {
            installed_only = true;
        }
    }
    
    uint32_t count;
    PackageMetadata** packages;
    
    if (installed_only) {
        packages = raepkg_list_installed_packages(g_pm, &count);
        printf("Installed packages (%d):\n\n", count);
    } else {
        packages = raepkg_list_available_packages(g_pm, &count);
        printf("Available packages (%d):\n\n", count);
    }
    
    print_package_list(packages, count);
    free(packages);
    return 0;
}

/**
 * Clean command
 */
static int cmd_clean(int argc, char* argv[]) {
    printf("Cleaning package cache...\n");
    
    if (!raepkg_clean_cache(g_pm)) {
        printf("Failed to clean cache\n");
        return 1;
    }
    
    printf("Cache cleaned successfully\n");
    return 0;
}

/**
 * Repository command
 */
static int cmd_repo(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: raepkg repo <add|remove|list|enable|disable> [options]\n");
        return 1;
    }
    
    const char* action = argv[1];
    
    if (strcmp(action, "add") == 0) {
        if (argc < 4) {
            printf("Usage: raepkg repo add <name> <url> [--trusted]\n");
            return 1;
        }
        
        const char* name = argv[2];
        const char* url = argv[3];
        bool trusted = (argc > 4 && strcmp(argv[4], "--trusted") == 0);
        
        if (raepkg_add_repository(g_pm, name, url, trusted)) {
            printf("Repository '%s' added successfully\n", name);
            return 0;
        } else {
            printf("Failed to add repository '%s'\n", name);
            return 1;
        }
    }
    else if (strcmp(action, "remove") == 0) {
        if (argc < 3) {
            printf("Usage: raepkg repo remove <name>\n");
            return 1;
        }
        
        const char* name = argv[2];
        
        if (raepkg_remove_repository(g_pm, name)) {
            printf("Repository '%s' removed successfully\n", name);
            return 0;
        } else {
            printf("Failed to remove repository '%s'\n", name);
            return 1;
        }
    }
    else if (strcmp(action, "list") == 0) {
        printf("Configured repositories:\n");
        // Implementation would list all repositories
        return 0;
    }
    else {
        printf("Unknown repository action: %s\n", action);
        return 1;
    }
}

/**
 * Statistics command
 */
static int cmd_stats(int argc, char* argv[]) {
    raepkg_print_statistics(g_pm);
    return 0;
}

/**
 * Help command
 */
static int cmd_help(int argc, char* argv[]) {
    print_usage();
    return 0;
}

// Helper function implementations

/**
 * Print usage information
 */
static void print_usage(void) {
    printf("RaePkg - RaeenOS Package Manager\n\n");
    printf("Usage: raepkg [options] <command> [arguments]\n\n");
    printf("Global Options:\n");
    printf("  -v, --verbose     Enable verbose output\n");
    printf("  -q, --quiet       Suppress output\n");
    printf("  -y, --yes         Assume yes for all prompts\n");
    printf("  -c, --config DIR  Use alternative config directory\n");
    printf("  -h, --help        Show this help\n");
    printf("  -V, --version     Show version information\n\n");
    printf("Commands:\n");
    
    for (int i = 0; commands[i].name; i++) {
        printf("  %-12s %s\n", commands[i].name, commands[i].description);
    }
    
    printf("\nExamples:\n");
    printf("  raepkg update                    Update package database\n");
    printf("  raepkg install firefox           Install Firefox browser\n");
    printf("  raepkg search text editor        Search for text editors\n");
    printf("  raepkg info vim                  Show information about vim\n");
    printf("  raepkg remove --purge old-pkg    Remove package and config\n");
    printf("  raepkg upgrade                   Upgrade all packages\n");
}

/**
 * Print version information
 */
static void print_version(void) {
    printf("RaePkg 1.0.0 - RaeenOS Package Manager\n");
    printf("Built for RaeenOS production release\n");
    printf("Copyright (C) 2024 RaeenOS Project\n");
}

/**
 * Setup signal handlers
 */
static void setup_signal_handlers(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

/**
 * Signal handler
 */
static void signal_handler(int sig) {
    printf("\nOperation interrupted by signal %d\n", sig);
    if (g_pm) {
        raepkg_shutdown(g_pm);
    }
    exit(1);
}

/**
 * Download progress callback
 */
static void download_progress_callback(const char* package_name, uint64_t downloaded, uint64_t total, void* user_data) {
    if (g_quiet) return;
    
    double percent = (double)downloaded / total * 100.0;
    printf("\rDownloading %s: %.1f%% (%lu/%lu bytes)", package_name, percent, downloaded, total);
    fflush(stdout);
    
    if (downloaded == total) {
        printf("\n");
    }
}

/**
 * Install progress callback
 */
static void install_progress_callback(const char* package_name, const char* current_file, uint32_t files_processed, uint32_t total_files, void* user_data) {
    if (g_quiet) return;
    
    double percent = (double)files_processed / total_files * 100.0;
    printf("\rInstalling %s: %.1f%% (%u/%u files)", package_name, percent, files_processed, total_files);
    fflush(stdout);
    
    if (files_processed == total_files) {
        printf("\n");
    }
}

/**
 * Confirm action with user
 */
static bool confirm_action(const char* message) {
    if (g_assume_yes) return true;
    
    printf("%s [y/N]: ", message);
    fflush(stdout);
    
    char response[16];
    if (fgets(response, sizeof(response), stdin)) {
        return (response[0] == 'y' || response[0] == 'Y');
    }
    
    return false;
}

/**
 * Print package information
 */
static void print_package_info(PackageMetadata* package) {
    printf("Package: %s\n", package->name);
    printf("Version: %s\n", raepkg_version_to_string(&package->version));
    printf("Description: %s\n", package->description);
    printf("Category: %s\n", raepkg_category_to_string(package->category));
    printf("Architecture: %s\n", package->architecture == PKG_ARCH_X86_64 ? "x86_64" : "unknown");
    printf("Status: %s\n", raepkg_status_to_string(package->status));
    printf("Download Size: %lu bytes\n", package->download_size);
    printf("Installed Size: %lu bytes\n", package->installed_size);
    printf("Repository: %s\n", package->repository_name);
    
    if (package->status == PKG_STATUS_INSTALLED) {
        printf("Install Date: %s", ctime(&package->install_time));
        printf("Install Path: %s\n", package->install_path);
    }
    
    if (package->dependency_count > 0) {
        printf("Dependencies:\n");
        for (uint32_t i = 0; i < package->dependency_count; i++) {
            printf("  %s\n", package->dependencies[i].name);
        }
    }
}

/**
 * Print package list
 */
static void print_package_list(PackageMetadata** packages, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        PackageMetadata* pkg = packages[i];
        printf("%-30s %-15s %s\n", 
               pkg->name, 
               raepkg_version_to_string(&pkg->version),
               pkg->summary[0] ? pkg->summary : pkg->description);
    }
}
