/**
 * @file [filename].h
 * @brief [Brief description of the module/component]
 * 
 * [Detailed description of what this header file provides, including
 * the main interfaces, data structures, and functionality exposed.
 * Explain the purpose and intended usage of this module.]
 * 
 * @author RaeenOS Development Team
 * @date [YYYY-MM-DD]
 * @version [X.Y.Z]
 * 
 * @copyright Copyright (c) 2025 RaeenOS Project
 * @license MIT License
 * 
 * @defgroup [module_name] [Module Display Name]
 * @brief [Brief module description]
 * 
 * [Detailed module description explaining the overall functionality,
 * relationships with other modules, and important usage patterns.]
 * 
 * @{
 */

#ifndef [MODULE_NAME]_H
#define [MODULE_NAME]_H

/*
 * System includes - Standard C library headers
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Kernel includes - RaeenOS kernel headers
 */
#include "kernel/types.h"
#include "kernel/errno.h"

/*
 * Forward declarations - Incomplete types referenced in this header
 */
struct [forward_declared_struct];
typedef struct [forward_declared_struct] [forward_declared_struct]_t;

/*
 * Constants and Macros
 */

/** @brief Maximum buffer size for [specific use case] */
#define [MODULE]_MAX_BUFFER_SIZE    1024

/** @brief Default timeout value in milliseconds */
#define [MODULE]_DEFAULT_TIMEOUT    5000

/** @brief Version number for API compatibility */
#define [MODULE]_API_VERSION        1

/**
 * @brief Macro to check if a value is within valid range
 * @param val Value to check
 * @param min Minimum valid value (inclusive)
 * @param max Maximum valid value (inclusive)
 * @return true if value is in range, false otherwise
 */
#define [MODULE]_IS_VALID_RANGE(val, min, max) \
    ((val) >= (min) && (val) <= (max))

/*
 * Type Definitions
 */

/**
 * @brief Error codes specific to this module
 */
typedef enum {
    [MODULE]_OK = 0,                    /**< Operation successful */
    [MODULE]_ERROR = -1,                /**< Generic error */
    [MODULE]_INVALID_PARAM = -2,        /**< Invalid parameter */
    [MODULE]_OUT_OF_MEMORY = -3,        /**< Memory allocation failed */
    [MODULE]_TIMEOUT = -4,              /**< Operation timed out */
    [MODULE]_NOT_INITIALIZED = -5,      /**< Module not initialized */
    [MODULE]_ALREADY_EXISTS = -6,       /**< Resource already exists */
    [MODULE]_NOT_FOUND = -7,            /**< Resource not found */
} [module]_error_t;

/**
 * @brief Configuration flags for module behavior
 */
typedef enum {
    [MODULE]_FLAG_NONE = 0x00,          /**< No special flags */
    [MODULE]_FLAG_BLOCKING = 0x01,      /**< Enable blocking operations */
    [MODULE]_FLAG_ASYNC = 0x02,         /**< Enable asynchronous mode */
    [MODULE]_FLAG_DEBUG = 0x04,         /**< Enable debug output */
    [MODULE]_FLAG_SECURE = 0x08,        /**< Enable security features */
} [module]_flags_t;

/**
 * @brief Callback function type for [specific purpose]
 * 
 * @param context User-provided context pointer
 * @param data Pointer to callback data
 * @param size Size of callback data in bytes
 * @return [module]_error_t Error code indicating success or failure
 * 
 * @note The callback must not block for extended periods
 * @warning The data pointer is only valid during the callback
 */
typedef [module]_error_t (*[module]_callback_t)(void* context, 
                                                const void* data, 
                                                size_t size);

/**
 * @brief Main configuration structure for the module
 * 
 * This structure contains all configuration parameters needed to
 * initialize and configure the module behavior.
 */
typedef struct {
    uint32_t version;                   /**< API version (set to [MODULE]_API_VERSION) */
    [module]_flags_t flags;             /**< Configuration flags */
    uint32_t buffer_size;               /**< Buffer size in bytes */
    uint32_t timeout_ms;                /**< Timeout in milliseconds */
    [module]_callback_t callback;       /**< Optional callback function */
    void* callback_context;             /**< Context for callback function */
    
    /* Private fields - do not access directly */
    void* _private;                     /**< Internal use only */
} [module]_config_t;

/**
 * @brief Opaque handle for module instances
 * 
 * This structure represents an instance of the module and should be
 * treated as opaque by users. Access the contents only through the
 * provided API functions.
 */
typedef struct {
    uint32_t magic;                     /**< Magic number for validation */
    [module]_config_t config;           /**< Current configuration */
    bool initialized;                   /**< Initialization state */
    
    /* Implementation-specific fields */
    uint8_t internal_buffer[[MODULE]_MAX_BUFFER_SIZE];
    uint32_t state;                     /**< Current internal state */
    
    /* Statistics and debugging */
    uint64_t operation_count;           /**< Number of operations performed */
    uint64_t error_count;               /**< Number of errors encountered */
} [module]_handle_t;

/*
 * Global Variables (avoid when possible)
 */

/* Example of properly documented global variable if needed */
extern const [module]_config_t [module]_default_config;

