/**
 * RaeenOS File Manager Application
 * Modern file browser with RaeenUI integration
 */

#include "../ui/raeenui.h"
#include "../ui/desktop_shell.h"
#include "../filesystem/fat32_production.h"
#include "../memory.h"
#include "../string.h"

// File Entry Structure
typedef struct file_entry {
    char name[256];
    char path[512];
    uint64_t size;
    bool is_directory;
    uint32_t modified_time;
    struct file_entry* next;
} file_entry_t;

// File Manager State
typedef struct {
    RaeenUIWindow* window;
    RaeenUIView* toolbar;
    RaeenUIView* address_bar;
    RaeenUIView* file_list;
    RaeenUIView* status_bar;
    RaeenUIView* sidebar;
    
    char current_path[512];
    file_entry_t* files;
    file_entry_t* selected_file;
    
    // View options
    bool show_hidden_files;
    bool list_view;
    bool details_view;
    
    // Navigation history
    char* history[50];
    uint32_t history_index;
    uint32_t history_count;
} file_manager_t;

static file_manager_t g_file_manager = {0};

// Function declarations
static bool file_manager_init(void);
static void file_manager_create_ui(void);
static void file_manager_refresh_files(void);
static void file_manager_navigate_to(const char* path);
static void file_manager_handle_file_click(RaeenUIView* view, void* user_data);
static void file_manager_handle_back_button(RaeenUIView* view, void* user_data);
static void file_manager_handle_forward_button(RaeenUIView* view, void* user_data);
static void file_manager_handle_up_button(RaeenUIView* view, void* user_data);
static void file_manager_handle_new_folder(RaeenUIView* view, void* user_data);
static void file_manager_handle_delete_file(RaeenUIView* view, void* user_data);
static void file_manager_handle_copy_file(RaeenUIView* view, void* user_data);
static void file_manager_handle_paste_file(RaeenUIView* view, void* user_data);
static void file_manager_add_to_history(const char* path);
static RaeenUIView* file_manager_create_file_item(file_entry_t* file);

/**
 * Launch file manager application
 */
int file_manager_main(int argc, char* argv[]) {
    printf("File Manager: Starting application...\n");
    
    if (!file_manager_init()) {
        printf("File Manager: Failed to initialize\n");
        return 1;
    }
    
    file_manager_create_ui();
    file_manager_navigate_to("/");
    
    printf("File Manager: Application started successfully\n");
    return 0;
}

/**
 * Initialize file manager
 */
static bool file_manager_init(void) {
    file_manager_t* fm = &g_file_manager;
    
    memory_set(fm, 0, sizeof(file_manager_t));
    string_copy(fm->current_path, "/", sizeof(fm->current_path));
    fm->list_view = true;
    
    return true;
}

/**
 * Create file manager UI
 */
