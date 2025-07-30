#ifndef SYSTEM_INTEGRATION_H
#define SYSTEM_INTEGRATION_H

#include <stdint.h>

// Initialize system integration
void system_integration_init(void);

// Perform a system action (e.g., open application, change setting)
int system_integration_perform_action(const char* action, const char* params);

#endif // SYSTEM_INTEGRATION_H
