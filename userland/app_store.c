#include "app_store.h"
#include "../kernel/vga.h"
#include "../kernel/memory.h"
#include "../libs/libc/include/string.h"

// Placeholder for a list of available applications
static app_metadata_t available_apps[] = {
    {
        .id = 1,
        .name = "RaeText Editor",
        .description = "A simple text editor for RaeenOS.",
        .version = "1.0.0",
        .developer = "RaeenOS Team",
        .size_bytes = 1024 * 500, // 500 KB
        .category = "Productivity",
        .rating = 4.5,
        .download_count = 1200,
        .is_installed = false
    },
    {
        .id = 2,
        .name = "RaePaint",
        .description = "A basic drawing application.",
        .version = "0.9.0",
        .developer = "RaeenOS Team",
        .size_bytes = 1024 * 700, // 700 KB
        .category = "Graphics",
        .rating = 4.0,
        .download_count = 800,
        .is_installed = false
    }
};

static uint32_t num_available_apps = sizeof(available_apps) / sizeof(app_metadata_t);

void app_store_init(void) {
    debug_print("App Store subsystem initialized (placeholder).\n");
}

app_metadata_t* app_store_list_apps(uint32_t* count) {
    if (count) {
        *count = num_available_apps;
    }
    debug_print("App Store: Listing available apps (simulated).\n");
    return available_apps;
}

app_metadata_t* app_store_get_app_details(uint32_t app_id) {
    for (uint32_t i = 0; i < num_available_apps; i++) {
        if (available_apps[i].id == app_id) {
            debug_print("App Store: Getting app details for ID ");
            vga_put_dec(app_id);
            debug_print(" (simulated).\n");
            return &available_apps[i];
        }
    }
    debug_print("App Store: App with ID ");
    vga_put_dec(app_id);
    debug_print(" not found.\n");
    return NULL;
}

int app_store_install_app(uint32_t app_id) {
    debug_print("App Store: Installing app with ID ");
    vga_put_dec(app_id);
    debug_print(" (simulated).\n");
    // In a real implementation, this would download, verify, and install the app.
    app_metadata_t* app = app_store_get_app_details(app_id);
    if (app) {
        app->is_installed = true;
        app->download_count++;
        return 0; // Success
    }
    return -1; // Failure
}

int app_store_uninstall_app(uint32_t app_id) {
    debug_print("App Store: Uninstalling app with ID ");
    vga_put_dec(app_id);
    debug_print(" (simulated).\n");
    // In a real implementation, this would remove the app files and data.
    app_metadata_t* app = app_store_get_app_details(app_id);
    if (app) {
        app->is_installed = false;
        return 0; // Success
    }
    return -1; // Failure
}
