/**
 * ProtonCTL - Command-line tool for managing RaeCompat prefixes and applications
 * Part of the RaeenOS Windows Compatibility Layer
 */

#include "raecompat_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

// ============================================================================
// COMMAND DEFINITIONS
// ============================================================================

typedef enum {
    CMD_HELP,
    CMD_LIST_PREFIXES,
    CMD_CREATE_PREFIX,
    CMD_DELETE_PREFIX,
    CMD_LIST_APPS,
    CMD_INSTALL_APP,
    CMD_LAUNCH_APP,
    CMD_CONFIGURE,
    CMD_DIAGNOSTICS,
    CMD_UPDATE_PROTON,
    CMD_INSTALL_DEPENDENCIES
} ProtonCTLCommand;

typedef struct {
    ProtonCTLCommand command;
    char* prefix_name;
    char* app_name;
    char* app_path;
    char* wine_version;
    bool dxvk_enabled;
    bool vkd3d_enabled;
    bool verbose;
} ProtonCTLOptions;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static void print_usage(const char* program_name) {
    printf("ProtonCTL - RaeenOS Windows Compatibility Manager\n\n");
    printf("Usage: %s [OPTIONS] COMMAND [ARGS]\n\n", program_name);
    printf("Commands:\n");
    printf("  help                     Show this help message\n");
    printf("  list-prefixes           List all Wine prefixes\n");
    printf("  create-prefix NAME      Create a new Wine prefix\n");
    printf("  delete-prefix NAME      Delete a Wine prefix\n");
    printf("  list-apps               List registered applications\n");
    printf("  install-app PATH        Install application from PATH\n");
    printf("  launch-app NAME         Launch registered application\n");
    printf("  configure PREFIX        Configure Wine prefix settings\n");
    printf("  diagnostics             Run system diagnostics\n");
    printf("  update-proton           Update Proton-GE to latest version\n");
    printf("  install-deps PREFIX     Install common dependencies\n\n");
    printf("Options:\n");
    printf("  -p, --prefix NAME       Specify Wine prefix name\n");
    printf("  -w, --wine-version VER  Specify Wine version (stable/staging/proton-ge)\n");
    printf("  -d, --dxvk              Enable DXVK\n");
    printf("  -v, --vkd3d             Enable VKD3D-Proton\n");
    printf("  -V, --verbose           Enable verbose output\n");
    printf("  -h, --help              Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s create-prefix mygame\n", program_name);
    printf("  %s install-app /path/to/game.exe -p mygame -d -v\n", program_name);
    printf("  %s launch-app \"My Game\"\n", program_name);
    printf("  %s configure mygame --wine-version proton-ge\n", program_name);
}

static void print_version(void) {
    printf("ProtonCTL 1.0.0\n");
    printf("Part of RaeenOS Windows Compatibility Layer\n");
}

static ProtonCTLCommand parse_command(const char* cmd_str) {
    if (strcmp(cmd_str, "help") == 0) return CMD_HELP;
    if (strcmp(cmd_str, "list-prefixes") == 0) return CMD_LIST_PREFIXES;
    if (strcmp(cmd_str, "create-prefix") == 0) return CMD_CREATE_PREFIX;
    if (strcmp(cmd_str, "delete-prefix") == 0) return CMD_DELETE_PREFIX;
    if (strcmp(cmd_str, "list-apps") == 0) return CMD_LIST_APPS;
    if (strcmp(cmd_str, "install-app") == 0) return CMD_INSTALL_APP;
    if (strcmp(cmd_str, "launch-app") == 0) return CMD_LAUNCH_APP;
    if (strcmp(cmd_str, "configure") == 0) return CMD_CONFIGURE;
    if (strcmp(cmd_str, "diagnostics") == 0) return CMD_DIAGNOSTICS;
    if (strcmp(cmd_str, "update-proton") == 0) return CMD_UPDATE_PROTON;
    if (strcmp(cmd_str, "install-deps") == 0) return CMD_INSTALL_DEPENDENCIES;
    return CMD_HELP;
}

// ============================================================================
// COMMAND IMPLEMENTATIONS
// ============================================================================

