/**
 * @file hal_core.c
 * @brief Hardware Abstraction Layer Core Implementation
 * 
 * This module provides the central HAL architecture for RaeenOS, including
 * platform detection, initialization, and unified hardware interface management.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#include "../include/hal_interface.h"
#include "../include/types.h"
#include "../include/errno.h"
#include "../pmm_production.h"
#include "../vmm_production.h"
// Using types.h for kernel build

// Maximum number of supported platforms
#define MAX_PLATFORMS 8

// Platform registry structure
typedef struct {
    hal_operations_t* ops;
    hal_arch_t arch;
    const char* name;
    bool active;
} hal_platform_entry_t;

// Global HAL state
static struct {
    hal_operations_t* current_ops;
    hal_arch_t current_arch;
    hal_platform_entry_t platforms[MAX_PLATFORMS];
    size_t platform_count;
    bool initialized;
    hal_cpu_features_t cpu_features;
} hal_state = {0};

// Global HAL operations pointer (for interface macros)
hal_operations_t* hal = NULL;

// Forward declarations
static hal_arch_t detect_architecture(void);
static int initialize_platform(hal_arch_t arch);
static void populate_cpu_features(void);

/**
 * Initialize the Hardware Abstraction Layer
 * 
 * @return 0 on success, negative error code on failure
 */
int hal_init(void)
{
    if (hal_state.initialized) {
        return HAL_SUCCESS;
    }

    // Detect the current architecture
    hal_state.current_arch = detect_architecture();
    if (hal_state.current_arch == HAL_ARCH_UNKNOWN) {
        return -ENOTSUP;
    }

    // Initialize the platform-specific implementation
    int result = initialize_platform(hal_state.current_arch);
    if (result != HAL_SUCCESS) {
        return result;
    }

    // Populate CPU features
    populate_cpu_features();

    // Initialize the selected platform
    if (hal_state.current_ops && hal_state.current_ops->init) {
        result = hal_state.current_ops->init();
        if (result != HAL_SUCCESS) {
            return result;
        }
    }

    // Set global HAL pointer
    hal = hal_state.current_ops;
    hal_state.initialized = true;

    return HAL_SUCCESS;
}

/**
 * Shutdown the Hardware Abstraction Layer
 */
void hal_shutdown(void)
{
    if (!hal_state.initialized) {
        return;
    }

    // Shutdown the current platform
    if (hal_state.current_ops && hal_state.current_ops->shutdown) {
        hal_state.current_ops->shutdown();
    }

    // Clear global state
    hal = NULL;
    hal_state.current_ops = NULL;
    hal_state.initialized = false;
}

/**
 * Register a platform implementation
 * 
 * @param ops Platform operations structure
 * @return 0 on success, negative error code on failure
 */
int hal_register_platform(hal_operations_t* ops)
{
    if (!ops || hal_state.platform_count >= MAX_PLATFORMS) {
        return -EINVAL;
    }

    hal_platform_entry_t* entry = &hal_state.platforms[hal_state.platform_count];
    entry->ops = ops;
    entry->active = false;
    
    // Determine architecture from platform data if available
    if (ops->platform_data) {
        // Platform-specific architecture detection would go here
    }

    hal_state.platform_count++;
    return HAL_SUCCESS;
}

/**
 * Get current HAL operations
 * 
 * @return Pointer to current HAL operations or NULL if not initialized
 */
hal_operations_t* hal_get_ops(void)
{
    return hal_state.current_ops;
}

/**
 * Get current architecture
 * 
 * @return Current architecture type
 */
hal_arch_t hal_get_architecture(void)
{
    return hal_state.current_arch;
}

/**
 * Get architecture name string
 * 
 * @return Architecture name or "Unknown"
 */
const char* hal_get_architecture_name(void)
{
    switch (hal_state.current_arch) {
        case HAL_ARCH_X86_64:
            return "x86_64";
        case HAL_ARCH_ARM64:
            return "aarch64";
        case HAL_ARCH_RISCV64:
            return "riscv64";
        case HAL_ARCH_MIPS64:
            return "mips64";
        default:
            return "Unknown";
    }
}

/**
 * Get CPU features
 * 
 * @param features Pointer to features structure to populate
 * @return 0 on success, negative error code on failure
 */
int hal_get_cpu_features(hal_cpu_features_t* features)
{
    if (!features) {
        return -EINVAL;
    }

    *features = hal_state.cpu_features;
    return HAL_SUCCESS;
}

/**
 * Detect the current processor architecture
 * 
 * @return Detected architecture type
 */
static hal_arch_t detect_architecture(void)
{
#if defined(__x86_64__) || defined(__amd64__)
    return HAL_ARCH_X86_64;
#elif defined(__aarch64__) || defined(__arm64__)
    return HAL_ARCH_ARM64;
#elif defined(__riscv) && (__riscv_xlen == 64)
    return HAL_ARCH_RISCV64;
#elif defined(__mips64__)
    return HAL_ARCH_MIPS64;
#else
    return HAL_ARCH_UNKNOWN;
#endif
}

/**
 * Initialize platform-specific implementation
 * 
 * @param arch Target architecture
 * @return 0 on success, negative error code on failure
 */
