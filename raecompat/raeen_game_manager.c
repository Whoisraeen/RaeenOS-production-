/**
 * RaeenGameManager Implementation
 * Native GUI Game Launcher for RaeenOS
 */

#include "raeen_game_manager.h"
#include "raecompat_core.h"
#include "../libs/raeenui/raeenui_core.h"
#include "../libs/raeenui/components.h"
#include "../kernel/memory.h"
#include "../kernel/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <json-c/json.h>

// ============================================================================
// INTERNAL STRUCTURES
// ============================================================================

struct RaeenGameManager {
    RaeCompatContext* compat_context;
    RaeenUIWindow* main_window;
    RaeenUIContainer* game_grid;
    RaeenUIContainer* sidebar;
    RaeenUIContainer* status_bar;
    
    // Game libraries
    GameLibrary** libraries;
    int library_count;
    
    // UI Components
    RaeenUIButton* install_button;
    RaeenUIButton* launch_button;
    RaeenUIButton* settings_button;
    RaeenUITextInput* search_box;
    RaeenUIProgressBar* download_progress;
    
    // State
    GameEntry* selected_game;
    GameInstallation** active_installations;
    int installation_count;
    
    // Configuration
    char* config_file;
    char* games_directory;
    char* downloads_directory;
    
    // ProtonDB integration
    json_object* protondb_data;
    time_t protondb_last_update;
};

struct GameEntry {
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
    RaeCompatAppType app_type;
    char* wine_version;
    char* proton_version;
    bool dxvk_enabled;
    bool vkd3d_enabled;
    char** launch_args;
    int launch_arg_count;
    
    // ProtonDB data
    char* protondb_tier;
    char* protondb_confidence;
    
    // Installation info
    bool is_installed;
    uint64_t install_size;
    char* install_path;
    time_t install_date;
    
    // Usage statistics
    int launch_count;
    time_t last_played;
    uint64_t total_playtime;
};

struct GameLibrary {
    char* name;
    char* path;
    GameLibraryType type;
    GameEntry** games;
    int game_count;
    bool auto_scan;
};

struct GameInstallation {
    GameEntry* game;
    char* source_path;
    char* destination_path;
    uint64_t total_size;
    uint64_t downloaded_size;
    float progress;
    GameInstallationStatus status;
    time_t start_time;
    pid_t installer_pid;
};

// ============================================================================
// CONSTANTS
// ============================================================================

#define RAEEN_GAME_MANAGER_CONFIG_FILE "/home/.raecompat/game_manager.json"
#define RAEEN_GAME_MANAGER_GAMES_DIR "/home/.raecompat/games"
#define RAEEN_GAME_MANAGER_DOWNLOADS_DIR "/home/.raecompat/downloads"
#define PROTONDB_CACHE_FILE "/home/.raecompat/protondb_cache.json"
#define PROTONDB_UPDATE_INTERVAL (24 * 60 * 60) // 24 hours

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static char* raeen_game_manager_join_path(const char* base, const char* relative) {
    size_t base_len = strlen(base);
    size_t rel_len = strlen(relative);
    char* result = malloc(base_len + rel_len + 2);
    
    strcpy(result, base);
    if (base[base_len - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, relative);
    
    return result;
}

static bool raeen_game_manager_create_directory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0755) == 0;
    }
    return true;
}

static void raeen_game_manager_scan_directory(GameLibrary* library, const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".exe")) {
            // Found an executable, create a game entry
            GameEntry* game = malloc(sizeof(GameEntry));
            memset(game, 0, sizeof(GameEntry));
            
            game->name = strdup(entry->d_name);
            game->executable_path = raeen_game_manager_join_path(path, entry->d_name);
            game->is_installed = true;
            game->app_type = RAECOMPAT_APP_GAME;
            
            // Add to library
            library->games = realloc(library->games, 
                sizeof(GameEntry*) * (library->game_count + 1));
            library->games[library->game_count++] = game;
        }
    }
    
    closedir(dir);
}

