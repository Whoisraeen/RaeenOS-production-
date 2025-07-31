# RaeenOS Module Development Template
## Template for Creating New Modules in the 42-Agent Architecture

**Document Version:** 1.0  
**Last Updated:** July 31, 2025  
**Target Audience:** All RaeenOS development agents

---

## Module Template Structure

### Directory Structure Template

```
module_name/
├── Makefile                      # Module build configuration
├── README.md                     # Module documentation
├── AGENT_OWNERS.md              # Agent ownership and responsibilities
├── include/                      # Public headers
│   └── module_name.h            # Main module header
├── src/                         # Source files
│   ├── module_name.c            # Main implementation
│   ├── internal.c               # Internal implementations
│   └── platform/                # Platform-specific code
│       ├── x86_64/              # x86-64 specific code
│       └── arm64/               # ARM64 specific code
├── tests/                       # Module tests
│   ├── unit/                    # Unit tests
│   ├── integration/             # Integration tests
│   └── performance/             # Performance tests
├── docs/                        # Module-specific documentation
│   ├── api.md                   # API documentation
│   ├── design.md                # Design documentation
│   └── examples/                # Usage examples
└── scripts/                     # Module-specific scripts
    ├── build.sh                 # Build script
    └── test.sh                  # Test script
```

### Makefile Template

```makefile
# [Module Name] Makefile
# Owner: [primary-agent], [secondary-agent]
# Description: [Brief description of module functionality]

include $(TOPDIR)/Makefile.config
include $(TOPDIR)/Makefile.rules

# Module configuration
MODULE_NAME = [module_name]
MODULE_VERSION = 1.0.0
MODULE_DESCRIPTION = "[Module description]"

# Source files
MODULE_SOURCES = $(wildcard src/*.c)
MODULE_HEADERS = $(wildcard include/*.h)
MODULE_PLATFORM_SOURCES = $(wildcard src/platform/$(ARCH)/*.c)

# Combined sources
ALL_SOURCES = $(MODULE_SOURCES) $(MODULE_PLATFORM_SOURCES)

# Include paths
MODULE_INCLUDES = \
    -I./include \
    -I$(TOPDIR)/core/kernel/include \
    -I$(TOPDIR)/core/hal/include

# Compiler flags specific to this module
MODULE_CFLAGS = -DMODULE_NAME=\"$(MODULE_NAME)\" \
                -DMODULE_VERSION=\"$(MODULE_VERSION)\"

# Libraries this module depends on
MODULE_LIBS = 

# Build rules using common build function
$(eval $(call build-module,$(MODULE_NAME),$(ALL_SOURCES),$(MODULE_INCLUDES),$(MODULE_LIBS)))

# Additional module-specific targets
.PHONY: all clean test docs install debug

all: $(LIB_DIR)/lib$(MODULE_NAME).a

# Testing
test:
	@echo "Running $(MODULE_NAME) tests..."
	$(MAKE) -C tests/unit
	$(MAKE) -C tests/integration

test-unit:
	$(MAKE) -C tests/unit

test-integration:
	$(MAKE) -C tests/integration

test-performance:
	$(MAKE) -C tests/performance

# Documentation
docs:
	@echo "Building $(MODULE_NAME) documentation..."
	doxygen docs/Doxyfile

# Installation
install: all
	$(call install-lib-$(MODULE_NAME))
	$(call install-header-$(MODULE_NAME))

# Debugging
debug:
	@echo "Module: $(MODULE_NAME)"
	@echo "Version: $(MODULE_VERSION)"
	@echo "Sources: $(ALL_SOURCES)"
	@echo "Headers: $(MODULE_HEADERS)"
	@echo "Includes: $(MODULE_INCLUDES)"
	@echo "Libraries: $(MODULE_LIBS)"

# Static analysis
analyze:
	@echo "Running static analysis on $(MODULE_NAME)..."
	$(CLANG_TIDY) $(ALL_SOURCES) $(CFLAGS) $(MODULE_INCLUDES) --

# Code formatting
format:
	@echo "Formatting $(MODULE_NAME) code..."
	$(CLANG_FORMAT) -i $(ALL_SOURCES) $(MODULE_HEADERS)

# Security scanning
security-scan:
	@echo "Running security scan on $(MODULE_NAME)..."
	$(FLAWFINDER) $(ALL_SOURCES) $(MODULE_HEADERS)

# Performance profiling
profile: all
	@echo "Building $(MODULE_NAME) with profiling..."
	$(MAKE) CFLAGS_EXTRA="-pg -fprofile-arcs -ftest-coverage" all

# Clean
clean: clean-$(MODULE_NAME)
	$(MAKE) -C tests clean
	$(MAKE) -C docs clean

# Module information
info:
	@echo "================================"
	@echo "Module: $(MODULE_NAME)"
	@echo "Version: $(MODULE_VERSION)"
	@echo "Description: $(MODULE_DESCRIPTION)"
	@echo "Architecture: $(ARCH)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Primary Agent: [primary-agent]"
	@echo "Secondary Agent: [secondary-agent]"
	@echo "================================"

# Help
help:
	@echo "$(MODULE_NAME) Module Build System"
	@echo "Available targets:"
	@echo "  all              - Build module (default)"
	@echo "  test             - Run all tests"
	@echo "  test-unit        - Run unit tests only"
	@echo "  test-integration - Run integration tests only"
	@echo "  test-performance - Run performance tests"
	@echo "  docs             - Build documentation"
	@echo "  install          - Install module"
	@echo "  clean            - Clean build artifacts"
	@echo "  debug            - Show build configuration"
	@echo "  analyze          - Run static analysis"
	@echo "  format           - Format source code"
	@echo "  security-scan    - Run security analysis"
	@echo "  profile          - Build with profiling"
	@echo "  info             - Show module information"
	@echo "  help             - Show this help"
```