static void file_manager_create_ui(void) {
    file_manager_t* fm = &g_file_manager;
    
    // Create main window
    fm->window = desktop_shell_create_application_window("File Manager", 1, 800, 600);
    if (!fm->window) {
        printf("File Manager: Failed to create window\n");
        return;
    }
    
    // Create main layout
    RaeenUIView* main_layout = RaeenUIVStack(0);
    
    // Create toolbar
    fm->toolbar = RaeenUIHStack(8);
    raeenui_view_set_size(fm->toolbar, (RaeenUISize){0, 40});
    raeenui_background(fm->toolbar, raeenui_color_rgba(0.95f, 0.95f, 0.95f, 1.0f));
    raeenui_padding(fm->toolbar, 8);
    
    // Navigation buttons
    RaeenUIView* back_btn = RaeenUIButton("←", file_manager_handle_back_button);
    RaeenUIView* forward_btn = RaeenUIButton("→", file_manager_handle_forward_button);
    RaeenUIView* up_btn = RaeenUIButton("↑", file_manager_handle_up_button);
    
    raeenui_view_set_size(back_btn, (RaeenUISize){32, 24});
    raeenui_view_set_size(forward_btn, (RaeenUISize){32, 24});
    raeenui_view_set_size(up_btn, (RaeenUISize){32, 24});
    
    // Address bar
    fm->address_bar = RaeenUIInput("/");
    raeenui_view_set_flex_grow(fm->address_bar, 1.0f);
    raeenui_background(fm->address_bar, raeenui_color_white());
    raeenui_corner_radius(fm->address_bar, 4);
    raeenui_padding(fm->address_bar, 8);
    
    // Toolbar buttons
    RaeenUIView* new_folder_btn = RaeenUIButton("New Folder", file_manager_handle_new_folder);
    RaeenUIView* view_btn = RaeenUIButton("View", NULL);
    
    raeenui_view_add_child(fm->toolbar, back_btn);
    raeenui_view_add_child(fm->toolbar, forward_btn);
    raeenui_view_add_child(fm->toolbar, up_btn);
    raeenui_view_add_child(fm->toolbar, fm->address_bar);
    raeenui_view_add_child(fm->toolbar, new_folder_btn);
    raeenui_view_add_child(fm->toolbar, view_btn);
    
    // Create content area with sidebar
    RaeenUIView* content_area = RaeenUIHStack(0);
    raeenui_view_set_flex_grow(content_area, 1.0f);
    
    // Sidebar
    fm->sidebar = RaeenUIVStack(8);
    raeenui_view_set_size(fm->sidebar, (RaeenUISize){200, 0});
    raeenui_background(fm->sidebar, raeenui_color_rgba(0.98f, 0.98f, 0.98f, 1.0f));
    raeenui_padding(fm->sidebar, 12);
    
    // Quick access items
    RaeenUIView* quick_access = RaeenUIText("Quick Access");
    raeenui_font_weight(quick_access, RAEENUI_FONT_WEIGHT_BOLD);
    raeenui_font_size(quick_access, 12);
    
    RaeenUIView* desktop_item = RaeenUIButton("🖥️ Desktop", NULL);
    RaeenUIView* documents_item = RaeenUIButton("📄 Documents", NULL);
    RaeenUIView* downloads_item = RaeenUIButton("⬇️ Downloads", NULL);
    RaeenUIView* pictures_item = RaeenUIButton("🖼️ Pictures", NULL);
    
    raeenui_view_add_child(fm->sidebar, quick_access);
    raeenui_view_add_child(fm->sidebar, desktop_item);
    raeenui_view_add_child(fm->sidebar, documents_item);
    raeenui_view_add_child(fm->sidebar, downloads_item);
    raeenui_view_add_child(fm->sidebar, pictures_item);
    
    // File list area
    fm->file_list = RaeenUIScrollView();
    raeenui_view_set_flex_grow(fm->file_list, 1.0f);
    raeenui_background(fm->file_list, raeenui_color_white());
    
    raeenui_view_add_child(content_area, fm->sidebar);
    raeenui_view_add_child(content_area, fm->file_list);
    
    // Status bar
    fm->status_bar = RaeenUIHStack(8);
    raeenui_view_set_size(fm->status_bar, (RaeenUISize){0, 24});
    raeenui_background(fm->status_bar, raeenui_color_rgba(0.9f, 0.9f, 0.9f, 1.0f));
    raeenui_padding(fm->status_bar, 8);
    
    RaeenUIView* status_text = RaeenUIText("Ready");
    raeenui_font_size(status_text, 11);
    raeenui_view_add_child(fm->status_bar, status_text);
    
    // Assemble main layout
    raeenui_view_add_child(main_layout, fm->toolbar);
    raeenui_view_add_child(main_layout, content_area);
    raeenui_view_add_child(main_layout, fm->status_bar);
    
    raeenui_window_set_content_view(fm->window, main_layout);
}

/**
 * Navigate to directory
 */
static void file_manager_navigate_to(const char* path) {
    file_manager_t* fm = &g_file_manager;
    
    if (!path) return;
    
    // Update current path
    string_copy(fm->current_path, path, sizeof(fm->current_path));
    
    // Add to history
    file_manager_add_to_history(path);
    
    // Update address bar
    raeenui_input_set_text(fm->address_bar, path);
    
    // Refresh file list
    file_manager_refresh_files();
    
    printf("File Manager: Navigated to %s\n", path);
}

