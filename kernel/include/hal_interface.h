#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

/**
 * @file hal_interface.h
 * @brief Hardware Abstraction Layer (HAL) Interface for RaeenOS
 * 
 * This interface provides a standardized API for hardware interactions
 * across different architectures (x86-64, ARM64, RISC-V).
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"

#ifdef __cplusplus
extern "C" {
#endif

// HAL API version for compatibility checking
#define HAL_API_VERSION 1

// Memory flags for HAL operations
#define HAL_MEM_READ       (1 << 0)
#define HAL_MEM_WRITE      (1 << 1)
#define HAL_MEM_EXECUTE    (1 << 2)
#define HAL_MEM_KERNEL     (1 << 3)
#define HAL_MEM_USER       (1 << 4)
#define HAL_MEM_CACHE      (1 << 5)
#define HAL_MEM_NOCACHE    (1 << 6)
#define HAL_MEM_DEVICE     (1 << 7)
#define HAL_MEM_ATOMIC     (1 << 8)
#define HAL_MEM_ZERO       (1 << 9)

// IRQ flags
#define HAL_IRQ_SHARED     (1 << 0)
#define HAL_IRQ_ONESHOT    (1 << 1)
#define HAL_IRQ_EDGE       (1 << 2)
#define HAL_IRQ_LEVEL      (1 << 3)

// DMA flags
#define HAL_DMA_COHERENT   (1 << 0)
#define HAL_DMA_STREAMING  (1 << 1)
#define HAL_DMA_BIDIRECTIONAL (0)
#define HAL_DMA_TO_DEVICE  (1)
#define HAL_DMA_FROM_DEVICE (2)

// Forward declarations
typedef struct hal_device hal_device_t;
typedef struct hal_dma_buffer hal_dma_buffer_t;

// Physical address type (platform dependent)
typedef uint64_t phys_addr_t;
typedef uint64_t dma_addr_t;

// CPU features structure
typedef struct {
    bool has_mmu;           // Memory management unit
    bool has_fpu;           // Floating point unit
    bool has_simd;          // SIMD instructions (SSE, NEON, etc.)
    bool has_virtualization;// Hardware virtualization support
    bool has_crypto;        // Hardware crypto acceleration
    bool has_atomic64;      // 64-bit atomic operations
    bool has_apic;          // Advanced Programmable Interrupt Controller
    uint32_t cache_line_size;
    uint32_t page_size;
    uint32_t num_cores;
    char arch_name[32];     // Architecture name (x86_64, aarch64, etc.)
} hal_cpu_features_t;

// Memory region descriptor
typedef struct {
    phys_addr_t start;
    uint64_t size;
    uint32_t type;          // Memory type (RAM, ROM, DEVICE, etc.)
    uint32_t flags;         // Memory flags
} hal_memory_region_t;

// IRQ handler function type
typedef void (*hal_irq_handler_t)(int irq, void* data);

// Timer callback function type
typedef void (*hal_timer_callback_t)(void* data);

// DMA completion callback
typedef void (*hal_dma_callback_t)(hal_dma_buffer_t* buffer, int status, void* data);

/**
 * Hardware Abstraction Layer Operations Structure
 * 
 * This structure contains function pointers for all HAL operations.
 * Platform-specific implementations provide concrete implementations.
 */