### README.md Template

```markdown
# [Module Name]

Brief description of what this module does and its role in RaeenOS.

## Overview

[Detailed description of the module's purpose, functionality, and how it fits into the overall RaeenOS architecture]

## Agent Ownership

- **Primary Agent**: [primary-agent-name]
- **Secondary Agent**: [secondary-agent-name]
- **Review Agents**: [list-of-review-agents]

## Architecture

[Description of the module's internal architecture, key components, and design decisions]

### Key Components

- **Component 1**: Description
- **Component 2**: Description
- **Component 3**: Description

### Dependencies

- **Required**: List of modules this depends on
- **Optional**: List of optional dependencies
- **Provides**: List of interfaces this module provides

## API Reference

### Public Functions

```c
/**
 * @brief Brief description of function
 * @param param1 Description of parameter
 * @return Description of return value
 */
int module_function_name(int param1);
```

### Data Structures

```c
/**
 * @brief Brief description of structure
 */
typedef struct {
    int field1;    /**< Description of field1 */
    char* field2;  /**< Description of field2 */
} module_struct_t;
```

### Constants and Macros

```c
#define MODULE_CONSTANT_NAME 42  /**< Description of constant */
```

## Usage Examples

### Basic Usage

```c
#include "module_name.h"

int main() {
    // Example usage
    module_init();
    
    // Use module functionality
    int result = module_function_name(42);
    
    // Cleanup
    module_cleanup();
    
    return 0;
}
```

### Advanced Usage

[More complex usage examples]

## Configuration

### Build Configuration

[Description of build-time configuration options]

### Runtime Configuration

[Description of runtime configuration options]

## Testing

### Running Tests

```bash
# Run all tests
make test

# Run specific test suites
make test-unit
make test-integration
make test-performance
```

### Test Coverage

Current test coverage: XX%

### Performance Benchmarks

[Performance benchmarks and targets]

## Development

### Building

```bash
# Build the module
make

# Build with debugging
make BUILD_TYPE=debug

# Build with profiling
make profile
```

### Code Quality

```bash
# Format code
make format

# Run static analysis
make analyze

# Run security scan
make security-scan
```

### Contributing

1. Follow the RaeenOS coding standards
2. Ensure all tests pass
3. Update documentation
4. Get review from designated agents

## Troubleshooting

### Common Issues

- **Issue 1**: Description and solution
- **Issue 2**: Description and solution

### Debugging

[Debugging tips and techniques specific to this module]

## Performance Considerations

[Performance characteristics, optimization notes, and benchmarking information]

## Security Considerations

[Security implications, threat model, and security best practices]

## Changelog

### Version 1.0.0 (Initial Release)
- Initial implementation
- Core functionality
- Basic test suite

## References

- [Related RaeenOS documentation]
- [External references]
- [Standards and specifications]
```

### AGENT_OWNERS.md Template

```markdown
# [Module Name] Agent Ownership

## Primary Ownership

**Primary Agent**: [primary-agent-name]
- **Responsibilities**: 
  - Module architecture and design
  - Core implementation
  - Performance optimization
  - Integration with dependent modules
- **Contact**: [agent-contact-info]
- **Availability**: [time-zones/hours]

## Secondary Ownership

**Secondary Agent**: [secondary-agent-name]
- **Responsibilities**:
  - Code review and quality assurance
  - Testing and validation
  - Documentation maintenance
  - Bug fixes and maintenance
- **Contact**: [agent-contact-info]
- **Availability**: [time-zones/hours]

## Review Requirements

### Code Review

All changes to this module must be reviewed by:
1. **Primary Agent**: [primary-agent-name] (mandatory)
2. **Secondary Agent**: [secondary-agent-name] (recommended)
3. **Security Review**: Required for security-sensitive changes
4. **Performance Review**: Required for performance-critical changes

### Approval Process

1. **Regular Changes**: Primary agent approval required
2. **Major Changes**: Both primary and secondary agent approval
3. **Architecture Changes**: Lead-os-developer approval required
4. **Security Changes**: Privacy-security-engineer approval required

## Coordination Requirements

### Dependencies

This module coordinates with the following agents for dependencies:
- **[Dependency 1]**: [responsible-agent] - [coordination-notes]
- **[Dependency 2]**: [responsible-agent] - [coordination-notes]

### Integration Points

- **[Integration Point 1]**: [description-and-responsible-agents]
- **[Integration Point 2]**: [description-and-responsible-agents]

## Communication Protocols

### Daily Standups
- **Frequency**: Daily at [time]
- **Participants**: Primary and secondary agents
- **Format**: [standup-format]

### Weekly Reviews
- **Frequency**: Weekly on [day] at [time]
- **Participants**: All stakeholders
- **Agenda**: Progress review, issue resolution, planning

### Issue Escalation
1. **Level 1**: Direct communication between agents (2 hours)
2. **Level 2**: Module lead intervention (4 hours)
3. **Level 3**: Architecture board escalation (24 hours)

## Quality Standards

### Code Quality Requirements
- **Test Coverage**: Minimum 90%
- **Code Review**: 100% of changes
- **Documentation**: Complete API documentation
- **Performance**: Meet specified benchmarks

### Testing Requirements
- **Unit Tests**: Required for all functions
- **Integration Tests**: Required for all interfaces
- **Performance Tests**: Required for critical paths
- **Security Tests**: Required for security-sensitive code

## Development Workflow

### Branch Naming
- **Feature branches**: `feature/[agent-name]/[module-name]/[feature-description]`
- **Bug fixes**: `bugfix/[agent-name]/[module-name]/[bug-description]`
- **Hotfixes**: `hotfix/[agent-name]/[module-name]/[issue-number]`

### Commit Message Format
```
[module-name]: [type]: [brief-description]

