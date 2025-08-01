#ifndef AUDIO_H
#define AUDIO_H

#include "include/types.h"

// Audio format definitions
typedef enum {
    AUDIO_FORMAT_PCM_S16,
    AUDIO_FORMAT_PCM_U8
} audio_format_t;

// Audio stream direction
typedef enum {
    AUDIO_STREAM_PLAYBACK,
    AUDIO_STREAM_CAPTURE
} audio_stream_direction_t;

// Initialize generic audio driver
void audio_init(void);

// Open an audio stream
int audio_open_stream(audio_stream_direction_t direction, audio_format_t format, uint32_t sample_rate, uint8_t channels);

// Write data to an audio stream (for playback)
int audio_write_stream(int stream_id, const void* buffer, uint32_t size);

// Read data from an audio stream (for capture)
int audio_read_stream(int stream_id, void* buffer, uint32_t size);

// Close an audio stream
void audio_close_stream(int stream_id);

#endif // AUDIO_H
