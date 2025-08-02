/**
 * RaeenOS Desktop Shell Implementation
 * Modern desktop environment with window management and compositor integration
 */

#include "raeenui.h"
#include "../gpu/graphics_pipeline.h"
#include "../gpu/compositor.h"
#include "../input/input_manager.h"
#include "../memory.h"
#include "../string.h"

// Desktop Shell Components
typedef struct {
    RaeenUIView* taskbar;
    RaeenUIView* start_menu;
    RaeenUIView* system_tray;
    RaeenUIView* desktop_background;
    RaeenUIView* window_switcher;
    bool start_menu_visible;
    bool window_switcher_visible;
} desktop_shell_t;

// Window Management
typedef struct desktop_window {
    RaeenUIWindow* ui_window;
    RaeenUIView* title_bar;
    RaeenUIView* content_area;
    RaeenUIView* close_button;
    RaeenUIView* minimize_button;
    RaeenUIView* maximize_button;
    
    char title[256];
    uint32_t process_id;
    bool minimized;
    bool maximized;
    bool focused;
    
    RaeenUIRect normal_frame;
    RaeenUIRect maximized_frame;
    
    struct desktop_window* next;
} desktop_window_t;

// Application Launcher Entry
typedef struct app_entry {
    char name[64];
    char icon_path[256];
    char executable[256];
    char description[128];
    struct app_entry* next;
} app_entry_t;

// Desktop Shell State
static desktop_shell_t g_desktop_shell = {0};
static desktop_window_t* g_windows = NULL;
static app_entry_t* g_applications = NULL;
static RaeenUIContext* g_ui_context = NULL;
static bool g_shell_initialized = false;

// Function declarations
static void desktop_shell_create_taskbar(void);
static void desktop_shell_create_start_menu(void);
static void desktop_shell_create_system_tray(void);
static void desktop_shell_create_background(void);
static void desktop_shell_handle_start_button_click(RaeenUIView* view, void* user_data);
static void desktop_shell_handle_window_close(RaeenUIView* view, void* user_data);
static void desktop_shell_handle_window_minimize(RaeenUIView* view, void* user_data);
static void desktop_shell_handle_window_maximize(RaeenUIView* view, void* user_data);
static void desktop_shell_handle_app_launch(RaeenUIView* view, void* user_data);
static desktop_window_t* desktop_shell_create_window(const char* title, uint32_t process_id);
static void desktop_shell_focus_window(desktop_window_t* window);
static void desktop_shell_minimize_window(desktop_window_t* window);
static void desktop_shell_maximize_window(desktop_window_t* window);
static void desktop_shell_close_window(desktop_window_t* window);
static void desktop_shell_load_applications(void);
static void desktop_shell_update_taskbar(void);

/**
 * Initialize desktop shell
 */
bool desktop_shell_init(void) {
    if (g_shell_initialized) {
        return true;
    }
    
    // Initialize UI context
    g_ui_context = raeenui_context_create();
    if (!g_ui_context) {
        printf("Desktop Shell: Failed to create UI context\n");
        return false;
    }
    
    // Set desktop theme
    raeenui_context_set_theme(g_ui_context, RAEENUI_THEME_FLUENT);
    
    // Create desktop components
    desktop_shell_create_background();
    desktop_shell_create_taskbar();
    desktop_shell_create_start_menu();
    desktop_shell_create_system_tray();
    
    // Load application entries
    desktop_shell_load_applications();
    
    g_shell_initialized = true;
    printf("Desktop Shell: Initialized successfully\n");
    
    return true;
}

/**
 * Update desktop shell (called each frame)
 */
void desktop_shell_update(void) {
    if (!g_shell_initialized) return;
    
    // Update UI context
    raeenui_context_update(g_ui_context);
    
    // Handle input events
    input_event_t event;
    while (input_manager_get_event(&event)) {
        switch (event.type) {
            case INPUT_EVENT_KEY:
                if (event.key.pressed) {
                    // Alt+Tab for window switching
                    if (event.key.key_code == KEY_TAB && (event.key.modifiers & KEY_MOD_ALT)) {
                        desktop_shell_show_window_switcher();
                    }
                    // Windows key for start menu
                    else if (event.key.key_code == KEY_LWIN) {
                        desktop_shell_toggle_start_menu();
                    }
                }
                break;
                
            case INPUT_EVENT_MOUSE:
                // Pass mouse events to UI system
                raeenui_context_handle_mouse_event(g_ui_context, &event.mouse);
                break;
        }
    }
    
    // Update taskbar with current windows
    desktop_shell_update_taskbar();
}

