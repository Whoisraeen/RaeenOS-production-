#ifndef AUTOMATION_SUGGESTIONS_H
#define AUTOMATION_SUGGESTIONS_H

#include <stdint.h>

// Initialize automation suggestions engine
void automation_suggestions_init(void);

// Suggest an automation based on user activity
const char* automation_suggestions_get_suggestion(void);

#endif // AUTOMATION_SUGGESTIONS_H
