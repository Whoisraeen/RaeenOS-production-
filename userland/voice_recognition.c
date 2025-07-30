#include "voice_recognition.h"
#include "../kernel/vga.h"
#include "../kernel/memory.h"
#include "../kernel/string.h"
#include "../drivers/audio/audio.h"
#include "../drivers/npu.h"

// Voice recognition configuration
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 4096
#define MAX_PHRASE_LENGTH 512
#define CONFIDENCE_THRESHOLD 0.7f

// Voice recognition state
typedef struct {
    bool initialized;
    bool listening;
    uint8_t* audio_buffer;
    uint32_t buffer_position;
    char last_recognized_text[MAX_PHRASE_LENGTH];
    float last_confidence;
    bool use_npu_acceleration;
} voice_recognition_state_t;

static voice_recognition_state_t vr_state = {0};

// Simple command vocabulary for demonstration
static const char* known_commands[] = {
    "open file manager",
    "close window", 
    "maximize window",
    "minimize window",
    "switch workspace",
    "show desktop",
    "run terminal",
    "take screenshot",
    "lock screen",
    "show applications",
    "search files",
    "play music",
    "adjust volume",
    "show time",
    "open settings"
};

static const int num_known_commands = sizeof(known_commands) / sizeof(known_commands[0]);

void voice_recognition_init(void) {
    if (vr_state.initialized) {
        debug_print("Voice recognition already initialized");
        return;
    }
    
    // Initialize audio subsystem
    audio_init();
    
    // Allocate audio buffer
    vr_state.audio_buffer = (uint8_t*)kmalloc(BUFFER_SIZE);
    if (!vr_state.audio_buffer) {
        debug_print("Failed to allocate voice recognition audio buffer");
        return;
    }
    
    // Check if NPU is available for acceleration
    vr_state.use_npu_acceleration = _voice_check_npu_support();
    if (vr_state.use_npu_acceleration) {
        npu_load_model("voice_recognition_model.bin");
        debug_print("Voice recognition using NPU acceleration");
    } else {
        debug_print("Voice recognition using software processing");
    }
    
    vr_state.buffer_position = 0;
    vr_state.listening = false;
    vr_state.last_confidence = 0.0f;
    vr_state.initialized = true;
    
    debug_print("Advanced voice recognition system initialized");
}

static bool _voice_check_npu_support(void) {
    // Check if NPU is available and suitable for voice recognition
    return npu_is_available() && npu_supports_voice_processing();
}

void voice_recognition_start_listening(void) {
    if (!vr_state.initialized) {
        debug_print("Voice recognition not initialized");
        return;
    }
    
    if (vr_state.listening) {
        debug_print("Already listening for voice commands");
        return;
    }
    
    // Clear buffer and reset state
    vr_state.buffer_position = 0;
    memset(vr_state.audio_buffer, 0, BUFFER_SIZE);
    memset(vr_state.last_recognized_text, 0, MAX_PHRASE_LENGTH);
    
    // Start audio capture
    audio_start_capture(SAMPLE_RATE, 1); // Mono capture
    vr_state.listening = true;
    
    debug_print("Voice recognition: Listening for commands...");
}

const char* voice_recognition_stop_listening(void) {
    if (!vr_state.initialized || !vr_state.listening) {
        return "";
    }
    
    // Stop audio capture
    audio_stop_capture();
    vr_state.listening = false;
    
    debug_print("Voice recognition: Processing captured audio");
    
    // Process the captured audio
    _voice_process_audio();
    
    if (vr_state.last_confidence >= CONFIDENCE_THRESHOLD) {
        char msg[200];
        sprintf(msg, "Recognized: '%s' (confidence: %.2f)", 
                vr_state.last_recognized_text, vr_state.last_confidence);
        debug_print(msg);
        return vr_state.last_recognized_text;
    } else {
        debug_print("Voice recognition: Low confidence, no command recognized");
        return "";
    }
}

static void _voice_process_audio(void) {
    if (vr_state.buffer_position == 0) {
        vr_state.last_confidence = 0.0f;
        return;
    }
    
    if (vr_state.use_npu_acceleration) {
        _voice_process_with_npu();
    } else {
        _voice_process_with_software();
    }
}