[Detailed description of changes]

Agent: [agent-name]
Reviewed-by: [reviewer-agent]
Fixes: #[issue-number] (if applicable)
```

### Pull Request Process
1. Create feature branch
2. Implement changes
3. Run local tests
4. Create pull request
5. Code review
6. Address feedback
7. Merge after approval

## Emergency Procedures

### Critical Issues
- **Response Time**: Within 1 hour
- **Escalation**: Direct to lead-os-developer
- **Communication**: All stakeholders notified

### Security Vulnerabilities
- **Response Time**: Within 30 minutes
- **Escalation**: Direct to privacy-security-engineer
- **Process**: Follow security incident response plan

### Performance Regressions
- **Response Time**: Within 2 hours
- **Escalation**: Performance-optimization-analyst
- **Rollback**: Immediate if severe impact
```

### Header File Template

```c
/**
 * @file [module_name].h
 * @brief [Brief description of module functionality]
 * 
 * This file defines the public interface for the [module name] module.
 * [Detailed description of what the module does and how it fits into RaeenOS]
 * 
 * @author RaeenOS Development Team
 * @date [Current Date]
 * @version 1.0.0
 * 
 * @agent_primary [primary-agent-name]
 * @agent_secondary [secondary-agent-name]
 * 
 * @copyright Copyright (c) 2025 RaeenOS Project
 * @license MIT License
 */

#ifndef [MODULE_NAME_UPPER]_H
#define [MODULE_NAME_UPPER]_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Forward declarations
struct [module_name]_context;
struct [module_name]_config;

/**
 * @defgroup [module_name] [Module Display Name]
 * @brief [Brief description of module group]
 * 
 * [Detailed description of the module group and its functionality]
 * 
 * @{
 */

//==============================================================================
// CONSTANTS AND MACROS
//==============================================================================

/** @brief Module version information */
#define [MODULE_NAME_UPPER]_VERSION_MAJOR 1
#define [MODULE_NAME_UPPER]_VERSION_MINOR 0
#define [MODULE_NAME_UPPER]_VERSION_PATCH 0

/** @brief Maximum [relevant limit] */
#define [MODULE_NAME_UPPER]_MAX_[LIMIT] 256

/** @brief Default [relevant value] */
#define [MODULE_NAME_UPPER]_DEFAULT_[VALUE] 42

/**
 * @brief Utility macro for [specific purpose]
 * @param x Input parameter
 * @return Processed value
 */
#define [MODULE_NAME_UPPER]_MACRO(x) ((x) * 2 + 1)

//==============================================================================
// TYPE DEFINITIONS
//==============================================================================

/**
 * @brief Error codes for [module name] operations
 * 
 * These error codes are returned by [module name] functions to indicate
 * the success or failure of operations.
 */
typedef enum {
    [MODULE_NAME_UPPER]_SUCCESS = 0,      /**< Operation completed successfully */
    [MODULE_NAME_UPPER]_ERROR = -1,       /**< Generic error occurred */
    [MODULE_NAME_UPPER]_INVALID_PARAM = -2, /**< Invalid parameter provided */
    [MODULE_NAME_UPPER]_OUT_OF_MEMORY = -3, /**< Memory allocation failed */
    [MODULE_NAME_UPPER]_NOT_FOUND = -4,   /**< Requested item not found */
    [MODULE_NAME_UPPER]_PERMISSION_DENIED = -5, /**< Access denied */
} [module_name]_error_t;

/**
 * @brief Configuration flags for [module name]
 * 
 * These flags control the behavior of the [module name] module.
 */
typedef enum {
    [MODULE_NAME_UPPER]_FLAG_NONE = 0,      /**< No special flags */
    [MODULE_NAME_UPPER]_FLAG_VERBOSE = 1,   /**< Enable verbose output */
    [MODULE_NAME_UPPER]_FLAG_DEBUG = 2,     /**< Enable debug mode */
    [MODULE_NAME_UPPER]_FLAG_ASYNC = 4,     /**< Enable asynchronous operation */
} [module_name]_flags_t;

/**
 * @brief [Module name] configuration structure
 * 
 * This structure contains configuration parameters for initializing
 * the [module name] module.
 */
typedef struct [module_name]_config {
    uint32_t version;           /**< Configuration version */
    [module_name]_flags_t flags; /**< Configuration flags */
    uint32_t buffer_size;       /**< Buffer size in bytes */
    uint32_t timeout_ms;        /**< Timeout in milliseconds */
    void* user_data;            /**< User-provided data pointer */
} [module_name]_config_t;

/**
 * @brief [Module name] context structure
 * 
 * This opaque structure represents the runtime context for a [module name]
 * instance. It should only be accessed through the provided API functions.
 */
typedef struct [module_name]_context [module_name]_context_t;

/**
 * @brief Callback function type for [specific purpose]
 * 
 * @param context The [module name] context
 * @param data User-provided data
 * @param size Size of the data in bytes
 * @param user_data User data provided during registration
 * @return [MODULE_NAME_UPPER]_SUCCESS on success, error code on failure
 */
typedef [module_name]_error_t (*[module_name]_callback_t)(
    [module_name]_context_t* context,
    const void* data,
    size_t size,
    void* user_data
);

//==============================================================================
// FUNCTION DECLARATIONS
//==============================================================================

/**
 * @brief Initialize the [module name] module
 * 
 * This function initializes the [module name] module with the provided
 * configuration. It must be called before any other [module name] functions.
 * 
 * @param config Configuration parameters (can be NULL for defaults)
 * @return [MODULE_NAME_UPPER]_SUCCESS on success, error code on failure
 * 
 * @note This function is thread-safe
 * @warning Must be called before any other module functions
 * 
 * @example
 * @code
 * [module_name]_config_t config = {
 *     .version = [MODULE_NAME_UPPER]_VERSION_MAJOR,
 *     .flags = [MODULE_NAME_UPPER]_FLAG_VERBOSE,
 *     .buffer_size = 1024,
 *     .timeout_ms = 5000,
 *     .user_data = NULL
 * };
 * 
 * [module_name]_error_t result = [module_name]_init(&config);
 * if (result != [MODULE_NAME_UPPER]_SUCCESS) {
 *     // Handle initialization error
 * }
 * @endcode
 */
[module_name]_error_t [module_name]_init(const [module_name]_config_t* config);

/**
 * @brief Create a new [module name] context
 * 
 * Creates a new context instance for [module name] operations. Each context
 * maintains its own state and can be used independently.
 * 
 * @param context Pointer to store the created context
 * @param config Configuration for the context (can be NULL for defaults)
 * @return [MODULE_NAME_UPPER]_SUCCESS on success, error code on failure
 * 
 * @note The caller must call [module_name]_destroy_context() to free resources
 * @see [module_name]_destroy_context()
 */
[module_name]_error_t [module_name]_create_context(
    [module_name]_context_t** context,
    const [module_name]_config_t* config
);

/**
 * @brief Destroy a [module name] context
 * 
 * Destroys a context created by [module_name]_create_context() and frees
 * all associated resources.
 * 
 * @param context Context to destroy (can be NULL)
 * 
 * @note After calling this function, the context pointer becomes invalid
 * @see [module_name]_create_context()
 */
void [module_name]_destroy_context([module_name]_context_t* context);

/**
 * @brief Perform [specific operation] on the context
 * 
 * [Detailed description of what this function does, its parameters,
 * return values, side effects, and any important usage notes]
 * 
 * @param context The [module name] context
 * @param input Input data for the operation
 * @param input_size Size of input data in bytes
 * @param output Buffer to store output data
 * @param output_size Size of output buffer in bytes
 * @param bytes_written Pointer to store number of bytes written (can be NULL)
 * @return [MODULE_NAME_UPPER]_SUCCESS on success, error code on failure
 * 
 * @pre context must be a valid context created by [module_name]_create_context()
 * @pre input must not be NULL if input_size > 0
 * @pre output must not be NULL if output_size > 0
 * @post On success, output contains the processed data
 * 
 * @thread_safety This function is thread-safe
 * @performance O(n) where n is input_size
 */
[module_name]_error_t [module_name]_process_data(
    [module_name]_context_t* context,
    const void* input,
    size_t input_size,
    void* output,
    size_t output_size,
    size_t* bytes_written
);

/**
 * @brief Register a callback for [specific events]
 * 
 * Registers a callback function that will be called when [specific events]
 * occur in the [module name] context.
 * 
 * @param context The [module name] context
 * @param callback Callback function to register
 * @param user_data User data to pass to the callback
 * @return [MODULE_NAME_UPPER]_SUCCESS on success, error code on failure
 * 
 * @note Only one callback can be registered per context
 * @see [module_name]_callback_t
 */
[module_name]_error_t [module_name]_register_callback(
    [module_name]_context_t* context,
    [module_name]_callback_t callback,
    void* user_data
);

/**
 * @brief Get [module name] version information
 * 
 * Returns version information for the [module name] module.
 * 
 * @param major Pointer to store major version number
 * @param minor Pointer to store minor version number
 * @param patch Pointer to store patch version number
 * 
 * @note All parameters can be NULL if not needed
 */
void [module_name]_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch);

/**
 * @brief Convert error code to human-readable string
 * 
 * Converts a [module name] error code to a descriptive string.
 * 
 * @param error Error code to convert
 * @return Pointer to static string describing the error
 * 
 * @note The returned string is statically allocated and should not be freed
 */
const char* [module_name]_error_string([module_name]_error_t error);

/**
 * @brief Cleanup and shutdown the [module name] module
 * 
 * Performs cleanup and shuts down the [module name] module. This should be
 * called during system shutdown or when the module is no longer needed.
 * 
 * @note After calling this function, [module_name]_init() must be called
 *       again before using any other module functions
 * @warning All contexts must be destroyed before calling this function
 */
void [module_name]_shutdown(void);

//==============================================================================
// INLINE FUNCTIONS
//==============================================================================

/**
 * @brief Check if an error code indicates success
 * 
 * @param error Error code to check
 * @return true if the error code indicates success, false otherwise
 */
static inline bool [module_name]_is_success([module_name]_error_t error) {
    return error == [MODULE_NAME_UPPER]_SUCCESS;
}

/**
 * @brief Check if an error code indicates failure
 * 
 * @param error Error code to check
 * @return true if the error code indicates failure, false otherwise
 */
static inline bool [module_name]_is_error([module_name]_error_t error) {
    return error != [MODULE_NAME_UPPER]_SUCCESS;
}

/** @} */ // End of [module_name] group

#endif // [MODULE_NAME_UPPER]_H
```

