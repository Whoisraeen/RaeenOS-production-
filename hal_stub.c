/**
 * @file hal_stub.c
 * @brief Hardware Abstraction Layer Integration
 * 
 * This file integrates the production HAL implementation with the kernel.
 * It initializes the HAL system and provides backward compatibility.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "kernel/include/hal_interface.h"
#include "kernel/include/types.h"

// Include production HAL components
extern int hal_init(void);
extern void hal_shutdown(void);

// Global HAL pointer (defined in hal_core.c)
extern hal_operations_t* hal;

/**
 * Initialize the HAL system for kernel use
 * This should be called early in kernel initialization
 */
int kernel_hal_init(void)
{
    // Initialize the production HAL
    int result = hal_init();
    if (result != HAL_SUCCESS) {
        // HAL initialization failed - system cannot continue
        return result;
    }
    
    // Verify HAL is properly initialized
    if (!hal) {
        return -1;
    }
    
    // Initialize HAL subsystems
    if (hal->cpu_init) {
        result = hal->cpu_init();
        if (result != HAL_SUCCESS) {
            return result;
        }
    }
    
    if (hal->irq_init) {
        result = hal->irq_init();
        if (result != HAL_SUCCESS) {
            return result;
        }
    }
    
    if (hal->timer_init) {
        result = hal->timer_init();
        if (result != HAL_SUCCESS) {
            return result;
        }
    }
    
    return HAL_SUCCESS;
}

/**
 * Shutdown the HAL system
 */
void kernel_hal_shutdown(void)
{
    if (hal && hal->shutdown) {
        hal->shutdown();
    }
    
    hal_shutdown();
}

/**
 * Get HAL version information
 */
void kernel_hal_get_version(uint32_t* major, uint32_t* minor, uint32_t* api_version)
{
    if (major) *major = 1;
    if (minor) *minor = 0;
    if (api_version) *api_version = HAL_API_VERSION;
}

// Backward compatibility - Legacy function names that may be used by existing kernel code
// These redirect to the new HAL interface

void* legacy_mem_alloc_pages(size_t pages) {
    if (hal && hal->mem_alloc_pages) {
        return hal->mem_alloc_pages(pages, HAL_MEM_READ | HAL_MEM_WRITE);
    }
    return NULL;
}

void legacy_mem_free_pages(void* addr, size_t pages) {
    if (hal && hal->mem_free_pages) {
        hal->mem_free_pages(addr, pages);
    }
}

uint8_t legacy_io_read8(uint16_t port) {
    if (hal && hal->io_read8) {
        return hal->io_read8(port);
    }
    return 0;
}

void legacy_io_write8(uint16_t port, uint8_t value) {
    if (hal && hal->io_write8) {
        hal->io_write8(port, value);
    }
}

unsigned long legacy_irq_save(void) {
    if (hal && hal->irq_save) {
        return hal->irq_save();
    }
    return 0;
}

void legacy_irq_restore(unsigned long flags) {
    if (hal && hal->irq_restore) {
        hal->irq_restore(flags);
    }
}