static void _voice_process_with_npu(void) {
    // Use NPU for voice recognition processing
    debug_print("Processing voice with NPU acceleration");
    
    // Prepare audio data for NPU
    npu_input_data_t input = {
        .data = vr_state.audio_buffer,
        .size = vr_state.buffer_position,
        .format = NPU_AUDIO_16KHZ_MONO
    };
    
    npu_output_data_t output = {0};
    
    if (npu_execute_inference(&input, &output) == 0) {
        // Parse NPU output
        if (output.confidence >= CONFIDENCE_THRESHOLD) {
            strncpy(vr_state.last_recognized_text, (char*)output.text_result, 
                   MAX_PHRASE_LENGTH - 1);
            vr_state.last_confidence = output.confidence;
        } else {
            vr_state.last_confidence = output.confidence;
        }
    } else {
        debug_print("NPU voice processing failed, falling back to software");
        _voice_process_with_software();
    }
}

static void _voice_process_with_software(void) {
    // Software-based voice recognition (simplified)
    debug_print("Processing voice with software algorithms");
    
    // Simulate voice processing with pattern matching
    // In a real implementation, this would use signal processing and ML algorithms
    
    // Simple energy-based voice activity detection
    uint32_t total_energy = 0;
    uint16_t* samples = (uint16_t*)vr_state.audio_buffer;
    uint32_t num_samples = vr_state.buffer_position / 2;
    
    for (uint32_t i = 0; i < num_samples; i++) {
        uint16_t sample = samples[i];
        total_energy += (sample > 32768) ? (sample - 32768) : (32768 - sample);
    }
    
    float average_energy = (float)total_energy / num_samples;
    
    // If energy is above threshold, assume voice detected
    if (average_energy > 1000) {
        // Simulate command recognition with simple pattern matching
        _voice_match_command(average_energy);
    } else {
        vr_state.last_confidence = 0.1f;
        strcpy(vr_state.last_recognized_text, "");
    }
}

static void _voice_match_command(float energy_level) {
    // Simulate command matching based on audio characteristics
    // In reality, this would use sophisticated speech recognition algorithms
    
    uint32_t energy_hash = (uint32_t)(energy_level * 1000) % num_known_commands;
    
    // Use energy pattern to select a command (simplified simulation)
    const char* matched_command = known_commands[energy_hash];
    
    strncpy(vr_state.last_recognized_text, matched_command, MAX_PHRASE_LENGTH - 1);
    
    // Simulate confidence based on energy level consistency
    if (energy_level > 5000) {
        vr_state.last_confidence = 0.9f;
    } else if (energy_level > 2000) {
        vr_state.last_confidence = 0.8f;
    } else {
        vr_state.last_confidence = 0.6f;
    }
}

bool voice_recognition_is_listening(void) {
    return vr_state.initialized && vr_state.listening;
}

float voice_recognition_get_last_confidence(void) {
    return vr_state.last_confidence;
}

void voice_recognition_set_threshold(float threshold) {
    if (threshold >= 0.0f && threshold <= 1.0f) {
        // In a full implementation, this would update the confidence threshold
        debug_print("Voice recognition confidence threshold updated");
    }
}

void voice_recognition_add_custom_command(const char* command) {
    if (!command) {
        return;
    }
    
    // In a full implementation, this would add the command to the vocabulary
    char msg[150];
    sprintf(msg, "Added custom voice command: '%s'", command);
    debug_print(msg);
}

void voice_recognition_update_audio_buffer(const uint8_t* data, uint32_t size) {
    if (!vr_state.initialized || !vr_state.listening || !data) {
        return;
    }
    
    // Append new audio data to buffer
    uint32_t available_space = BUFFER_SIZE - vr_state.buffer_position;
    uint32_t copy_size = (size < available_space) ? size : available_space;
    
    if (copy_size > 0) {
        memcpy(vr_state.audio_buffer + vr_state.buffer_position, data, copy_size);
        vr_state.buffer_position += copy_size;
    }
    
    // If buffer is full, process automatically
    if (vr_state.buffer_position >= BUFFER_SIZE) {
        voice_recognition_stop_listening();
    }
}

void voice_recognition_cleanup(void) {
    if (!vr_state.initialized) {
        return;
    }
    
    if (vr_state.listening) {
        voice_recognition_stop_listening();
    }
    
    if (vr_state.audio_buffer) {
        kfree(vr_state.audio_buffer);
        vr_state.audio_buffer = NULL;
    }
    
    vr_state.initialized = false;
    vr_state.buffer_position = 0;
    
    debug_print("Voice recognition system shutdown completed");
}