/**
 * Refresh file list
 */
static void file_manager_refresh_files(void) {
    file_manager_t* fm = &g_file_manager;
    
    // Clear existing files
    file_entry_t* file = fm->files;
    while (file) {
        file_entry_t* next = file->next;
        memory_free(file);
        file = next;
    }
    fm->files = NULL;
    
    // Clear file list view
    raeenui_view_remove_all_children(fm->file_list);
    
    // Create file list container
    RaeenUIView* file_container = RaeenUIVStack(2);
    raeenui_padding(file_container, 8);
    
    // Simulate reading directory (would use actual filesystem)
    const char* demo_files[] = {
        "Documents", "Downloads", "Pictures", "Music", "Videos",
        "readme.txt", "config.ini", "app.exe", "data.bin", NULL
    };
    
    bool demo_is_dir[] = {true, true, true, true, true, false, false, false, false};
    uint64_t demo_sizes[] = {0, 0, 0, 0, 0, 1024, 512, 2048, 4096};
    
    for (int i = 0; demo_files[i] != NULL; i++) {
        file_entry_t* file = (file_entry_t*)memory_alloc(sizeof(file_entry_t));
        if (!file) continue;
        
        memory_set(file, 0, sizeof(file_entry_t));
        string_copy(file->name, demo_files[i], sizeof(file->name));
        string_format(file->path, sizeof(file->path), "%s/%s", fm->current_path, demo_files[i]);
        file->is_directory = demo_is_dir[i];
        file->size = demo_sizes[i];
        file->modified_time = 1640995200; // Jan 1, 2022
        
        // Add to file list
        file->next = fm->files;
        fm->files = file;
        
        // Create UI item
        RaeenUIView* file_item = file_manager_create_file_item(file);
        raeenui_view_add_child(file_container, file_item);
    }
    
    raeenui_scroll_view_set_content(fm->file_list, file_container);
    
    // Update status bar
    uint32_t file_count = 0;
    uint32_t folder_count = 0;
    file = fm->files;
    while (file) {
        if (file->is_directory) {
            folder_count++;
        } else {
            file_count++;
        }
        file = file->next;
    }
    
    char status_text[128];
    string_format(status_text, sizeof(status_text), "%u folders, %u files", folder_count, file_count);
    // TODO: Update status bar text
}

/**
 * Create file item UI
 */
static RaeenUIView* file_manager_create_file_item(file_entry_t* file) {
    RaeenUIView* item = RaeenUIHStack(8);
    raeenui_view_set_size(item, (RaeenUISize){0, 32});
    raeenui_padding(item, 8);
    raeenui_corner_radius(item, 4);
    
    // File icon
    RaeenUIView* icon = RaeenUIText(file->is_directory ? "📁" : "📄");
    raeenui_view_set_size(icon, (RaeenUISize){24, 24});
    
    // File name
    RaeenUIView* name = RaeenUIText(file->name);
    raeenui_font_size(name, 12);
    raeenui_view_set_flex_grow(name, 1.0f);
    
    // File size (for files only)
    RaeenUIView* size = NULL;
    if (!file->is_directory) {
        char size_str[32];
        if (file->size < 1024) {
            string_format(size_str, sizeof(size_str), "%llu B", file->size);
        } else if (file->size < 1024 * 1024) {
            string_format(size_str, sizeof(size_str), "%.1f KB", file->size / 1024.0);
        } else {
            string_format(size_str, sizeof(size_str), "%.1f MB", file->size / (1024.0 * 1024.0));
        }
        
        size = RaeenUIText(size_str);
        raeenui_font_size(size, 11);
        raeenui_foreground(size, raeenui_color_rgba(0.6f, 0.6f, 0.6f, 1.0f));
        raeenui_view_set_size(size, (RaeenUISize){80, 24});
    }
    
    // Add hover effect
    raeenui_onHover(item, NULL, NULL); // TODO: Implement hover handlers
    
    // Add click handler
    raeenui_onClick(item, file_manager_handle_file_click, file);
    
    raeenui_view_add_child(item, icon);
    raeenui_view_add_child(item, name);
    if (size) {
        raeenui_view_add_child(item, size);
    }
    
    return item;
}

