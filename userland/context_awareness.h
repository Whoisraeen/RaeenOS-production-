#ifndef CONTEXT_AWARENESS_H
#define CONTEXT_AWARENESS_H

#include <stdint.h>

// Initialize context awareness system
void context_awareness_init(void);

// Update context (e.g., current application, user activity, time of day)
void context_awareness_update(const char* key, const char* value);

// Get context value
const char* context_awareness_get(const char* key);

#endif // CONTEXT_AWARENESS_H