static int initialize_platform(hal_arch_t arch)
{
    // External platform initialization functions
    extern int hal_x86_64_init(hal_operations_t** ops);
    extern int hal_arm64_init(hal_operations_t** ops);
    extern int hal_riscv64_init(hal_operations_t** ops);
    extern int hal_mips64_init(hal_operations_t** ops);

    hal_operations_t* ops = NULL;
    int result = -ENOTSUP;

    switch (arch) {
        case HAL_ARCH_X86_64:
            result = hal_x86_64_init(&ops);
            break;
        case HAL_ARCH_ARM64:
            result = hal_arm64_init(&ops);
            break;
        case HAL_ARCH_RISCV64:
            result = hal_riscv64_init(&ops);
            break;
        case HAL_ARCH_MIPS64:
            result = hal_mips64_init(&ops);
            break;
        default:
            return -ENOTSUP;
    }

    if (result == HAL_SUCCESS && ops) {
        hal_state.current_ops = ops;
        
        // Mark platform as active
        for (size_t i = 0; i < hal_state.platform_count; i++) {
            if (hal_state.platforms[i].ops == ops) {
                hal_state.platforms[i].active = true;
                hal_state.platforms[i].arch = arch;
                break;
            }
        }
    }

    return result;
}

/**
 * Populate CPU features structure
 */
static void populate_cpu_features(void)
{
    hal_cpu_features_t* features = &hal_state.cpu_features;
    
    // Initialize with defaults
    features->has_mmu = true;  // All supported architectures have MMU
    features->has_fpu = true;  // All modern processors have FPU
    features->has_atomic64 = true; // All 64-bit architectures support this
    features->page_size = 4096; // Default page size
    features->cache_line_size = 64; // Common cache line size
    features->num_cores = 1; // Will be updated by platform-specific code
    
    // Copy architecture name
    const char* arch_name = hal_get_architecture_name();
    size_t name_len = 0;
    while (arch_name[name_len] && name_len < sizeof(features->arch_name) - 1) {
        features->arch_name[name_len] = arch_name[name_len];
        name_len++;
    }
    features->arch_name[name_len] = '\0';

    // Let platform-specific code fill in detailed features
    if (hal_state.current_ops && hal_state.current_ops->cpu_get_features) {
        hal_state.current_ops->cpu_get_features(features);
    }
}

/**
 * Default implementations for optional HAL operations
 */

static int default_platform_init(void)
{
    return HAL_SUCCESS;
}

static void default_platform_shutdown(void)
{
    // Default: no-op
}

static int default_dt_init(void)
{
    return HAL_ERR_NOT_SUPPORTED;
}

static void* default_dt_get_property(const char* path, const char* property, size_t* len)
{
    (void)path;
    (void)property;
    (void)len;
    return NULL;
}

static int default_dt_get_irq(const char* path, int index)
{
    (void)path;
    (void)index;
    return -1;
}

static phys_addr_t default_dt_get_reg(const char* path, int index, size_t* size)
{
    (void)path;
    (void)index;
    (void)size;
    return 0;
}

static void default_debug_putchar(char c)
{
    // Default: no-op (could use early VGA or serial)
    (void)c;
}

static char default_debug_getchar(void)
{
    return 0;
}

static void default_debug_puts(const char* str)
{
    if (!str) return;
    while (*str) {
        default_debug_putchar(*str++);
    }
}

static int default_debug_early_init(void)
{
    return HAL_SUCCESS;
}

/**
 * Fill in default implementations for optional operations
 * 
 * @param ops Operations structure to populate
 */
void hal_fill_defaults(hal_operations_t* ops)
{
    if (!ops) return;

    // Platform-specific defaults
    if (!ops->platform_init) {
        ops->platform_init = default_platform_init;
    }
    if (!ops->platform_shutdown) {
        ops->platform_shutdown = default_platform_shutdown;
    }

    // Device tree defaults (for non-ARM platforms)
    if (!ops->dt_init) {
        ops->dt_init = default_dt_init;
    }
    if (!ops->dt_get_property) {
        ops->dt_get_property = default_dt_get_property;
    }
    if (!ops->dt_get_irq) {
        ops->dt_get_irq = default_dt_get_irq;
    }
    if (!ops->dt_get_reg) {
        ops->dt_get_reg = default_dt_get_reg;
    }

    // Debug defaults
    if (!ops->debug_putchar) {
        ops->debug_putchar = default_debug_putchar;
    }
    if (!ops->debug_getchar) {
        ops->debug_getchar = default_debug_getchar;
    }
    if (!ops->debug_puts) {
        ops->debug_puts = default_debug_puts;
    }
    if (!ops->debug_early_init) {
        ops->debug_early_init = default_debug_early_init;
    }
}

/**
 * Validate HAL operations structure
 * 
 * @param ops Operations structure to validate
 * @return 0 if valid, negative error code if invalid
 */
int hal_validate_ops(hal_operations_t* ops)
{
    if (!ops) {
        return -EINVAL;
    }

    // Check required operations
    if (!ops->init || !ops->cpu_init || !ops->cpu_halt || 
        !ops->mem_alloc_pages || !ops->mem_free_pages ||
        !ops->irq_init || !ops->irq_save || !ops->irq_restore) {
        return -EINVAL;
    }

    return HAL_SUCCESS;
}