typedef struct hal_operations {
    // Initialization and shutdown
    int (*init)(void);
    void (*shutdown)(void);
    
    // CPU Management
    int (*cpu_init)(void);
    void (*cpu_idle)(void);
    void (*cpu_halt)(void) __attribute__((noreturn));
    uint64_t (*cpu_timestamp)(void);           // High-resolution timestamp
    void (*cpu_pause)(void);                   // CPU pause instruction
    void (*cpu_memory_barrier)(void);          // Memory barrier
    int (*cpu_get_features)(hal_cpu_features_t* features);
    
    // SMP (Symmetric Multiprocessing) Support
    int (*smp_start_cpu)(uint32_t cpu_id, void (*entry_point)(void));
    uint32_t (*smp_get_cpu_id)(void);
    uint32_t (*smp_get_cpu_count)(void);
    void (*smp_send_ipi)(uint32_t cpu_id, uint32_t vector);
    
    // Memory Operations
    void* (*mem_alloc_pages)(size_t pages, uint32_t flags);
    void (*mem_free_pages)(void* addr, size_t pages);
    int (*mem_map_physical)(phys_addr_t phys, void* virt, size_t size, uint32_t flags);
    int (*mem_unmap)(void* virt, size_t size);
    int (*mem_protect)(void* virt, size_t size, uint32_t flags);
    phys_addr_t (*mem_virt_to_phys)(void* virt);
    void* (*mem_phys_to_virt)(phys_addr_t phys);
    int (*mem_get_regions)(hal_memory_region_t* regions, size_t* count);
    
    // Cache Operations
    void (*cache_flush_all)(void);
    void (*cache_flush_range)(void* start, size_t size);
    void (*cache_invalidate_range)(void* start, size_t size);
    void (*cache_clean_range)(void* start, size_t size);
    
    // Interrupt Management
    int (*irq_init)(void);
    int (*irq_register)(int irq, hal_irq_handler_t handler, uint32_t flags, const char* name, void* data);
    int (*irq_unregister)(int irq, void* data);
    void (*irq_enable)(int irq);
    void (*irq_disable)(int irq);
    void (*irq_mask)(int irq);
    void (*irq_unmask)(int irq);
    void (*irq_end)(int irq);
    int (*irq_get_pending)(void);
    
    // Interrupt Control (disable/enable all interrupts)
    unsigned long (*irq_save)(void);           // Disable interrupts, return flags
    void (*irq_restore)(unsigned long flags);  // Restore interrupt state
    
    // I/O Port Operations (primarily for x86)
    uint8_t (*io_read8)(uint16_t port);
    uint16_t (*io_read16)(uint16_t port);
    uint32_t (*io_read32)(uint16_t port);
    void (*io_write8)(uint16_t port, uint8_t value);
    void (*io_write16)(uint16_t port, uint16_t value);
    void (*io_write32)(uint16_t port, uint32_t value);
    
    // Memory-Mapped I/O Operations
    uint8_t (*mmio_read8)(volatile void* addr);
    uint16_t (*mmio_read16)(volatile void* addr);
    uint32_t (*mmio_read32)(volatile void* addr);
    uint64_t (*mmio_read64)(volatile void* addr);
    void (*mmio_write8)(volatile void* addr, uint8_t value);
    void (*mmio_write16)(volatile void* addr, uint16_t value);
    void (*mmio_write32)(volatile void* addr, uint32_t value);
    void (*mmio_write64)(volatile void* addr, uint64_t value);
    
    // DMA Operations
    hal_dma_buffer_t* (*dma_alloc)(size_t size, uint32_t flags);
    void (*dma_free)(hal_dma_buffer_t* buffer);
    dma_addr_t (*dma_map)(void* virt, size_t size, int direction);
    void (*dma_unmap)(dma_addr_t dma_addr, size_t size, int direction);
    int (*dma_sync_for_cpu)(hal_dma_buffer_t* buffer);
    int (*dma_sync_for_device)(hal_dma_buffer_t* buffer);
    
    // Timer Operations
    int (*timer_init)(void);
    uint64_t (*timer_get_ticks)(void);         // Get current timer ticks
    uint64_t (*timer_get_frequency)(void);     // Get timer frequency (Hz)
    int (*timer_set_periodic)(uint32_t hz, hal_timer_callback_t callback, void* data);
    int (*timer_set_oneshot)(uint64_t usecs, hal_timer_callback_t callback, void* data);
    void (*timer_cancel)(int timer_id);
    
    // Power Management
    int (*power_suspend)(void);
    int (*power_resume)(void);
    int (*power_set_cpu_freq)(uint32_t cpu_id, uint32_t freq_khz);
    uint32_t (*power_get_cpu_freq)(uint32_t cpu_id);
    int (*power_set_voltage)(uint32_t rail_id, uint32_t voltage_mv);
    
    // Device Tree / ACPI Support
    int (*dt_init)(void);                      // Device tree initialization
    void* (*dt_get_property)(const char* path, const char* property, size_t* len);
    int (*dt_get_irq)(const char* path, int index);
    phys_addr_t (*dt_get_reg)(const char* path, int index, size_t* size);
    
    // Debug Support
    void (*debug_putchar)(char c);
    char (*debug_getchar)(void);
    void (*debug_puts)(const char* str);
    int (*debug_early_init)(void);
    
    // Platform-Specific Extensions
    void* platform_data;                      // Platform-specific data
    int (*platform_init)(void);               // Platform-specific initialization
    void (*platform_shutdown)(void);          // Platform-specific shutdown
} hal_operations_t;

