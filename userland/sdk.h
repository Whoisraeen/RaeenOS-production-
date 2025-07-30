#ifndef SDK_H
#define SDK_H

#include <stdint.h>

// Initialize the RaeenOS SDK
void raeenos_sdk_init(void);

// Get system information
int raeenos_sdk_get_system_info(void* info_buffer, uint32_t buffer_size);

// Register a new application
int raeenos_sdk_register_app(const char* app_manifest_path);

#endif // SDK_H