static int cmd_list_prefixes(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    printf("Wine Prefixes:\n");
    printf("==============\n");

    // Get list of prefixes from RaeCompat context
    for (int i = 0; i < ctx->prefix_count; i++) {
        RaeCompatPrefix* prefix = ctx->prefixes[i];
        printf("  %-20s %s\n", prefix->config.name, prefix->config.path);

        if (opts->verbose) {
            printf("    Wine Version: %s\n", prefix->config.wine_version);
            printf("    Windows Ver:  %s\n", prefix->config.windows_version);
            printf("    DXVK:         %s\n", prefix->config.dxvk_enabled ? "Enabled" : "Disabled");
            printf("    VKD3D:        %s\n", prefix->config.vkd3d_enabled ? "Enabled" : "Disabled");
            printf("    Last Used:    %s", ctime(&prefix->last_used));
            printf("\n");
        }
    }

    if (ctx->prefix_count == 0) {
        printf("  No prefixes found. Create one with: protonctl create-prefix <name>\n");
    }

    return 0;
}

static int cmd_create_prefix(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    if (!opts->prefix_name) {
        fprintf(stderr, "Error: Prefix name required\n");
        return 1;
    }

    printf("Creating Wine prefix '%s'...\n", opts->prefix_name);

    RaeCompatPrefix* prefix = raecompat_create_prefix(ctx, opts->prefix_name);
    if (!prefix) {
        fprintf(stderr, "Error: Failed to create prefix '%s'\n", opts->prefix_name);
        return 1;
    }

    // Configure prefix with options
    RaeCompatPrefixConfig config = {0};
    strncpy(config.name, opts->prefix_name, sizeof(config.name) - 1);
    config.wine_version = opts->wine_version ? opts->wine_version : "staging";
    config.dxvk_enabled = opts->dxvk_enabled;
    config.vkd3d_enabled = opts->vkd3d_enabled;
    strncpy(config.windows_version, "win10", sizeof(config.windows_version) - 1);

    if (!raecompat_configure_prefix(prefix, &config)) {
        fprintf(stderr, "Error: Failed to configure prefix '%s'\n", opts->prefix_name);
        return 1;
    }

    printf("✓ Prefix '%s' created successfully\n", opts->prefix_name);
    printf("  Path: %s\n", prefix->config.path);
    printf("  Wine Version: %s\n", prefix->config.wine_version);
    printf("  DXVK: %s\n", prefix->config.dxvk_enabled ? "Enabled" : "Disabled");
    printf("  VKD3D: %s\n", prefix->config.vkd3d_enabled ? "Enabled" : "Disabled");

    return 0;
}

static int cmd_delete_prefix(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    if (!opts->prefix_name) {
        fprintf(stderr, "Error: Prefix name required\n");
        return 1;
    }

    printf("Deleting Wine prefix '%s'...\n", opts->prefix_name);

    // Find prefix
    RaeCompatPrefix* prefix = NULL;
    for (int i = 0; i < ctx->prefix_count; i++) {
        if (strcmp(ctx->prefixes[i]->config.name, opts->prefix_name) == 0) {
            prefix = ctx->prefixes[i];
            break;
        }
    }

    if (!prefix) {
        fprintf(stderr, "Error: Prefix '%s' not found\n", opts->prefix_name);
        return 1;
    }

    // Confirm deletion
    printf("This will permanently delete the prefix and all its data.\n");
    printf("Are you sure? (y/N): ");

    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL) {
        printf("Deletion cancelled.\n");
        return 0;
    }
    // Simple check for 'y' or 'Y' as the first non-whitespace character
    if (strspn(response, " \t\n\r") == strlen(response) || (response[0] != 'y' && response[0] != 'Y')) {
        printf("Deletion cancelled.\n");
        return 0;
    }

    if (raecompat_delete_prefix(ctx, opts->prefix_name)) {
        printf("✓ Prefix '%s' deleted successfully\n", opts->prefix_name);
        return 0;
    } else {
        fprintf(stderr, "Error: Failed to delete prefix '%s'\n", opts->prefix_name);
        return 1;
    }
}