/**
 * Add path to navigation history
 */
static void file_manager_add_to_history(const char* path) {
    file_manager_t* fm = &g_file_manager;
    
    // Remove any forward history
    for (uint32_t i = fm->history_index + 1; i < fm->history_count; i++) {
        if (fm->history[i]) {
            memory_free(fm->history[i]);
            fm->history[i] = NULL;
        }
    }
    fm->history_count = fm->history_index + 1;
    
    // Add new path
    if (fm->history_count < 50) {
        fm->history[fm->history_count] = string_duplicate(path);
        fm->history_count++;
        fm->history_index = fm->history_count - 1;
    }
}

// Event handlers

static void file_manager_handle_file_click(RaeenUIView* view, void* user_data) {
    file_entry_t* file = (file_entry_t*)user_data;
    file_manager_t* fm = &g_file_manager;
    
    if (!file) return;
    
    fm->selected_file = file;
    
    if (file->is_directory) {
        // Navigate to directory
        file_manager_navigate_to(file->path);
    } else {
        // Open file (would launch appropriate application)
        printf("File Manager: Opening file %s\n", file->name);
        
        // Simple file type detection
        if (string_ends_with(file->name, ".txt")) {
            // Launch text editor
            printf("File Manager: Launching text editor for %s\n", file->name);
        } else if (string_ends_with(file->name, ".exe")) {
            // Execute program
            printf("File Manager: Executing %s\n", file->name);
        }
    }
}

static void file_manager_handle_back_button(RaeenUIView* view, void* user_data) {
    file_manager_t* fm = &g_file_manager;
    
    if (fm->history_index > 0) {
        fm->history_index--;
        file_manager_navigate_to(fm->history[fm->history_index]);
    }
}

static void file_manager_handle_forward_button(RaeenUIView* view, void* user_data) {
    file_manager_t* fm = &g_file_manager;
    
    if (fm->history_index < fm->history_count - 1) {
        fm->history_index++;
        file_manager_navigate_to(fm->history[fm->history_index]);
    }
}

static void file_manager_handle_up_button(RaeenUIView* view, void* user_data) {
    file_manager_t* fm = &g_file_manager;
    
    // Navigate to parent directory
    char parent_path[512];
    string_copy(parent_path, fm->current_path, sizeof(parent_path));
    
    char* last_slash = string_find_last(parent_path, '/');
    if (last_slash && last_slash != parent_path) {
        *last_slash = '\0';
        file_manager_navigate_to(parent_path);
    } else if (string_compare(parent_path, "/") != 0) {
        file_manager_navigate_to("/");
    }
}

static void file_manager_handle_new_folder(RaeenUIView* view, void* user_data) {
    file_manager_t* fm = &g_file_manager;
    
    // Create new folder dialog (simplified)
    char folder_name[64];
    string_copy(folder_name, "New Folder", sizeof(folder_name));
    
    char folder_path[512];
    string_format(folder_path, sizeof(folder_path), "%s/%s", fm->current_path, folder_name);
    
    // TODO: Create actual directory
    printf("File Manager: Creating folder %s\n", folder_path);
    
    // Refresh view
    file_manager_refresh_files();
}

static void file_manager_handle_delete_file(RaeenUIView* view, void* user_data) {
    file_manager_t* fm = &g_file_manager;
    
    if (!fm->selected_file) return;
    
    printf("File Manager: Deleting %s\n", fm->selected_file->name);
    
    if (fm->selected_file->is_directory) {
        // TODO: Remove directory
    } else {
        // Delete file
        fat32_delete_file(fm->selected_file->path);
    }
    
    file_manager_refresh_files();
}