/**
 * Render desktop shell
 */
void desktop_shell_render(void) {
    if (!g_shell_initialized) return;
    
    // Render all UI components
    raeenui_context_render(g_ui_context);
}

/**
 * Create application window
 */
desktop_window_t* desktop_shell_create_application_window(const char* title, uint32_t process_id, 
                                                         uint32_t width, uint32_t height) {
    if (!g_shell_initialized) return NULL;
    
    desktop_window_t* window = desktop_shell_create_window(title, process_id);
    if (!window) return NULL;
    
    // Set window size
    RaeenUIRect frame = {100, 100, width, height};
    raeenui_window_set_frame(window->ui_window, frame);
    
    // Focus new window
    desktop_shell_focus_window(window);
    
    printf("Desktop Shell: Created window '%s' for process %u\n", title, process_id);
    return window;
}

/**
 * Toggle start menu visibility
 */
void desktop_shell_toggle_start_menu(void) {
    if (!g_shell_initialized) return;
    
    g_desktop_shell.start_menu_visible = !g_desktop_shell.start_menu_visible;
    raeenui_view_set_hidden(g_desktop_shell.start_menu, !g_desktop_shell.start_menu_visible);
    
    if (g_desktop_shell.start_menu_visible) {
        // Animate start menu appearance
        raeenui_animate_opacity(g_desktop_shell.start_menu, 0.0f, 1.0f, 0.2f, RAEENUI_EASE_OUT);
        raeenui_animate_frame(g_desktop_shell.start_menu, 
                             (RaeenUIRect){0, -300, 300, 400},
                             (RaeenUIRect){0, 50, 300, 400}, 0.3f, RAEENUI_EASE_BOUNCE);
    }
}

/**
 * Show window switcher (Alt+Tab)
 */
void desktop_shell_show_window_switcher(void) {
    if (!g_shell_initialized) return;
    
    // Create window switcher if not exists
    if (!g_desktop_shell.window_switcher) {
        g_desktop_shell.window_switcher = RaeenUIVStack(10);
        raeenui_background(g_desktop_shell.window_switcher, raeenui_color_rgba(0, 0, 0, 0.8f));
        raeenui_corner_radius(g_desktop_shell.window_switcher, 12);
        raeenui_padding(g_desktop_shell.window_switcher, 20);
        
        // Add window entries
        desktop_window_t* window = g_windows;
        while (window) {
            if (!window->minimized) {
                RaeenUIView* window_entry = RaeenUIHStack(10);
                
                // Window icon (placeholder)
                RaeenUIView* icon = RaeenUIView();
                raeenui_view_set_size(icon, (RaeenUISize){32, 32});
                raeenui_background(icon, raeenui_color_hex(0x0078D4));
                raeenui_corner_radius(icon, 4);
                
                // Window title
                RaeenUIView* title = RaeenUIText(window->title);
                raeenui_font_size(title, 14);
                raeenui_foreground(title, raeenui_color_white());
                
                raeenui_view_add_child(window_entry, icon);
                raeenui_view_add_child(window_entry, title);
                raeenui_view_add_child(g_desktop_shell.window_switcher, window_entry);
            }
            window = window->next;
        }
        
        raeenui_context_add_overlay(g_ui_context, g_desktop_shell.window_switcher);
    }
    
    g_desktop_shell.window_switcher_visible = true;
    raeenui_view_set_hidden(g_desktop_shell.window_switcher, false);
    
    // Center on screen
    RaeenUISize screen_size = raeenui_context_get_screen_size(g_ui_context);
    RaeenUIRect switcher_frame = {
        (screen_size.width - 400) / 2,
        (screen_size.height - 200) / 2,
        400, 200
    };
    raeenui_view_set_frame(g_desktop_shell.window_switcher, switcher_frame);
    
    // Animate appearance
    raeenui_animate_opacity(g_desktop_shell.window_switcher, 0.0f, 1.0f, 0.2f, RAEENUI_EASE_OUT);
}