// DMA buffer structure
struct hal_dma_buffer {
    void* virt_addr;            // Virtual address
    dma_addr_t dma_addr;        // DMA address
    size_t size;                // Buffer size
    uint32_t flags;             // DMA flags
    hal_dma_callback_t callback; // Completion callback
    void* callback_data;        // Callback data
    void* private_data;         // HAL private data
};

// HAL device structure for device management
struct hal_device {
    char name[64];              // Device name
    uint32_t device_id;         // Unique device ID
    uint32_t vendor_id;         // Vendor ID
    uint32_t class_id;          // Device class
    phys_addr_t base_addr;      // Base physical address
    size_t mem_size;            // Memory region size
    int irq;                    // IRQ number
    void* private_data;         // Device-specific data
};

// Error codes specific to HAL
#define HAL_SUCCESS           0
#define HAL_ERR_NOT_SUPPORTED -1001
#define HAL_ERR_NO_MEMORY     -1002
#define HAL_ERR_INVALID_PARAM -1003
#define HAL_ERR_DEVICE_BUSY   -1004
#define HAL_ERR_TIMEOUT       -1005
#define HAL_ERR_IO_ERROR      -1006

// Global HAL operations pointer
extern hal_operations_t* hal;

// HAL Initialization and Management Functions
int hal_init(void);
void hal_shutdown(void);
int hal_register_platform(hal_operations_t* ops);
hal_operations_t* hal_get_ops(void);

// Architecture Detection
typedef enum {
    HAL_ARCH_UNKNOWN,
    HAL_ARCH_X86_64,
    HAL_ARCH_ARM64,
    HAL_ARCH_RISCV64,
    HAL_ARCH_MIPS64
} hal_arch_t;

hal_arch_t hal_get_architecture(void);
const char* hal_get_architecture_name(void);

// Utility macros for common operations
#define HAL_READ8(addr)     (hal->mmio_read8(addr))
#define HAL_READ16(addr)    (hal->mmio_read16(addr))
#define HAL_READ32(addr)    (hal->mmio_read32(addr))
#define HAL_READ64(addr)    (hal->mmio_read64(addr))
#define HAL_WRITE8(addr, val)  (hal->mmio_write8(addr, val))
#define HAL_WRITE16(addr, val) (hal->mmio_write16(addr, val))
#define HAL_WRITE32(addr, val) (hal->mmio_write32(addr, val))
#define HAL_WRITE64(addr, val) (hal->mmio_write64(addr, val))

#define HAL_IO_READ8(port)     (hal->io_read8(port))
#define HAL_IO_READ16(port)    (hal->io_read16(port))
#define HAL_IO_READ32(port)    (hal->io_read32(port))
#define HAL_IO_WRITE8(port, val)  (hal->io_write8(port, val))
#define HAL_IO_WRITE16(port, val) (hal->io_write16(port, val))
#define HAL_IO_WRITE32(port, val) (hal->io_write32(port, val))

#define HAL_IRQ_SAVE()         (hal->irq_save())
#define HAL_IRQ_RESTORE(flags) (hal->irq_restore(flags))

// Memory barriers for different architectures
#define HAL_MEMORY_BARRIER()   (hal->cpu_memory_barrier())
#define HAL_CPU_PAUSE()        (hal->cpu_pause())

// Version compatibility checking
static inline bool hal_is_api_compatible(uint32_t required_version) {
    return HAL_API_VERSION >= required_version;
}

#ifdef __cplusplus
}
#endif

#endif // HAL_INTERFACE_H