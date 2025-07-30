#ifndef VOICE_RECOGNITION_H
#define VOICE_RECOGNITION_H

#include <stdint.h>

// Initialize voice recognition system
void voice_recognition_init(void);

// Start listening for voice commands
void voice_recognition_start_listening(void);

// Stop listening and get recognized text
const char* voice_recognition_stop_listening(void);

#endif // VOICE_RECOGNITION_H
