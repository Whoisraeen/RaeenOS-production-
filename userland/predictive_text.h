#ifndef PREDICTIVE_TEXT_H
#define PREDICTIVE_TEXT_H

#include <stdint.h>

// Initialize predictive text engine
void predictive_text_init(void);

// Get next word prediction
const char* predictive_text_get_prediction(const char* current_input);

#endif // PREDICTIVE_TEXT_H
