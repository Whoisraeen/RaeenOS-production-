#ifndef AI_INTERFACE_H
#define AI_INTERFACE_H

/**
 * @file ai_interface.h
 * @brief Comprehensive AI Integration Interface for RaeenOS
 * 
 * This interface defines the system-wide AI service APIs, NPU access,
 * and AI-powered system features that make RaeenOS an AI-first operating system.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "security_interface.h"
#include "process_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// AI API version
#define AI_API_VERSION 1

// AI system limits
#define AI_MODEL_NAME_MAX       128
#define AI_CONTEXT_ID_MAX       64
#define AI_PROMPT_MAX           32768
#define AI_RESPONSE_MAX         131072
#define AI_MEMORY_KEY_MAX       256
#define AI_BACKEND_NAME_MAX     64
#define MAX_AI_CONTEXTS         1024
#define MAX_AI_MODELS           256
#define MAX_AI_BACKENDS         16
#define MAX_AI_PLUGINS          64

// AI model types
typedef enum {
    AI_MODEL_TYPE_UNKNOWN,
    AI_MODEL_TYPE_LLM,              // Large Language Model
    AI_MODEL_TYPE_VISION,           // Computer Vision
    AI_MODEL_TYPE_AUDIO,            // Audio processing
    AI_MODEL_TYPE_MULTIMODAL,       // Multi-modal (text+image+audio)
    AI_MODEL_TYPE_CODE,             // Code generation/analysis
    AI_MODEL_TYPE_EMBEDDING,        // Text embeddings
    AI_MODEL_TYPE_CLASSIFIER,       // Classification
    AI_MODEL_TYPE_GENERATIVE,       // Generative model
    AI_MODEL_TYPE_REINFORCEMENT,    // Reinforcement learning
    AI_MODEL_TYPE_CUSTOM            // Custom model type
} ai_model_type_t;

// AI backend types
typedef enum {
    AI_BACKEND_LOCAL,               // Local inference
    AI_BACKEND_OPENAI,              // OpenAI API
    AI_BACKEND_ANTHROPIC,           // Anthropic API
    AI_BACKEND_OLLAMA,              // Ollama local server
    AI_BACKEND_HUGGINGFACE,         // HuggingFace inference
    AI_BACKEND_CUSTOM               // Custom backend
} ai_backend_type_t;

// AI hardware acceleration types
typedef enum {
    AI_ACCEL_NONE,                  // CPU only
    AI_ACCEL_GPU,                   // GPU acceleration
    AI_ACCEL_NPU,                   // Neural Processing Unit
    AI_ACCEL_TPU,                   // Tensor Processing Unit
    AI_ACCEL_VPU,                   // Vision Processing Unit
    AI_ACCEL_FPGA,                  // FPGA acceleration
    AI_ACCEL_CUSTOM                 // Custom accelerator
} ai_accel_type_t;

// AI data types
typedef enum {
    AI_DATA_TEXT,                   // Text data
    AI_DATA_IMAGE,                  // Image data
    AI_DATA_AUDIO,                  // Audio data
    AI_DATA_VIDEO,                  // Video data
    AI_DATA_BINARY,                 // Binary data
    AI_DATA_JSON,                   // JSON structured data
    AI_DATA_TENSOR                  // Raw tensor data
} ai_data_type_t;

// AI inference modes
typedef enum {
    AI_MODE_SYNC,                   // Synchronous inference
    AI_MODE_ASYNC,                  // Asynchronous inference
    AI_MODE_STREAMING,              // Streaming inference
    AI_MODE_BATCH                   // Batch inference
} ai_inference_mode_t;

// AI permission flags
typedef enum {
    AI_PERM_INFERENCE     = (1 << 0),  // Basic inference
    AI_PERM_TRAINING      = (1 << 1),  // Model training
    AI_PERM_FINE_TUNING   = (1 << 2),  // Model fine-tuning
    AI_PERM_MODEL_LOADING = (1 << 3),  // Load custom models
    AI_PERM_MEMORY_ACCESS = (1 << 4),  // Access AI memory
    AI_PERM_SYSTEM_INTEGRATION = (1 << 5), // System integration
    AI_PERM_HARDWARE_ACCESS = (1 << 6),    // Hardware acceleration
    AI_PERM_NETWORK_ACCESS = (1 << 7),     // Network inference
    AI_PERM_FILE_ACCESS   = (1 << 8),      // File system access
    AI_PERM_ADMIN         = (1 << 9)       // AI system administration
} ai_permission_t;

// Forward declarations
typedef struct ai_context ai_context_t;
typedef struct ai_model ai_model_t;
typedef struct ai_backend ai_backend_t;
typedef struct ai_request ai_request_t;
typedef struct ai_response ai_response_t;
typedef struct ai_memory ai_memory_t;

// AI data structure
typedef struct ai_data {
    ai_data_type_t type;            // Data type
    size_t size;                    // Data size
    void* data;                     // Data buffer
    
    // Metadata
    char mime_type[64];             // MIME type
    uint32_t width, height;         // Image/video dimensions
    uint32_t channels;              // Audio channels
    uint32_t sample_rate;           // Audio sample rate
    char encoding[32];              // Data encoding
    
    // Memory management
    bool owns_data;                 // Whether we own the data
    void (*free_func)(void*);       // Custom free function
} ai_data_t;

// AI model information
struct ai_model {
    char name[AI_MODEL_NAME_MAX];   // Model name
    char version[32];               // Model version
    ai_model_type_t type;           // Model type
    ai_backend_type_t backend;      // Backend type
    
    // Model specifications
    size_t parameter_count;         // Number of parameters
    size_t context_length;          // Maximum context length
    size_t memory_usage;            // Memory usage in bytes
    ai_accel_type_t acceleration;   // Hardware acceleration
    
    // Capabilities
    struct {
        bool supports_streaming;    // Streaming inference
        bool supports_batching;     // Batch inference
        bool supports_fine_tuning;  // Fine-tuning
        bool supports_training;     // Full training
        bool supports_multimodal;   // Multi-modal input
    } capabilities;
    
    // Performance metrics
    struct {
        float tokens_per_second;    // Inference speed
        float memory_bandwidth;     // Memory bandwidth GB/s
        uint32_t latency_ms;        // Average latency
        float accuracy;             // Model accuracy (0-1)
    } performance;
    
    // Model files
    char model_path[256];           // Path to model files
    char config_path[256];          // Path to config file
    char tokenizer_path[256];       // Path to tokenizer
    
    // Security
    char checksum[64];              // Model checksum
    security_context_t* security;   // Security context
    
    // Reference counting
    uint32_t ref_count;
    
    void* private_data;             // Backend-specific data
};

// AI backend interface
struct ai_backend {
    char name[AI_BACKEND_NAME_MAX]; // Backend name
    ai_backend_type_t type;         // Backend type
    char version[32];               // Backend version
    
    // Backend operations
    struct ai_backend_operations* ops;
    
    // Configuration
    struct {
        char endpoint[256];         // API endpoint (for remote)
        char api_key[128];          // API key (for remote)
        uint32_t timeout_ms;        // Request timeout
        uint32_t retry_count;       // Retry attempts
        bool use_cache;             // Enable caching
    } config;
    
    // Supported models
    ai_model_t** models;            // Available models
    size_t model_count;             // Number of models
    
    // Statistics
    struct {
        uint64_t requests_total;    // Total requests
        uint64_t requests_success;  // Successful requests
        uint64_t requests_failed;   // Failed requests
        uint64_t total_tokens;      // Total tokens processed
        uint64_t total_latency_ms;  // Total latency
    } stats;
    
    void* private_data;
};

// AI context (conversation/session)
struct ai_context {
    char context_id[AI_CONTEXT_ID_MAX]; // Unique context ID
    uint32_t process_id;            // Owner process ID
    ai_model_t* model;              // Associated model
    ai_backend_t* backend;          // Backend instance
    
    // Context configuration
    struct {
        float temperature;          // Sampling temperature (0-2)
        float top_p;                // Top-p sampling (0-1)
        uint32_t max_tokens;        // Maximum response tokens
        uint32_t seed;              // Random seed
        char* stop_sequences[8];    // Stop sequences
        size_t stop_count;          // Number of stop sequences
    } params;
    
    // Conversation history
    ai_memory_t* memory;            // Context memory
    size_t max_history;             // Maximum history length
    size_t current_tokens;          // Current token count
    
    // Permissions and security
    ai_permission_t permissions;    // Allowed operations
    security_context_t* security;   // Security context
    
    // Statistics
    struct {
        uint64_t created_time;      // Creation timestamp
        uint64_t last_used;         // Last usage timestamp  
        uint32_t request_count;     // Number of requests
        uint64_t total_tokens;      // Total tokens used
        uint64_t total_latency_ms;  // Total latency
    } stats;
    
    // Synchronization
    void* lock;                     // Context lock
    
    void* private_data;
};

// AI memory system
struct ai_memory {
    char** keys;                    // Memory keys
    ai_data_t** values;             // Memory values
    size_t count;                   // Number of entries
    size_t capacity;                // Memory capacity
    
    // Memory types
    struct {
        ai_data_t** short_term;     // Short-term memory
        size_t short_term_size;
        ai_data_t** long_term;      // Long-term memory
        size_t long_term_size;
        ai_data_t** episodic;       // Episodic memory
        size_t episodic_size;
    } types;
    
    // Memory management
    size_t max_size;                // Maximum memory size
    size_t current_size;            // Current memory usage
    
    void* private_data;
};

// AI request structure
struct ai_request {
    char request_id[64];            // Unique request ID
    ai_context_t* context;          // AI context
    ai_inference_mode_t mode;       // Inference mode
    
    // Input data
    ai_data_t** inputs;             // Input data array
    size_t input_count;             // Number of inputs
    
    // Request parameters
    struct {
        float temperature;          // Override temperature
        uint32_t max_tokens;        // Override max tokens
        bool stream;                // Stream response
        void* callback;             // Completion callback
        void* callback_data;        // Callback user data
    } params;
    
    // Timing
    uint64_t submitted_time;        // Request submission time
    uint64_t started_time;          // Processing start time
    uint32_t timeout_ms;            // Request timeout
    
    // Status
    enum {
        AI_REQUEST_PENDING,         // Waiting in queue
        AI_REQUEST_PROCESSING,      // Currently processing
        AI_REQUEST_COMPLETED,       // Completed successfully
        AI_REQUEST_FAILED,          // Failed with error
        AI_REQUEST_CANCELLED        // Cancelled by user
    } status;
    
    int error_code;                 // Error code (if failed)
    char error_message[256];        // Error message
};

// AI response structure
struct ai_response {
    char request_id[64];            // Corresponding request ID
    
    // Output data
    ai_data_t** outputs;            // Output data array
    size_t output_count;            // Number of outputs
    
    // Response metadata
    struct {
        uint32_t tokens_used;       // Tokens consumed
        uint32_t latency_ms;        // Response latency
        float confidence;           // Confidence score (0-1)
        bool truncated;             // Response truncated
        char stop_reason[64];       // Why generation stopped
    } metadata;
    
    // Timing information
    uint64_t completed_time;        // Completion timestamp
    uint64_t processing_time;       // Processing duration
    
    // Quality metrics
    struct {
        float coherence;            // Response coherence
        float relevance;            // Response relevance
        float safety;               // Safety score
        bool flagged;               // Content flagged
    } quality;
};

// AI backend operations
typedef struct ai_backend_operations {
    // Initialization
    int (*init)(ai_backend_t* backend);
    void (*cleanup)(ai_backend_t* backend);
    
    // Model management
    int (*load_model)(ai_backend_t* backend, const char* model_name, ai_model_t** model);
    int (*unload_model)(ai_backend_t* backend, ai_model_t* model);
    int (*list_models)(ai_backend_t* backend, ai_model_t*** models, size_t* count);
    
    // Inference
    int (*infer_sync)(ai_backend_t* backend, ai_request_t* request, ai_response_t** response);
    int (*infer_async)(ai_backend_t* backend, ai_request_t* request, void* callback);
    int (*infer_stream)(ai_backend_t* backend, ai_request_t* request, void* stream_callback);
    int (*cancel_request)(ai_backend_t* backend, const char* request_id);
    
    // Context management
    int (*create_context)(ai_backend_t* backend, ai_model_t* model, ai_context_t** context);
    void (*destroy_context)(ai_backend_t* backend, ai_context_t* context);
    int (*reset_context)(ai_backend_t* backend, ai_context_t* context);
    
    // Memory operations
    int (*save_memory)(ai_backend_t* backend, ai_context_t* context, const char* key, ai_data_t* data);
    int (*load_memory)(ai_backend_t* backend, ai_context_t* context, const char* key, ai_data_t** data);
    int (*clear_memory)(ai_backend_t* backend, ai_context_t* context, const char* key);
    
    // Configuration
    int (*set_config)(ai_backend_t* backend, const char* key, const void* value);
    int (*get_config)(ai_backend_t* backend, const char* key, void** value);
    
    // Statistics
    int (*get_stats)(ai_backend_t* backend, void* stats);
} ai_backend_ops_t;

// AI system operations
typedef struct ai_operations {
    // System initialization
    int (*init)(void);
    void (*cleanup)(void);
    
    // Backend management
    int (*register_backend)(ai_backend_t* backend);
    int (*unregister_backend)(const char* name);
    ai_backend_t* (*get_backend)(const char* name);
    int (*list_backends)(ai_backend_t*** backends, size_t* count);
    
    // Model management
    int (*register_model)(ai_model_t* model);
    int (*unregister_model)(const char* name);
    ai_model_t* (*get_model)(const char* name);
    int (*list_models)(ai_model_t*** models, size_t* count);
    int (*load_model_from_file)(const char* path, ai_model_t** model);
    
    // Context management
    int (*create_context)(const char* model_name, ai_context_t** context);
    void (*destroy_context)(ai_context_t* context);
    ai_context_t* (*get_context)(const char* context_id);
    int (*list_contexts)(ai_context_t*** contexts, size_t* count);
    
    // Inference operations
    int (*infer_text)(ai_context_t* context, const char* prompt, char** response);
    int (*infer_image)(ai_context_t* context, ai_data_t* image, const char* prompt, char** response);
    int (*infer_audio)(ai_context_t* context, ai_data_t* audio, char** response);
    int (*infer_multimodal)(ai_context_t* context, ai_data_t** inputs, size_t count, char** response);
    
    // Streaming operations
    int (*start_stream)(ai_context_t* context, const char* prompt, void** stream_handle);
    int (*read_stream)(void* stream_handle, char* buffer, size_t size, size_t* read);
    void (*close_stream)(void* stream_handle);
    
    // Memory operations
    int (*save_memory)(ai_context_t* context, const char* key, const void* data, size_t size);
    int (*load_memory)(ai_context_t* context, const char* key, void** data, size_t* size);
    int (*clear_memory)(ai_context_t* context, const char* key);
    int (*list_memory_keys)(ai_context_t* context, char*** keys, size_t* count);
    
    // Hardware acceleration
    int (*init_npu)(void);
    int (*get_npu_info)(void* info);
    int (*allocate_npu_memory)(size_t size, void** memory);
    int (*free_npu_memory)(void* memory);
    int (*npu_execute)(void* model, void* input, void* output);
    
    // AI-powered system features
    int (*ai_file_organize)(const char* directory, const char* criteria);
    int (*ai_smart_search)(const char* query, const char* scope, char*** results, size_t* count);
    int (*ai_code_completion)(const char* code, const char* language, char** completion);
    int (*ai_command_suggestion)(const char* partial_command, char** suggestion);
    int (*ai_system_optimization)(void);
    int (*ai_security_analysis)(const char* file_path, float* threat_score);
    
    // Configuration and settings
    int (*set_default_model)(const char* model_name);
    const char* (*get_default_model)(void);
    int (*set_global_config)(const char* key, const void* value);
    int (*get_global_config)(const char* key, void** value);
    
    // Permission and security
    int (*check_ai_permission)(process_t* process, ai_permission_t permission);
    int (*grant_ai_permission)(process_t* process, ai_permission_t permission);
    int (*revoke_ai_permission)(process_t* process, ai_permission_t permission);
    
    // Statistics and monitoring
    int (*get_system_stats)(void* stats);
    int (*get_model_stats)(const char* model_name, void* stats);
    int (*get_context_stats)(const char* context_id, void* stats);
    
    // Model training and fine-tuning
    int (*fine_tune_model)(const char* base_model, ai_data_t* training_data, 
                          const char* output_model);
    int (*train_model)(ai_data_t* training_data, const char* model_config,
                      const char* output_model);
    
    // Plugin system
    int (*load_ai_plugin)(const char* plugin_path, void** plugin_handle);
    int (*unload_ai_plugin)(void* plugin_handle);
    int (*call_ai_plugin)(void* plugin_handle, const char* function, 
                         void* args, void** result);
} ai_ops_t;

// Global AI operations
extern ai_ops_t* ai;

// AI system API functions

// Initialization
int ai_init(void);
void ai_cleanup(void);

// Simple AI interface for applications
int ai_query(const char* prompt, char** response);
int ai_query_with_context(const char* context_id, const char* prompt, char** response);
int ai_analyze_image(const char* image_path, const char* question, char** response);
int ai_transcribe_audio(const char* audio_path, char** transcript);

// Context management
int ai_create_context(const char* model_name, char** context_id);
int ai_destroy_context(const char* context_id);
int ai_reset_context(const char* context_id);

// Memory operations
int ai_remember(const char* context_id, const char* key, const char* value);
int ai_recall(const char* context_id, const char* key, char** value);
int ai_forget(const char* context_id, const char* key);

// System integration
int ai_organize_files(const char* directory);
int ai_smart_search(const char* query, char*** results, size_t* count);
int ai_complete_code(const char* code, const char* language, char** completion);
int ai_suggest_command(const char* description, char** command);

// Model management
int ai_load_model(const char* model_path);
int ai_unload_model(const char* model_name);
int ai_list_models(char*** model_names, size_t* count);

// Configuration
int ai_set_default_model(const char* model_name);
const char* ai_get_default_model(void);
int ai_set_temperature(const char* context_id, float temperature);
int ai_set_max_tokens(const char* context_id, uint32_t max_tokens);

// Hardware acceleration
bool ai_has_npu(void);
bool ai_has_gpu_acceleration(void);
int ai_get_acceleration_info(void* info);

// Utility functions
ai_data_t* ai_data_create(ai_data_type_t type, const void* data, size_t size);
void ai_data_destroy(ai_data_t* data);
int ai_data_from_file(const char* file_path, ai_data_t** data);
int ai_data_to_file(ai_data_t* data, const char* file_path);

// Error codes
#define AI_SUCCESS              0
#define AI_ERR_INVALID_PARAM    -3001
#define AI_ERR_NO_MEMORY        -3002
#define AI_ERR_MODEL_NOT_FOUND  -3003
#define AI_ERR_BACKEND_ERROR    -3004
#define AI_ERR_PERMISSION_DENIED -3005
#define AI_ERR_TIMEOUT          -3006
#define AI_ERR_CONTEXT_INVALID  -3007
#define AI_ERR_MODEL_LOAD_FAILED -3008
#define AI_ERR_INFERENCE_FAILED -3009
#define AI_ERR_HARDWARE_ERROR   -3010

// Utility macros
#define AI_PERMISSION_CHECK(perm) \
    ai_check_permission(current_process(), (perm))

#define AI_CONTEXT_VALID(ctx) \
    ((ctx) != NULL && (ctx)->context_id[0] != '\0')

#define AI_MODEL_SUPPORTS(model, feature) \
    ((model)->capabilities.feature)

// Common AI system calls (for syscall interface)
#define SYS_AI_QUERY            300
#define SYS_AI_CREATE_CONTEXT   301
#define SYS_AI_DESTROY_CONTEXT  302
#define SYS_AI_INFER            303
#define SYS_AI_STREAM           304
#define SYS_AI_MEMORY_SAVE      305
#define SYS_AI_MEMORY_LOAD      306

#ifdef __cplusplus
}
#endif

#endif // AI_INTERFACE_H