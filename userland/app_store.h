#ifndef APP_STORE_H
#define APP_STORE_H

#include <stdint.h>
#include <stdbool.h>

// Application metadata structure
typedef struct {
    uint32_t id;
    char name[128];
    char description[512];
    char version[32];
    char developer[128];
    uint64_t size_bytes;
    char category[64];
    float rating;
    uint32_t download_count;
    bool is_installed;
} app_metadata_t;

// Initialize the App Store subsystem
void app_store_init(void);

// List available applications
app_metadata_t* app_store_list_apps(uint32_t* count);

// Get details for a specific application
app_metadata_t* app_store_get_app_details(uint32_t app_id);

// Install an application (placeholder)
int app_store_install_app(uint32_t app_id);

// Uninstall an application (placeholder)
int app_store_uninstall_app(uint32_t app_id);

#endif // APP_STORE_H