// Internal helper functions

static void desktop_shell_create_taskbar(void) {
    RaeenUISize screen_size = raeenui_context_get_screen_size(g_ui_context);
    
    // Create taskbar container
    g_desktop_shell.taskbar = RaeenUIHStack(8);
    raeenui_view_set_frame(g_desktop_shell.taskbar, 
                          (RaeenUIRect){0, screen_size.height - 48, screen_size.width, 48});
    raeenui_background(g_desktop_shell.taskbar, raeenui_color_rgba(0.1f, 0.1f, 0.1f, 0.95f));
    raeenui_blur(g_desktop_shell.taskbar, 10);
    raeenui_padding(g_desktop_shell.taskbar, 8);
    
    // Start button
    RaeenUIView* start_button = RaeenUIButton("⊞", desktop_shell_handle_start_button_click);
    raeenui_view_set_size(start_button, (RaeenUISize){40, 32});
    raeenui_background(start_button, raeenui_color_hex(0x0078D4));
    raeenui_corner_radius(start_button, 6);
    raeenui_font_size(start_button, 16);
    raeenui_foreground(start_button, raeenui_color_white());
    
    // Task list area (will be populated dynamically)
    RaeenUIView* task_list = RaeenUIHStack(4);
    raeenui_view_set_flex_grow(task_list, 1.0f);
    
    // System tray area
    RaeenUIView* tray_area = RaeenUIHStack(4);
    raeenui_view_set_size(tray_area, (RaeenUISize){200, 32});
    
    // Clock
    char time_str[32];
    // TODO: Get actual time
    string_copy(time_str, "12:34 PM", sizeof(time_str));
    RaeenUIView* clock = RaeenUIText(time_str);
    raeenui_font_size(clock, 12);
    raeenui_foreground(clock, raeenui_color_white());
    
    raeenui_view_add_child(g_desktop_shell.taskbar, start_button);
    raeenui_view_add_child(g_desktop_shell.taskbar, task_list);
    raeenui_view_add_child(g_desktop_shell.taskbar, tray_area);
    raeenui_view_add_child(tray_area, clock);
    
    raeenui_context_add_view(g_ui_context, g_desktop_shell.taskbar);
}