// ============================================================================
// CORE IMPLEMENTATION
// ============================================================================

RaeenGameManager* raeen_game_manager_create(void) {
    RaeenGameManager* manager = malloc(sizeof(RaeenGameManager));
    memset(manager, 0, sizeof(RaeenGameManager));
    
    // Initialize RaeCompat context
    manager->compat_context = raecompat_init();
    if (!manager->compat_context) {
        free(manager);
        return NULL;
    }
    
    // Set up directories
    manager->config_file = strdup(RAEEN_GAME_MANAGER_CONFIG_FILE);
    manager->games_directory = strdup(RAEEN_GAME_MANAGER_GAMES_DIR);
    manager->downloads_directory = strdup(RAEEN_GAME_MANAGER_DOWNLOADS_DIR);
    
    raeen_game_manager_create_directory(manager->games_directory);
    raeen_game_manager_create_directory(manager->downloads_directory);
    
    // Initialize UI
    manager->main_window = raeenui_create_window("RaeenOS Game Manager", 1200, 800);
    if (!manager->main_window) {
        raeen_game_manager_destroy(manager);
        return NULL;
    }
    
    // Create UI layout
    raeen_game_manager_create_ui(manager);
    
    // Load configuration
    raeen_game_manager_load_config(manager);
    
    // Scan for games
    raeen_game_manager_scan_libraries(manager);
    
    return manager;
}

void raeen_game_manager_destroy(RaeenGameManager* manager) {
    if (!manager) return;
    
    // Save configuration
    raeen_game_manager_save_config(manager);
    
    // Cleanup libraries
    for (int i = 0; i < manager->library_count; i++) {
        GameLibrary* lib = manager->libraries[i];
        for (int j = 0; j < lib->game_count; j++) {
            GameEntry* game = lib->games[j];
            free(game->name);
            free(game->description);
            free(game->executable_path);
            free(game->icon_path);
            free(game->cover_art_path);
            free(game->developer);
            free(game->publisher);
            free(game->release_date);
            free(game->genre);
            free(game->wine_version);
            free(game->proton_version);
            free(game->protondb_tier);
            free(game->protondb_confidence);
            free(game->install_path);
            
            if (game->launch_args) {
                for (int k = 0; k < game->launch_arg_count; k++) {
                    free(game->launch_args[k]);
                }
                free(game->launch_args);
            }
            
            free(game);
        }
        free(lib->games);
        free(lib->name);
        free(lib->path);
        free(lib);
    }
    free(manager->libraries);
    
    // Cleanup installations
    for (int i = 0; i < manager->installation_count; i++) {
        GameInstallation* install = manager->active_installations[i];
        free(install->source_path);
        free(install->destination_path);
        free(install);
    }
    free(manager->active_installations);
    
    // Cleanup UI
    if (manager->main_window) {
        raeenui_destroy_window(manager->main_window);
    }
    
    // Cleanup RaeCompat
    if (manager->compat_context) {
        raecompat_shutdown(manager->compat_context);
    }
    
    // Cleanup ProtonDB data
    if (manager->protondb_data) {
        json_object_put(manager->protondb_data);
    }
    
    free(manager->config_file);
    free(manager->games_directory);
    free(manager->downloads_directory);
    free(manager);
}

