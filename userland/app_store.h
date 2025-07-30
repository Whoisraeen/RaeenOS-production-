#ifndef APP_STORE_H
#define APP_STORE_H

#include <stdint.h>

// App metadata structure (simplified)
typedef struct {
    char name[64];
    char description[256];
    char developer[64];
    // Add version, icon, screenshots, etc.
} app_metadata_t;

// Initialize the App Store
void app_store_init(void);

// Search for applications
app_metadata_t* app_store_search(const char* query);

// Install an application
int app_store_install(const char* app_id);

#endif // APP_STORE_H