static void desktop_shell_create_start_menu(void) {
    // Create start menu container
    g_desktop_shell.start_menu = RaeenUIVStack(8);
    raeenui_view_set_frame(g_desktop_shell.start_menu, (RaeenUIRect){0, 50, 300, 400});
    raeenui_background(g_desktop_shell.start_menu, raeenui_color_rgba(0.05f, 0.05f, 0.05f, 0.95f));
    raeenui_blur(g_desktop_shell.start_menu, 20);
    raeenui_corner_radius(g_desktop_shell.start_menu, 12);
    raeenui_padding(g_desktop_shell.start_menu, 16);
    raeenui_shadow(g_desktop_shell.start_menu, raeenui_color_rgba(0, 0, 0, 0.3f), 8, (RaeenUIOffset){0, 4});
    
    // Search box
    RaeenUIView* search_box = RaeenUIInput("Search apps...");
    raeenui_background(search_box, raeenui_color_rgba(1, 1, 1, 0.1f));
    raeenui_corner_radius(search_box, 8);
    raeenui_padding(search_box, 12);
    raeenui_font_size(search_box, 14);
    
    // Applications grid
    RaeenUIView* apps_grid = RaeenUIGrid(3, 10); // 3 columns, 10px spacing
    
    // Add application entries
    app_entry_t* app = g_applications;
    while (app) {
        RaeenUIView* app_button = RaeenUIVStack(4);
        raeenui_view_set_size(app_button, (RaeenUISize){80, 80});
        raeenui_corner_radius(app_button, 8);
        raeenui_onClick(app_button, desktop_shell_handle_app_launch, app);
        
        // App icon (placeholder)
        RaeenUIView* icon = RaeenUIView();
        raeenui_view_set_size(icon, (RaeenUISize){48, 48});
        raeenui_background(icon, raeenui_color_hex(0x0078D4));
        raeenui_corner_radius(icon, 12);
        
        // App name
        RaeenUIView* name = RaeenUIText(app->name);
        raeenui_font_size(name, 10);
        raeenui_foreground(name, raeenui_color_white());
        
        raeenui_view_add_child(app_button, icon);
        raeenui_view_add_child(app_button, name);
        raeenui_view_add_child(apps_grid, app_button);
        
        app = app->next;
    }
    
    // Power options
    RaeenUIView* power_section = RaeenUIHStack(8);
    RaeenUIView* shutdown_btn = RaeenUIButton("⏻", NULL);
    RaeenUIView* restart_btn = RaeenUIButton("⟲", NULL);
    RaeenUIView* sleep_btn = RaeenUIButton("☾", NULL);
    
    raeenui_view_set_size(shutdown_btn, (RaeenUISize){32, 32});
    raeenui_view_set_size(restart_btn, (RaeenUISize){32, 32});
    raeenui_view_set_size(sleep_btn, (RaeenUISize){32, 32});
    
    raeenui_background(shutdown_btn, raeenui_color_hex(0xE74C3C));
    raeenui_background(restart_btn, raeenui_color_hex(0xF39C12));
    raeenui_background(sleep_btn, raeenui_color_hex(0x3498DB));
    
    raeenui_corner_radius(shutdown_btn, 16);
    raeenui_corner_radius(restart_btn, 16);
    raeenui_corner_radius(sleep_btn, 16);
    
    raeenui_view_add_child(power_section, shutdown_btn);
    raeenui_view_add_child(power_section, restart_btn);
    raeenui_view_add_child(power_section, sleep_btn);
    
    raeenui_view_add_child(g_desktop_shell.start_menu, search_box);
    raeenui_view_add_child(g_desktop_shell.start_menu, apps_grid);
    raeenui_view_add_child(g_desktop_shell.start_menu, power_section);
    
    // Initially hidden
    raeenui_view_set_hidden(g_desktop_shell.start_menu, true);
    g_desktop_shell.start_menu_visible = false;
    
    raeenui_context_add_view(g_ui_context, g_desktop_shell.start_menu);
}

static void desktop_shell_create_background(void) {
    RaeenUISize screen_size = raeenui_context_get_screen_size(g_ui_context);
    
    g_desktop_shell.desktop_background = RaeenUIView();
    raeenui_view_set_frame(g_desktop_shell.desktop_background, 
                          (RaeenUIRect){0, 0, screen_size.width, screen_size.height});
    
    // Gradient background
    raeenui_background_gradient(g_desktop_shell.desktop_background,
                               raeenui_color_hex(0x1E3A8A),
                               raeenui_color_hex(0x3B82F6),
                               RAEENUI_GRADIENT_DIAGONAL);
    
    raeenui_context_add_view(g_ui_context, g_desktop_shell.desktop_background);
}

