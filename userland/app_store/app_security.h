#ifndef APP_SECURITY_H
#define APP_SECURITY_H

#include <stdint.h>
#include <stdbool.h>

// Initialize app security module
void app_security_init(void);

// Verify application package signature
bool app_security_verify_signature(const char* package_path);

// Check if an application is trusted
bool app_security_is_trusted(uint32_t app_id);

#endif // APP_SECURITY_H