static int cmd_list_apps(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    printf("Registered Applications:\n");
    printf("========================\n");

    for (int i = 0; i < ctx->app_count; i++) {
        RaeCompatApplication* app = ctx->applications[i];
        printf("  %-30s %s\n", app->config.name, app->config.executable_path);

        if (opts->verbose) {
            printf("    Prefix:       %s\n", app->prefix->config.name);
            printf("    Type:         %s\n",
                app->config.app_type == RAECOMPAT_APP_GAME ? "Game" :
                app->config.app_type == RAECOMPAT_APP_LAUNCHER ? "Launcher" :
                app->config.app_type == RAECOMPAT_APP_PRODUCTIVITY ? "Productivity" :
                app->config.app_type == RAECOMPAT_APP_UTILITY ? "Utility" : "Unknown");
            printf("    Launch Count: %d\n", app->launch_count);
            if (app->last_launched > 0) {
                printf("    Last Played:  %s", ctime(&app->last_launched));
            }
            printf("\n");
        }
    }

    if (ctx->app_count == 0) {
        printf("  No applications registered. Install one with: protonctl install-app <path>\n");
    }

    return 0;
}

static int cmd_install_app(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    if (!opts->app_path) {
        fprintf(stderr, "Error: Application path required\n");
        return 1;
    }

    // Check if file exists
    if (access(opts->app_path, F_OK) != 0) {
        fprintf(stderr, "Error: File '%s' not found\n", opts->app_path);
        return 1;
    }

    printf("Installing application from '%s'...\n", opts->app_path);

    // Extract application name from path
    char* app_name = strrchr(opts->app_path, '/');
    if (app_name) {
        app_name++; // Skip the '/'
    } else {
        app_name = opts->app_path;
    }

    // Remove .exe extension if present
    char clean_name[256];
    strncpy(clean_name, app_name, sizeof(clean_name) - 1);
    char* ext = strrchr(clean_name, '.');
    if (ext && strcmp(ext, ".exe") == 0) {
        *ext = '\0';
    }

    // Create application config
    RaeCompatAppConfig app_config = {0};
    strncpy(app_config.name, clean_name, sizeof(app_config.name) - 1);
    strncpy(app_config.executable_path, opts->app_path, sizeof(app_config.executable_path) - 1);
    app_config.app_type = RAECOMPAT_APP_GAME; // Default to game
    app_config.wine_version = opts->wine_version ? opts->wine_version : "staging";
    app_config.dxvk_enabled = opts->dxvk_enabled;
    app_config.vkd3d_enabled = opts->vkd3d_enabled;

    // Use specified prefix or create default
    if (opts->prefix_name) {
        strncpy(app_config.prefix_name, opts->prefix_name, sizeof(app_config.prefix_name) - 1);
    } else {
        strncpy(app_config.prefix_name, clean_name, sizeof(app_config.prefix_name) - 1);
    }

    // Register application
    RaeCompatApplication* app = raecompat_register_application(ctx, &app_config);
    if (!app) {
        fprintf(stderr, "Error: Failed to register application '%s'\n", clean_name);
        return 1;
    }

    printf("✓ Application '%s' installed successfully\n", clean_name);
    printf("  Prefix: %s\n", app->prefix->config.name);
    printf("  Path: %s\n", opts->app_path);
    printf("  DXVK: %s\n", opts->dxvk_enabled ? "Enabled" : "Disabled");
    printf("  VKD3D: %s\n", opts->vkd3d_enabled ? "Enabled" : "Disabled");

    return 0;
}

static int cmd_launch_app(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    if (!opts->app_name) {
        fprintf(stderr, "Error: Application name required\n");
        return 1;
    }

    printf("Launching application '%s'...\n", opts->app_name);

    RaeCompatProcessInfo* process = raecompat_launch_application(ctx, opts->app_name);
    if (!process) {
        fprintf(stderr, "Error: Failed to launch application '%s'\n", opts->app_name);
        return 1;
    }

    printf("✓ Application launched successfully\n");
    printf("  Process ID: %d\n", process->pid);
    printf("  Command: %s\n", process->command_line);

    return 0;
}