### Source File Template

```c
/**
 * @file [module_name].c
 * @brief [Brief description of module implementation]
 * 
 * [Detailed description of the implementation, algorithms used,
 * and any important implementation notes]
 * 
 * @author RaeenOS Development Team
 * @date [Current Date]
 * @version 1.0.0
 * 
 * @agent_primary [primary-agent-name]
 * @agent_secondary [secondary-agent-name]
 * 
 * @copyright Copyright (c) 2025 RaeenOS Project
 * @license MIT License
 */

#include "[module_name].h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Internal headers
#include "internal.h"

//==============================================================================
// INTERNAL CONSTANTS AND MACROS
//==============================================================================

/** @brief Internal buffer size */
#define INTERNAL_BUFFER_SIZE 4096

/** @brief Magic number for context validation */
#define CONTEXT_MAGIC 0x[MODULE_HEX]

/** @brief Debug logging macro */
#ifdef DEBUG
    #define DEBUG_LOG(fmt, ...) \
        printf("[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...) do { } while (0)
#endif

//==============================================================================
// INTERNAL TYPE DEFINITIONS
//==============================================================================

/**
 * @brief Internal context structure
 * 
 * This structure contains the internal state for a [module name] context.
 */
struct [module_name]_context {
    uint32_t magic;                 /**< Magic number for validation */
    [module_name]_config_t config;  /**< Configuration copy */
    [module_name]_callback_t callback; /**< Registered callback */
    void* callback_user_data;       /**< User data for callback */
    void* internal_buffer;          /**< Internal working buffer */
    size_t buffer_size;             /**< Size of internal buffer */
    uint32_t reference_count;       /**< Reference count */
    bool initialized;               /**< Initialization flag */
};

//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

/** @brief Global module initialization flag */
static bool g_module_initialized = false;

/** @brief Global module configuration */
static [module_name]_config_t g_module_config;

/** @brief Module reference count */
static uint32_t g_module_refcount = 0;

//==============================================================================
// INTERNAL FUNCTION DECLARATIONS
//==============================================================================

/**
 * @brief Validate context pointer
 * @param context Context to validate
 * @return true if valid, false otherwise
 */
static bool is_valid_context(const [module_name]_context_t* context);

/**
 * @brief Initialize default configuration
 * @param config Configuration structure to initialize
 */
static void init_default_config([module_name]_config_t* config);

/**
 * @brief Internal cleanup function
 * @param context Context to clean up
 */
static void internal_cleanup([module_name]_context_t* context);

//==============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
//==============================================================================

[module_name]_error_t [module_name]_init(const [module_name]_config_t* config) {
    DEBUG_LOG("Initializing [module name] module");
    
    // Check if already initialized
    if (g_module_initialized) {
        DEBUG_LOG("Module already initialized, incrementing reference count");
        g_module_refcount++;
        return [MODULE_NAME_UPPER]_SUCCESS;
    }
    
    // Initialize default configuration if none provided
    if (config) {
        g_module_config = *config;
    } else {
        init_default_config(&g_module_config);
    }
    
    // Validate configuration
    if (g_module_config.version == 0) {
        DEBUG_LOG("Invalid configuration version");
        return [MODULE_NAME_UPPER]_INVALID_PARAM;
    }
    
    // Perform module-specific initialization
    // [Add module-specific initialization code here]
    
    g_module_initialized = true;
    g_module_refcount = 1;
    
    DEBUG_LOG("[Module name] module initialized successfully");
    return [MODULE_NAME_UPPER]_SUCCESS;
}

[module_name]_error_t [module_name]_create_context(
    [module_name]_context_t** context,
    const [module_name]_config_t* config) {
    
    if (!context) {
        return [MODULE_NAME_UPPER]_INVALID_PARAM;
    }
    
    if (!g_module_initialized) {
        DEBUG_LOG("Module not initialized");
        return [MODULE_NAME_UPPER]_ERROR;
    }
    
    DEBUG_LOG("Creating new [module name] context");
    
    // Allocate context structure
    [module_name]_context_t* ctx = calloc(1, sizeof([module_name]_context_t));
    if (!ctx) {
        DEBUG_LOG("Failed to allocate context memory");
        return [MODULE_NAME_UPPER]_OUT_OF_MEMORY;
    }
    
    // Initialize context
    ctx->magic = CONTEXT_MAGIC;
    ctx->reference_count = 1;
    ctx->initialized = false;
    
    // Copy configuration
    if (config) {
        ctx->config = *config;
    } else {
        ctx->config = g_module_config;
    }
    
    // Allocate internal buffer
    ctx->buffer_size = ctx->config.buffer_size > 0 ? 
                       ctx->config.buffer_size : INTERNAL_BUFFER_SIZE;
    ctx->internal_buffer = malloc(ctx->buffer_size);
    if (!ctx->internal_buffer) {
        DEBUG_LOG("Failed to allocate internal buffer");
        free(ctx);
        return [MODULE_NAME_UPPER]_OUT_OF_MEMORY;
    }
    
    // Perform context-specific initialization
    // [Add context-specific initialization code here]
    
    ctx->initialized = true;
    *context = ctx;
    
    DEBUG_LOG("Context created successfully");
    return [MODULE_NAME_UPPER]_SUCCESS;
}

void [module_name]_destroy_context([module_name]_context_t* context) {
    if (!is_valid_context(context)) {
        DEBUG_LOG("Invalid context pointer");
        return;
    }
    
    DEBUG_LOG("Destroying [module name] context");
    
    // Decrement reference count
    context->reference_count--;
    if (context->reference_count > 0) {
        DEBUG_LOG("Context still has references, not destroying");
        return;
    }
    
    // Perform cleanup
    internal_cleanup(context);
    
    // Free internal buffer
    if (context->internal_buffer) {
        free(context->internal_buffer);
        context->internal_buffer = NULL;
    }
    
    // Invalidate magic number
    context->magic = 0;
    
    // Free context structure
    free(context);
    
    DEBUG_LOG("Context destroyed successfully");
}

[module_name]_error_t [module_name]_process_data(
    [module_name]_context_t* context,
    const void* input,
    size_t input_size,
    void* output,
    size_t output_size,
    size_t* bytes_written) {
    
    // Validate parameters
    if (!is_valid_context(context)) {
        return [MODULE_NAME_UPPER]_INVALID_PARAM;
    }
    
    if (!input && input_size > 0) {
        return [MODULE_NAME_UPPER]_INVALID_PARAM;
    }
    
    if (!output && output_size > 0) {
        return [MODULE_NAME_UPPER]_INVALID_PARAM;
    }
    
    DEBUG_LOG("Processing %zu bytes of data", input_size);
    
    // Check buffer size
    if (input_size > context->buffer_size) {
        DEBUG_LOG("Input size exceeds buffer capacity");
        return [MODULE_NAME_UPPER]_ERROR;
    }
    
    // Perform data processing
    // [Add actual data processing logic here]
    
    // Example processing: copy input to output
    size_t bytes_to_copy = (input_size < output_size) ? input_size : output_size;
    if (bytes_to_copy > 0) {
        memcpy(output, input, bytes_to_copy);
    }
    
    if (bytes_written) {
        *bytes_written = bytes_to_copy;
    }
    
    // Call registered callback if present
    if (context->callback) {
        [module_name]_error_t callback_result = context->callback(
            context, output, bytes_to_copy, context->callback_user_data);
        if (callback_result != [MODULE_NAME_UPPER]_SUCCESS) {
            DEBUG_LOG("Callback returned error: %d", callback_result);
            return callback_result;
        }
    }
    
    DEBUG_LOG("Data processed successfully, %zu bytes written", bytes_to_copy);
    return [MODULE_NAME_UPPER]_SUCCESS;
}

[module_name]_error_t [module_name]_register_callback(
    [module_name]_context_t* context,
    [module_name]_callback_t callback,
    void* user_data) {
    
    if (!is_valid_context(context)) {
        return [MODULE_NAME_UPPER]_INVALID_PARAM;
    }
    
    DEBUG_LOG("Registering callback function");
    
    context->callback = callback;
    context->callback_user_data = user_data;
    
    return [MODULE_NAME_UPPER]_SUCCESS;
}

void [module_name]_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch) {
    if (major) *major = [MODULE_NAME_UPPER]_VERSION_MAJOR;
    if (minor) *minor = [MODULE_NAME_UPPER]_VERSION_MINOR;
    if (patch) *patch = [MODULE_NAME_UPPER]_VERSION_PATCH;
}

const char* [module_name]_error_string([module_name]_error_t error) {
    switch (error) {
        case [MODULE_NAME_UPPER]_SUCCESS:
            return "Success";
        case [MODULE_NAME_UPPER]_ERROR:
            return "Generic error";
        case [MODULE_NAME_UPPER]_INVALID_PARAM:
            return "Invalid parameter";
        case [MODULE_NAME_UPPER]_OUT_OF_MEMORY:
            return "Out of memory";
        case [MODULE_NAME_UPPER]_NOT_FOUND:
            return "Not found";
        case [MODULE_NAME_UPPER]_PERMISSION_DENIED:
            return "Permission denied";
        default:
            return "Unknown error";
    }
}

void [module_name]_shutdown(void) {
    if (!g_module_initialized) {
        DEBUG_LOG("Module not initialized");
        return;
    }
    
    DEBUG_LOG("Shutting down [module name] module");
    
    // Decrement reference count
    g_module_refcount--;
    if (g_module_refcount > 0) {
        DEBUG_LOG("Module still has references, not shutting down");
        return;
    }
    
    // Perform module cleanup
    // [Add module-specific cleanup code here]
    
    g_module_initialized = false;
    memset(&g_module_config, 0, sizeof(g_module_config));
    
    DEBUG_LOG("[Module name] module shut down successfully");
}

//==============================================================================
// INTERNAL FUNCTION IMPLEMENTATIONS
//==============================================================================

static bool is_valid_context(const [module_name]_context_t* context) {
    return context != NULL && 
           context->magic == CONTEXT_MAGIC && 
           context->initialized;
}

static void init_default_config([module_name]_config_t* config) {
    memset(config, 0, sizeof([module_name]_config_t));
    
    config->version = [MODULE_NAME_UPPER]_VERSION_MAJOR;
    config->flags = [MODULE_NAME_UPPER]_FLAG_NONE;
    config->buffer_size = INTERNAL_BUFFER_SIZE;
    config->timeout_ms = 5000;
    config->user_data = NULL;
}

static void internal_cleanup([module_name]_context_t* context) {
    if (!context) return;
    
    DEBUG_LOG("Performing internal context cleanup");
    
    // Reset callback
    context->callback = NULL;
    context->callback_user_data = NULL;
    
    // Clear configuration
    memset(&context->config, 0, sizeof(context->config));
    
    // Mark as uninitialized
    context->initialized = false;
    
    DEBUG_LOG("Internal cleanup completed");
}
```