bool raeen_game_manager_create_ui(RaeenGameManager* manager) {
    if (!manager || !manager->main_window) return false;
    
    // Create main layout (horizontal split)
    RaeenUIContainer* main_container = raeenui_create_container(RAEENUI_CONTAINER_HORIZONTAL);
    raeenui_window_set_content(manager->main_window, main_container);
    
    // Create sidebar
    manager->sidebar = raeenui_create_container(RAEENUI_CONTAINER_VERTICAL);
    raeenui_container_set_width(manager->sidebar, 250);
    raeenui_container_add_child(main_container, manager->sidebar);
    
    // Add sidebar buttons
    RaeenUIButton* library_btn = raeenui_create_button("Library");
    RaeenUIButton* store_btn = raeenui_create_button("Store");
    RaeenUIButton* downloads_btn = raeenui_create_button("Downloads");
    manager->settings_button = raeenui_create_button("Settings");
    
    raeenui_container_add_child(manager->sidebar, library_btn);
    raeenui_container_add_child(manager->sidebar, store_btn);
    raeenui_container_add_child(manager->sidebar, downloads_btn);
    raeenui_container_add_child(manager->sidebar, manager->settings_button);
    
    // Create main content area
    RaeenUIContainer* content_area = raeenui_create_container(RAEENUI_CONTAINER_VERTICAL);
    raeenui_container_add_child(main_container, content_area);
    
    // Create toolbar
    RaeenUIContainer* toolbar = raeenui_create_container(RAEENUI_CONTAINER_HORIZONTAL);
    raeenui_container_set_height(toolbar, 50);
    raeenui_container_add_child(content_area, toolbar);
    
    // Add search box
    manager->search_box = raeenui_create_text_input("Search games...");
    raeenui_container_add_child(toolbar, manager->search_box);
    
    // Add action buttons
    manager->install_button = raeenui_create_button("Install");
    manager->launch_button = raeenui_create_button("Launch");
    
    raeenui_container_add_child(toolbar, manager->install_button);
    raeenui_container_add_child(toolbar, manager->launch_button);
    
    // Create game grid
    manager->game_grid = raeenui_create_container(RAEENUI_CONTAINER_GRID);
    raeenui_container_set_grid_columns(manager->game_grid, 4);
    raeenui_container_add_child(content_area, manager->game_grid);
    
    // Create status bar
    manager->status_bar = raeenui_create_container(RAEENUI_CONTAINER_HORIZONTAL);
    raeenui_container_set_height(manager->status_bar, 30);
    raeenui_container_add_child(content_area, manager->status_bar);
    
    // Add download progress bar
    manager->download_progress = raeenui_create_progress_bar();
    raeenui_container_add_child(manager->status_bar, manager->download_progress);
    
    // Set up event handlers
    raeenui_button_set_click_handler(manager->launch_button, 
        (RaeenUIEventHandler)raeen_game_manager_on_launch_clicked, manager);
    raeenui_button_set_click_handler(manager->install_button,
        (RaeenUIEventHandler)raeen_game_manager_on_install_clicked, manager);
    raeenui_button_set_click_handler(manager->settings_button,
        (RaeenUIEventHandler)raeen_game_manager_on_settings_clicked, manager);
    
    return true;
}

bool raeen_game_manager_launch_game(RaeenGameManager* manager, GameEntry* game) {
    if (!manager || !game || !game->is_installed) return false;
    
    // Create application config
    RaeCompatAppConfig app_config = {0};
    app_config.name = game->name;
    app_config.executable_path = game->executable_path;
    app_config.app_type = game->app_type;
    app_config.wine_version = game->wine_version ? game->wine_version : "staging";
    app_config.dxvk_enabled = game->dxvk_enabled;
    app_config.vkd3d_enabled = game->vkd3d_enabled;
    
    // Register application with RaeCompat
    RaeCompatApplication* app = raecompat_register_application(
        manager->compat_context, &app_config);
    if (!app) return false;
    
    // Launch the application
    RaeCompatProcessInfo* process = raecompat_launch_application(
        manager->compat_context, game->name);
    if (!process) return false;
    
    // Update game statistics
    game->launch_count++;
    game->last_played = time(NULL);
    
    // Save updated stats
    raeen_game_manager_save_config(manager);
    
    return true;
}

