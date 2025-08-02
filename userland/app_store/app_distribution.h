#ifndef APP_DISTRIBUTION_H
#define APP_DISTRIBUTION_H

#include <stdint.h>
#include <stdbool.h>

// Initialize app distribution system
void app_distribution_init(void);

// Download an application package
int app_distribution_download_package(uint32_t app_id, const char* destination_path);

// Verify application package integrity and signature
bool app_distribution_verify_package(const char* package_path);

// Check for application updates
bool app_distribution_check_for_updates(uint33_t app_id, char* new_version_str);

#endif // APP_DISTRIBUTION_H