static desktop_window_t* desktop_shell_create_window(const char* title, uint32_t process_id) {
    desktop_window_t* window = (desktop_window_t*)memory_alloc(sizeof(desktop_window_t));
    if (!window) return NULL;
    
    memory_set(window, 0, sizeof(desktop_window_t));
    string_copy(window->title, title, sizeof(window->title));
    window->process_id = process_id;
    
    // Create UI window
    window->ui_window = raeenui_window_create(g_ui_context);
    if (!window->ui_window) {
        memory_free(window);
        return NULL;
    }
    
    // Create title bar
    window->title_bar = RaeenUIHStack(8);
    raeenui_view_set_size(window->title_bar, (RaeenUISize){0, 32});
    raeenui_background(window->title_bar, raeenui_color_rgba(0.2f, 0.2f, 0.2f, 0.9f));
    raeenui_padding(window->title_bar, 8);
    
    // Window title
    RaeenUIView* title_label = RaeenUIText(title);
    raeenui_font_size(title_label, 12);
    raeenui_foreground(title_label, raeenui_color_white());
    raeenui_view_set_flex_grow(title_label, 1.0f);
    
    // Window controls
    window->minimize_button = RaeenUIButton("−", desktop_shell_handle_window_minimize);
    window->maximize_button = RaeenUIButton("□", desktop_shell_handle_window_maximize);
    window->close_button = RaeenUIButton("×", desktop_shell_handle_window_close);
    
    raeenui_view_set_size(window->minimize_button, (RaeenUISize){24, 24});
    raeenui_view_set_size(window->maximize_button, (RaeenUISize){24, 24});
    raeenui_view_set_size(window->close_button, (RaeenUISize){24, 24});
    
    raeenui_background(window->close_button, raeenui_color_hex(0xE74C3C));
    raeenui_background(window->maximize_button, raeenui_color_hex(0xF39C12));
    raeenui_background(window->minimize_button, raeenui_color_hex(0x95A5A6));
    
    raeenui_corner_radius(window->close_button, 12);
    raeenui_corner_radius(window->maximize_button, 12);
    raeenui_corner_radius(window->minimize_button, 12);
    
    // Set user data for button callbacks
    raeenui_view_set_user_data(window->close_button, window);
    raeenui_view_set_user_data(window->maximize_button, window);
    raeenui_view_set_user_data(window->minimize_button, window);
    
    raeenui_view_add_child(window->title_bar, title_label);
    raeenui_view_add_child(window->title_bar, window->minimize_button);
    raeenui_view_add_child(window->title_bar, window->maximize_button);
    raeenui_view_add_child(window->title_bar, window->close_button);
    
    // Create content area
    window->content_area = RaeenUIView();
    raeenui_background(window->content_area, raeenui_color_white());
    raeenui_view_set_flex_grow(window->content_area, 1.0f);
    
    // Add to window
    RaeenUIView* window_root = RaeenUIVStack(0);
    raeenui_view_add_child(window_root, window->title_bar);
    raeenui_view_add_child(window_root, window->content_area);
    raeenui_window_set_content_view(window->ui_window, window_root);
    
    // Add to window list
    window->next = g_windows;
    g_windows = window;
    
    return window;
}

static void desktop_shell_load_applications(void) {
    // Load default applications (placeholder)
    const char* default_apps[][4] = {
        {"File Manager", "/icons/files.png", "/bin/files", "Browse files and folders"},
        {"Text Editor", "/icons/editor.png", "/bin/editor", "Edit text files"},
        {"Terminal", "/icons/terminal.png", "/bin/terminal", "Command line interface"},
        {"Web Browser", "/icons/browser.png", "/bin/browser", "Browse the web"},
        {"Settings", "/icons/settings.png", "/bin/settings", "System settings"},
        {"Calculator", "/icons/calc.png", "/bin/calc", "Perform calculations"},
        {NULL, NULL, NULL, NULL}
    };
    
    for (int i = 0; default_apps[i][0] != NULL; i++) {
        app_entry_t* app = (app_entry_t*)memory_alloc(sizeof(app_entry_t));
        if (app) {
            string_copy(app->name, default_apps[i][0], sizeof(app->name));
            string_copy(app->icon_path, default_apps[i][1], sizeof(app->icon_path));
            string_copy(app->executable, default_apps[i][2], sizeof(app->executable));
            string_copy(app->description, default_apps[i][3], sizeof(app->description));
            
            app->next = g_applications;
            g_applications = app;
        }
    }
}

// Event handlers
static void desktop_shell_handle_start_button_click(RaeenUIView* view, void* user_data) {
    desktop_shell_toggle_start_menu();
}

static void desktop_shell_handle_window_close(RaeenUIView* view, void* user_data) {
    desktop_window_t* window = (desktop_window_t*)user_data;
    if (window) {
        desktop_shell_close_window(window);
    }
}

static void desktop_shell_handle_app_launch(RaeenUIView* view, void* user_data) {
    app_entry_t* app = (app_entry_t*)user_data;
    if (app) {
        printf("Desktop Shell: Launching %s (%s)\n", app->name, app->executable);
        // TODO: Launch application process
        
        // Hide start menu
        g_desktop_shell.start_menu_visible = false;
        raeenui_view_set_hidden(g_desktop_shell.start_menu, true);
    }
}