bool raeen_game_manager_scan_libraries(RaeenGameManager* manager) {
    if (!manager) return false;
    
    // Create default library if none exist
    if (manager->library_count == 0) {
        GameLibrary* default_lib = malloc(sizeof(GameLibrary));
        memset(default_lib, 0, sizeof(GameLibrary));
        
        default_lib->name = strdup("My Games");
        default_lib->path = strdup(manager->games_directory);
        default_lib->type = RAEEN_GAME_LIBRARY_LOCAL;
        default_lib->auto_scan = true;
        
        manager->libraries = malloc(sizeof(GameLibrary*));
        manager->libraries[0] = default_lib;
        manager->library_count = 1;
    }
    
    // Scan all libraries
    for (int i = 0; i < manager->library_count; i++) {
        GameLibrary* lib = manager->libraries[i];
        if (lib->auto_scan && lib->type == RAEEN_GAME_LIBRARY_LOCAL) {
            raeen_game_manager_scan_directory(lib, lib->path);
        }
    }
    
    // Update UI
    raeen_game_manager_refresh_game_grid(manager);
    
    return true;
}

bool raeen_game_manager_refresh_game_grid(RaeenGameManager* manager) {
    if (!manager || !manager->game_grid) return false;
    
    // Clear existing grid
    raeenui_container_clear_children(manager->game_grid);
    
    // Add games from all libraries
    for (int i = 0; i < manager->library_count; i++) {
        GameLibrary* lib = manager->libraries[i];
        for (int j = 0; j < lib->game_count; j++) {
            GameEntry* game = lib->games[j];
            
            // Create game card
            RaeenUIContainer* game_card = raeenui_create_container(RAEENUI_CONTAINER_VERTICAL);
            raeenui_container_set_width(game_card, 200);
            raeenui_container_set_height(game_card, 280);
            
            // Add cover art
            RaeenUIImage* cover_image;
            if (game->cover_art_path && raeen_game_manager_file_exists(game->cover_art_path)) {
                cover_image = raeenui_create_image(game->cover_art_path);
            } else {
                cover_image = raeenui_create_image("/usr/share/raeenos/icons/game_default.png");
            }
            raeenui_container_add_child(game_card, cover_image);
            
            // Add game title
            RaeenUILabel* title_label = raeenui_create_label(game->name);
            raeenui_label_set_font_size(title_label, 14);
            raeenui_label_set_font_weight(title_label, RAEENUI_FONT_WEIGHT_BOLD);
            raeenui_container_add_child(game_card, title_label);
            
            // Add developer
            if (game->developer) {
                RaeenUILabel* dev_label = raeenui_create_label(game->developer);
                raeenui_label_set_font_size(dev_label, 12);
                raeenui_label_set_color(dev_label, 0x808080);
                raeenui_container_add_child(game_card, dev_label);
            }
            
            // Add ProtonDB rating if available
            if (game->protondb_tier) {
                RaeenUILabel* rating_label = raeenui_create_label(game->protondb_tier);
                raeenui_label_set_font_size(rating_label, 10);
                
                // Color code based on rating
                if (strcmp(game->protondb_tier, "Platinum") == 0) {
                    raeenui_label_set_color(rating_label, 0x00FF00);
                } else if (strcmp(game->protondb_tier, "Gold") == 0) {
                    raeenui_label_set_color(rating_label, 0xFFD700);
                } else if (strcmp(game->protondb_tier, "Silver") == 0) {
                    raeenui_label_set_color(rating_label, 0xC0C0C0);
                } else if (strcmp(game->protondb_tier, "Bronze") == 0) {
                    raeenui_label_set_color(rating_label, 0xCD7F32);
                } else {
                    raeenui_label_set_color(rating_label, 0xFF0000);
                }
                
                raeenui_container_add_child(game_card, rating_label);
            }
            
            // Set click handler
            raeenui_container_set_click_handler(game_card,
                (RaeenUIEventHandler)raeen_game_manager_on_game_selected, game);
            
            raeenui_container_add_child(manager->game_grid, game_card);
        }
    }
    
    return true;
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void raeen_game_manager_on_game_selected(GameEntry* game, RaeenGameManager* manager) {
    if (!game || !manager) return;
    
    manager->selected_game = game;
    
    // Update button states
    raeenui_button_set_enabled(manager->launch_button, game->is_installed);
    raeenui_button_set_enabled(manager->install_button, !game->is_installed);
}

void raeen_game_manager_on_launch_clicked(RaeenGameManager* manager) {
    if (!manager || !manager->selected_game) return;
    
    raeen_game_manager_launch_game(manager, manager->selected_game);
}

void raeen_game_manager_on_install_clicked(RaeenGameManager* manager) {
    if (!manager || !manager->selected_game) return;
    
    raeen_game_manager_install_game(manager, manager->selected_game);
}

void raeen_game_manager_on_settings_clicked(RaeenGameManager* manager) {
    if (!manager) return;
    
    raeen_game_manager_show_settings_dialog(manager);
}

// ============================================================================
// CONFIGURATION MANAGEMENT
// ============================================================================

bool raeen_game_manager_load_config(RaeenGameManager* manager) {
    if (!manager) return false;
    
    FILE* file = fopen(manager->config_file, "r");
    if (!file) return false;
    
    // Read JSON configuration
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* json_string = malloc(file_size + 1);
    fread(json_string, 1, file_size, file);
    json_string[file_size] = '\0';
    fclose(file);
    
    json_object* root = json_tokener_parse(json_string);
    free(json_string);
    
    if (!root) return false;
    
    // Parse configuration
    json_object* libraries_array;
    if (json_object_object_get_ex(root, "libraries", &libraries_array)) {
        int lib_count = json_object_array_length(libraries_array);
        manager->libraries = malloc(sizeof(GameLibrary*) * lib_count);
        manager->library_count = 0;
        
        for (int i = 0; i < lib_count; i++) {
            json_object* lib_obj = json_object_array_get_idx(libraries_array, i);
            GameLibrary* lib = malloc(sizeof(GameLibrary));
            memset(lib, 0, sizeof(GameLibrary));
            
            json_object* name_obj, *path_obj, *type_obj;
            if (json_object_object_get_ex(lib_obj, "name", &name_obj)) {
                lib->name = strdup(json_object_get_string(name_obj));
            }
            if (json_object_object_get_ex(lib_obj, "path", &path_obj)) {
                lib->path = strdup(json_object_get_string(path_obj));
            }
            if (json_object_object_get_ex(lib_obj, "type", &type_obj)) {
                lib->type = json_object_get_int(type_obj);
            }
            
            manager->libraries[manager->library_count++] = lib;
        }
    }
    
    json_object_put(root);
    return true;
}

bool raeen_game_manager_save_config(RaeenGameManager* manager) {
    if (!manager) return false;
    
    json_object* root = json_object_new_object();
    json_object* libraries_array = json_object_new_array();
    
    // Save libraries
    for (int i = 0; i < manager->library_count; i++) {
        GameLibrary* lib = manager->libraries[i];
        json_object* lib_obj = json_object_new_object();
        
        json_object_object_add(lib_obj, "name", json_object_new_string(lib->name));
        json_object_object_add(lib_obj, "path", json_object_new_string(lib->path));
        json_object_object_add(lib_obj, "type", json_object_new_int(lib->type));
        json_object_object_add(lib_obj, "auto_scan", json_object_new_boolean(lib->auto_scan));
        
        json_object_array_add(libraries_array, lib_obj);
    }
    
    json_object_object_add(root, "libraries", libraries_array);
    
    // Write to file
    FILE* file = fopen(manager->config_file, "w");
    if (!file) {
        json_object_put(root);
        return false;
    }
    
    const char* json_string = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    fprintf(file, "%s", json_string);
    fclose(file);
    
    json_object_put(root);
    return true;
}

// ============================================================================
// UTILITY IMPLEMENTATIONS
// ============================================================================

static bool raeen_game_manager_file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

bool raeen_game_manager_show_settings_dialog(RaeenGameManager* manager) {
    // Implementation would create a settings dialog
    // This is a placeholder for the UI framework integration
    return true;
}

bool raeen_game_manager_install_game(RaeenGameManager* manager, GameEntry* game) {
    // Implementation would handle game installation
    // This is a placeholder for the installation system
    return true;
}