static int cmd_diagnostics(RaeCompatContext* ctx, ProtonCTLOptions* opts) {
    printf("Running RaeCompat diagnostics...\n");
    printf("=================================\n");

    RaeCompatDiagnostics* diag = raecompat_run_diagnostics(ctx);
    if (!diag) {
        fprintf(stderr, "Error: Failed to run diagnostics\n");
        return 1;
    }

    printf("System Information:\n");
    printf("  Architecture: %s\n", diag->system_info.architecture);
    printf("  Kernel:       %s\n", diag->system_info.kernel_version);
    printf("  Memory:       %lu MB\n", diag->system_info.total_memory / 1024 / 1024);
    printf("  CPU Cores:    %d\n", diag->system_info.cpu_count);

    printf("\nWine Installation:\n");
    printf("  Wine Found:   %s\n", diag->wine_available ? "Yes" : "No");
    if (diag->wine_available) {
        printf("  Wine Version: %s\n", diag->wine_version);
    }

    printf("\nGraphics Support:\n");
    printf("  Vulkan:       %s\n", diag->vulkan_available ? "Available" : "Not Available");
    printf("  DXVK:         %s\n", diag->dxvk_available ? "Available" : "Not Available");
    printf("  VKD3D:        %s\n", diag->vkd3d_available ? "Available" : "Not Available");

    printf("\nCompatibility Status:\n");
    printf("  Overall:      %s\n",
        diag->overall_status == RAECOMPAT_STATUS_EXCELLENT ? "Excellent" :
        diag->overall_status == RAECOMPAT_STATUS_GOOD ? "Good" :
        diag->overall_status == RAECOMPAT_STATUS_FAIR ? "Fair" : "Poor");

    if (diag->issues_found > 0) {
        printf("\nIssues Found:\n");
        for (int i = 0; i < diag->issues_found; i++) {
            printf("  - %s\n", diag->issues[i]);
        }
    }

    return 0;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int argc, char* argv[]) {
    ProtonCTLOptions opts = {0};
    opts.command = CMD_HELP;

    // Parse command line options
    static struct option long_options[] = {
        {"prefix", required_argument, 0, 'p'},
        {"wine-version", required_argument, 0, 'w'},
        {"dxvk", no_argument, 0, 'd'},
        {"vkd3d", no_argument, 0, 'v'},
        {"verbose", no_argument, 0, 'V'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "p:w:dvVh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'p':
                opts.prefix_name = optarg;
                break;
            case 'w':
                opts.wine_version = optarg;
                break;
            case 'd':
                opts.dxvk_enabled = true;
                break;
            case 'v':
                opts.vkd3d_enabled = true;
                break;
            case 'V':
                opts.verbose = true;
                break;
            case 'h':
                opts.command = CMD_HELP;
                break;
            case 0:
                if (strcmp(long_options[option_index].name, "version") == 0) {
                    print_version();
                    return 0;
                }
                break;
            case '?':
                return 1;
        }
    }

    // Parse command
    if (optind < argc) {
        opts.command = parse_command(argv[optind]);
        optind++;

        // Parse command arguments
        if (optind < argc) {
            switch (opts.command) {
                case CMD_CREATE_PREFIX:
                case CMD_DELETE_PREFIX:
                case CMD_CONFIGURE:
                    opts.prefix_name = argv[optind];
                    break;
                case CMD_INSTALL_APP:
                    opts.app_path = argv[optind];
                    break;
                case CMD_LAUNCH_APP:
                    opts.app_name = argv[optind];
                    break;
                default:
                    break;
            }
        }
    }

    // Handle help command
    if (opts.command == CMD_HELP) {
        print_usage(argv[0]);
        return 0;
    }

    // Initialize RaeCompat context
    RaeCompatContext* ctx = raecompat_init();
    if (!ctx) {
        fprintf(stderr, "Error: Failed to initialize RaeCompat\n");
        return 1;
    }

    // Execute command
    int result = 0;
    switch (opts.command) {
        case CMD_LIST_PREFIXES:
            result = cmd_list_prefixes(ctx, &opts);
            break;
        case CMD_CREATE_PREFIX:
            result = cmd_create_prefix(ctx, &opts);
            break;
        case CMD_DELETE_PREFIX:
            result = cmd_delete_prefix(ctx, &opts);
            break;
        case CMD_LIST_APPS:
            result = cmd_list_apps(ctx, &opts);
            break;
        case CMD_INSTALL_APP:
            result = cmd_install_app(ctx, &opts);
            break;
        case CMD_LAUNCH_APP:
            result = cmd_launch_app(ctx, &opts);
            break;
        case CMD_DIAGNOSTICS:
            result = cmd_diagnostics(ctx, &opts);
            break;
        default:
            fprintf(stderr, "Error: Command not implemented yet\n");
            result = 1;
            break;
    }

    // Cleanup
    raecompat_shutdown(ctx);

    return result;
}