### Unit Test Template

```c
/**
 * @file test_[module_name].c
 * @brief Unit tests for [module name] module
 * 
 * @author RaeenOS Development Team
 * @date [Current Date]
 * @version 1.0.0
 */

#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "../include/[module_name].h"

//==============================================================================
// TEST FIXTURES
//==============================================================================

static [module_name]_context_t* test_context = NULL;

void setup(void) {
    // Initialize module
    [module_name]_error_t result = [module_name]_init(NULL);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS, 
                 "Failed to initialize module");
    
    // Create test context
    result = [module_name]_create_context(&test_context, NULL);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Failed to create test context");
    cr_assert_not_null(test_context, "Context should not be NULL");
}

void teardown(void) {
    // Destroy test context
    if (test_context) {
        [module_name]_destroy_context(test_context);
        test_context = NULL;
    }
    
    // Shutdown module
    [module_name]_shutdown();
}

//==============================================================================
// INITIALIZATION TESTS
//==============================================================================

Test(module_init, successful_initialization) {
    [module_name]_error_t result = [module_name]_init(NULL);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Module initialization should succeed");
    
    [module_name]_shutdown();
}

Test(module_init, double_initialization) {
    [module_name]_error_t result1 = [module_name]_init(NULL);
    cr_assert_eq(result1, [MODULE_NAME_UPPER]_SUCCESS,
                 "First initialization should succeed");
    
    [module_name]_error_t result2 = [module_name]_init(NULL);
    cr_assert_eq(result2, [MODULE_NAME_UPPER]_SUCCESS,
                 "Second initialization should succeed (ref counting)");
    
    [module_name]_shutdown();
    [module_name]_shutdown();
}

Test(module_init, custom_configuration) {
    [module_name]_config_t config = {
        .version = [MODULE_NAME_UPPER]_VERSION_MAJOR,
        .flags = [MODULE_NAME_UPPER]_FLAG_VERBOSE,
        .buffer_size = 2048,
        .timeout_ms = 10000,
        .user_data = NULL
    };
    
    [module_name]_error_t result = [module_name]_init(&config);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Initialization with custom config should succeed");
    
    [module_name]_shutdown();
}

//==============================================================================
// CONTEXT TESTS
//==============================================================================

Test(context, create_and_destroy, .init = setup, .fini = teardown) {
    // Context is created in setup, just verify it exists
    cr_assert_not_null(test_context, "Context should be created");
    
    // Test multiple context creation
    [module_name]_context_t* context2 = NULL;
    [module_name]_error_t result = [module_name]_create_context(&context2, NULL);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Second context creation should succeed");
    cr_assert_not_null(context2, "Second context should not be NULL");
    
    [module_name]_destroy_context(context2);
}

Test(context, null_parameter_handling) {
    [module_name]_context_t* context = NULL;
    
    // Test NULL context parameter
    [module_name]_error_t result = [module_name]_create_context(NULL, NULL);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_INVALID_PARAM,
                 "Should return invalid parameter for NULL context pointer");
    
    // Test destroying NULL context (should not crash)
    [module_name]_destroy_context(NULL);
}

//==============================================================================
// FUNCTIONALITY TESTS
//==============================================================================

Test(functionality, basic_data_processing, .init = setup, .fini = teardown) {
    const char* input_data = "Hello, World!";
    char output_buffer[256];
    size_t bytes_written = 0;
    
    [module_name]_error_t result = [module_name]_process_data(
        test_context,
        input_data,
        strlen(input_data),
        output_buffer,
        sizeof(output_buffer),
        &bytes_written
    );
    
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Data processing should succeed");
    cr_assert_eq(bytes_written, strlen(input_data),
                 "Bytes written should match input size");
    cr_assert_str_eq(output_buffer, input_data,
                     "Output should match input");
}

Test(functionality, empty_data_processing, .init = setup, .fini = teardown) {
    char output_buffer[256];
    size_t bytes_written = 0;
    
    [module_name]_error_t result = [module_name]_process_data(
        test_context,
        NULL,
        0,
        output_buffer,
        sizeof(output_buffer),
        &bytes_written
    );
    
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Empty data processing should succeed");
    cr_assert_eq(bytes_written, 0,
                 "No bytes should be written for empty input");
}

Test(functionality, invalid_parameters, .init = setup, .fini = teardown) {
    const char* input_data = "Test data";
    char output_buffer[256];
    
    // Test invalid context
    [module_name]_error_t result = [module_name]_process_data(
        NULL, input_data, strlen(input_data), 
        output_buffer, sizeof(output_buffer), NULL
    );
    cr_assert_eq(result, [MODULE_NAME_UPPER]_INVALID_PARAM,
                 "Should return invalid parameter for NULL context");
    
    // Test NULL input with non-zero size
    result = [module_name]_process_data(
        test_context, NULL, 10, 
        output_buffer, sizeof(output_buffer), NULL
    );
    cr_assert_eq(result, [MODULE_NAME_UPPER]_INVALID_PARAM,
                 "Should return invalid parameter for NULL input with size > 0");
}

//==============================================================================
// CALLBACK TESTS
//==============================================================================

static [module_name]_error_t test_callback(
    [module_name]_context_t* context,
    const void* data,
    size_t size,
    void* user_data) {
    
    (void)context; // Unused parameter
    
    // Verify callback parameters
    cr_assert_not_null(data, "Callback data should not be NULL");
    cr_assert_gt(size, 0, "Callback size should be greater than 0");
    
    // Set user data flag
    if (user_data) {
        *(bool*)user_data = true;
    }
    
    return [MODULE_NAME_UPPER]_SUCCESS;
}

Test(callback, callback_registration, .init = setup, .fini = teardown) {
    bool callback_called = false;
    
    // Register callback
    [module_name]_error_t result = [module_name]_register_callback(
        test_context, test_callback, &callback_called
    );
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Callback registration should succeed");
    
    // Process data to trigger callback
    const char* input_data = "Test";
    char output_buffer[256];
    
    result = [module_name]_process_data(
        test_context,
        input_data,
        strlen(input_data),
        output_buffer,
        sizeof(output_buffer),
        NULL
    );
    
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Data processing should succeed");
    cr_assert(callback_called, "Callback should have been called");
}

//==============================================================================
// VERSION AND ERROR TESTS
//==============================================================================

Test(version, version_information) {
    uint32_t major, minor, patch;
    
    [module_name]_get_version(&major, &minor, &patch);
    
    cr_assert_eq(major, [MODULE_NAME_UPPER]_VERSION_MAJOR,
                 "Major version should match");
    cr_assert_eq(minor, [MODULE_NAME_UPPER]_VERSION_MINOR,
                 "Minor version should match");
    cr_assert_eq(patch, [MODULE_NAME_UPPER]_VERSION_PATCH,
                 "Patch version should match");
}

Test(error, error_string_conversion) {
    const char* success_str = [module_name]_error_string([MODULE_NAME_UPPER]_SUCCESS);
    cr_assert_not_null(success_str, "Success error string should not be NULL");
    cr_assert_str_eq(success_str, "Success", "Success string should match");
    
    const char* error_str = [module_name]_error_string([MODULE_NAME_UPPER]_ERROR);
    cr_assert_not_null(error_str, "Error string should not be NULL");
    
    const char* invalid_str = [module_name]_error_string([MODULE_NAME_UPPER]_INVALID_PARAM);
    cr_assert_not_null(invalid_str, "Invalid param string should not be NULL");
}

//==============================================================================
// PERFORMANCE TESTS
//==============================================================================

Test(performance, large_data_processing, .init = setup, .fini = teardown) {
    const size_t data_size = 1024 * 1024; // 1MB
    char* input_data = malloc(data_size);
    char* output_buffer = malloc(data_size);
    
    cr_assert_not_null(input_data, "Input buffer allocation should succeed");
    cr_assert_not_null(output_buffer, "Output buffer allocation should succeed");
    
    // Fill input with test pattern
    for (size_t i = 0; i < data_size; i++) {
        input_data[i] = (char)(i % 256);
    }
    
    // Measure processing time
    clock_t start = clock();
    
    [module_name]_error_t result = [module_name]_process_data(
        test_context,
        input_data,
        data_size,
        output_buffer,
        data_size,
        NULL
    );
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                 "Large data processing should succeed");
    cr_assert_lt(cpu_time, 1.0, "Processing should complete within 1 second");
    
    free(input_data);
    free(output_buffer);
}

//==============================================================================
// STRESS TESTS
//==============================================================================

Test(stress, multiple_contexts, .timeout = 10) {
    const int num_contexts = 100;
    [module_name]_context_t* contexts[num_contexts];
    
    // Initialize module
    [module_name]_error_t result = [module_name]_init(NULL);
    cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS);
    
    // Create multiple contexts
    for (int i = 0; i < num_contexts; i++) {
        result = [module_name]_create_context(&contexts[i], NULL);
        cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                     "Context %d creation should succeed", i);
    }
    
    // Use contexts
    const char* test_data = "Stress test data";
    char output_buffer[256];
    
    for (int i = 0; i < num_contexts; i++) {
        result = [module_name]_process_data(
            contexts[i],
            test_data,
            strlen(test_data),
            output_buffer,
            sizeof(output_buffer),
            NULL
        );
        cr_assert_eq(result, [MODULE_NAME_UPPER]_SUCCESS,
                     "Context %d processing should succeed", i);
    }
    
    // Destroy contexts
    for (int i = 0; i < num_contexts; i++) {
        [module_name]_destroy_context(contexts[i]);
    }
    
    [module_name]_shutdown();
}
```