/*
 * Function Declarations
 */

/**
 * @brief Initialize the [module name] module
 * 
 * This function performs one-time initialization of the module.
 * It should be called once before using any other module functions.
 * 
 * @return [module]_error_t Error code
 * @retval [MODULE]_OK Initialization successful
 * @retval [MODULE]_ERROR Initialization failed
 * @retval [MODULE]_OUT_OF_MEMORY Insufficient memory
 * 
 * @thread_safety This function is not thread-safe
 * @note Must be called before any other module functions
 * @see [module]_cleanup()
 */
[module]_error_t [module]_init(void);

/**
 * @brief Clean up the [module name] module
 * 
 * This function performs cleanup and releases all resources allocated
 * by the module. After calling this function, [module]_init() must be
 * called again before using the module.
 * 
 * @thread_safety This function is not thread-safe
 * @note All handles must be destroyed before calling this function
 * @see [module]_init()
 */
void [module]_cleanup(void);

/**
 * @brief Create a new module handle with specified configuration
 * 
 * @param config Pointer to configuration structure (cannot be NULL)
 * @param handle Pointer to handle variable to initialize
 * 
 * @return [module]_error_t Error code
 * @retval [MODULE]_OK Handle created successfully
 * @retval [MODULE]_INVALID_PARAM Invalid parameter
 * @retval [MODULE]_OUT_OF_MEMORY Insufficient memory
 * @retval [MODULE]_NOT_INITIALIZED Module not initialized
 * 
 * @pre Module must be initialized with [module]_init()
 * @pre config must not be NULL
 * @pre handle must not be NULL
 * @post On success, handle is valid and ready for use
 * 
 * @thread_safety This function is thread-safe
 * @see [module]_destroy_handle()
 */
[module]_error_t [module]_create_handle(const [module]_config_t* config,
                                        [module]_handle_t** handle);

/**
 * @brief Destroy a module handle and free its resources
 * 
 * @param handle Pointer to handle to destroy (can be NULL)
 * 
 * @pre handle was created with [module]_create_handle()
 * @post handle is no longer valid and cannot be used
 * 
 * @thread_safety This function is thread-safe
 * @note Safe to call with NULL handle
 * @see [module]_create_handle()
 */
void [module]_destroy_handle([module]_handle_t* handle);

/**
 * @brief Perform main operation of the module
 * 
 * [Detailed description of what this operation does, including
 * any side effects, requirements, and expected behavior.]
 * 
 * @param handle Valid module handle
 * @param input Input data buffer
 * @param input_size Size of input data in bytes
 * @param output Output buffer for results
 * @param output_size Size of output buffer in bytes
 * @param bytes_written Pointer to variable receiving bytes written
 * 
 * @return [module]_error_t Error code
 * @retval [MODULE]_OK Operation successful
 * @retval [MODULE]_INVALID_PARAM Invalid parameter
 * @retval [MODULE]_TIMEOUT Operation timed out
 * 
 * @pre handle must be valid and initialized
 * @pre input must not be NULL if input_size > 0
 * @pre output must not be NULL if output_size > 0
 * @pre bytes_written must not be NULL
 * 
 * @thread_safety This function is thread-safe with different handles
 * @note The same handle cannot be used simultaneously from multiple threads
 * 
 * @par Example:
 * @code
 * [module]_handle_t* handle;
 * [module]_error_t result;
 * size_t written;
 * 
 * result = [module]_create_handle(&config, &handle);
 * if (result != [MODULE]_OK) {
 *     return result;
 * }
 * 
 * result = [module]_process(handle, input_data, input_len,
 *                          output_buffer, sizeof(output_buffer), &written);
 * if (result == [MODULE]_OK) {
 *     // Process output_buffer with 'written' bytes
 * }
 * 
 * [module]_destroy_handle(handle);
 * @endcode
 */
[module]_error_t [module]_process([module]_handle_t* handle,
                                 const void* input,
                                 size_t input_size,
                                 void* output,
                                 size_t output_size,
                                 size_t* bytes_written);

/**
 * @brief Get current module statistics
 * 
 * @param handle Valid module handle
 * @param stats Pointer to statistics structure to fill
 * 
 * @return [module]_error_t Error code
 * @retval [MODULE]_OK Statistics retrieved successfully
 * @retval [MODULE]_INVALID_PARAM Invalid parameter
 * 
 * @thread_safety This function is thread-safe
 */
[module]_error_t [module]_get_stats(const [module]_handle_t* handle,
                                   [module]_stats_t* stats);

/*
 * Utility Functions and Macros
 */

/**
 * @brief Check if handle is valid
 * @param handle Handle to validate
 * @return true if handle is valid, false otherwise
 */
static inline bool [module]_is_valid_handle(const [module]_handle_t* handle) {
    return handle != NULL && handle->magic == [MODULE]_MAGIC_NUMBER;
}

/**
 * @brief Get human-readable error message for error code
 * @param error Error code to translate
 * @return String describing the error (never NULL)
 */
const char* [module]_strerror([module]_error_t error);

#endif /* [MODULE_NAME]_H */

/** @} */ /* End of [module_name] group */