---

## Usage Instructions

### Creating a New Module

1. **Choose Module Location**: Determine where your module fits in the repository structure
2. **Copy Template**: Copy the appropriate template directory structure
3. **Customize Names**: Replace all `[module_name]` placeholders with your actual module name
4. **Update Ownership**: Set the correct primary and secondary agents in `AGENT_OWNERS.md`
5. **Implement Functionality**: Fill in the actual implementation
6. **Write Tests**: Create comprehensive unit and integration tests
7. **Update Documentation**: Complete the README and API documentation

### Template Customization Checklist

- [ ] Replace `[module_name]` with actual module name (lowercase with underscores)
- [ ] Replace `[MODULE_NAME_UPPER]` with uppercase module name
- [ ] Replace `[Module Name]` with human-readable module name
- [ ] Replace `[primary-agent-name]` with responsible agent name
- [ ] Replace `[secondary-agent-name]` with secondary agent name
- [ ] Replace `[MODULE_HEX]` with unique hex identifier
- [ ] Update version numbers and dates
- [ ] Customize functionality descriptions
- [ ] Add module-specific constants and types
- [ ] Implement actual module logic
- [ ] Write comprehensive tests
- [ ] Update build dependencies

### Integration Steps

1. **Add to Build System**: Update parent Makefile to include new module
2. **Register Ownership**: Add module to `.github/CODEOWNERS`
3. **Update Documentation**: Add module to architecture documentation
4. **Coordinate with Agents**: Inform relevant agents of new module
5. **Quality Assurance**: Ensure module passes all quality gates

This template provides a solid foundation for creating new modules that integrate seamlessly with the RaeenOS 42-agent development architecture.