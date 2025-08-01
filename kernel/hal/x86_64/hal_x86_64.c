/**
 * @file hal_x86_64.c
 * @brief x86-64 Hardware Abstraction Layer Implementation
 * 
 * This module provides comprehensive x86-64 platform support including
 * CPU management, memory operations, interrupt handling, and hardware optimization.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#include "../../include/hal_interface.h"
#include "../../include/types.h"
#include "../../include/errno.h"
#include "../../include/memory_interface.h"
#include "../../pmm_production.h"
#include "../../vmm_production.h"
#include "hal_x86_64.h"
// Using types.h for kernel build
// Using types.h for kernel build

// x86-64 specific constants
// X86_64_PAGE_SIZE already defined in hal_x86_64.h
#define X86_64_CACHE_LINE_SIZE  64
#define MSR_IA32_TSC            0x10
#define MSR_IA32_APIC_BASE      0x1B
#define MSR_IA32_FEATURE_CONTROL 0x3A

// CPU feature flags
#define CPUID_FEAT_EDX_FPU      (1 << 0)
#define CPUID_FEAT_EDX_PSE      (1 << 3)
#define CPUID_FEAT_EDX_TSC      (1 << 4)
#define CPUID_FEAT_EDX_MSR      (1 << 5)
#define CPUID_FEAT_EDX_PAE      (1 << 6)
#define CPUID_FEAT_EDX_CX8      (1 << 8)
#define CPUID_FEAT_EDX_APIC     (1 << 9)
#define CPUID_FEAT_EDX_SEP      (1 << 11)
#define CPUID_FEAT_EDX_PGE      (1 << 13)
#define CPUID_FEAT_EDX_CMOV     (1 << 15)
#define CPUID_FEAT_EDX_CLFLUSH  (1 << 19)
#define CPUID_FEAT_EDX_MMX      (1 << 23)
#define CPUID_FEAT_EDX_FXSR     (1 << 24)
#define CPUID_FEAT_EDX_SSE      (1 << 25)
#define CPUID_FEAT_EDX_SSE2     (1 << 26)

#define CPUID_FEAT_ECX_SSE3     (1 << 0)
#define CPUID_FEAT_ECX_SSSE3    (1 << 9)
#define CPUID_FEAT_ECX_SSE4_1   (1 << 19)
#define CPUID_FEAT_ECX_SSE4_2   (1 << 20)
#define CPUID_FEAT_ECX_AES      (1 << 25)
#define CPUID_FEAT_ECX_AVX      (1 << 28)

// Global state
static struct {
    hal_cpu_features_t cpu_features;
    uint32_t num_cpus;
    bool apic_available;
    uint64_t tsc_frequency;
    uint64_t apic_base;
} x86_64_state = {0};

// Forward declarations
static void detect_cpu_features(void);
static void init_apic(void);
static uint64_t calibrate_tsc(void);
static void setup_smp(void);

// Assembly helper functions
// x86_64_cpu_pause defined below
extern void x86_64_memory_barrier(void);
extern uint64_t x86_64_read_tsc(void);
extern uint64_t x86_64_read_msr(uint32_t msr);
extern void x86_64_write_msr(uint32_t msr, uint64_t value);
extern void x86_64_cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
extern void x86_64_wbinvd(void);
extern void x86_64_invlpg(void* addr);

/**
 * x86-64 HAL Initialization
 */
static int x86_64_init(void)
{
    // Detect CPU features
    detect_cpu_features();
    
    // Calibrate TSC frequency
    x86_64_state.tsc_frequency = calibrate_tsc();
    
    // Initialize APIC if available
    if (x86_64_state.cpu_features.has_apic) {
        init_apic();
    }
    
    // Setup SMP if multiple CPUs detected
    if (x86_64_state.num_cpus > 1) {
        setup_smp();
    }
    
    return HAL_SUCCESS;
}

/**
 * x86-64 HAL Shutdown
 */
static void x86_64_shutdown(void)
{
    // Disable APIC
    if (x86_64_state.apic_available) {
        // Disable APIC in MSR
        uint64_t apic_base = x86_64_read_msr(MSR_IA32_APIC_BASE);
        apic_base &= ~(1ULL << 11); // Clear APIC enable bit
        x86_64_write_msr(MSR_IA32_APIC_BASE, apic_base);
    }
}

/**
 * CPU Management Operations
 */
static int x86_64_cpu_init(void)
{
    // CPU already initialized during main init
    return HAL_SUCCESS;
}

static void x86_64_cpu_idle(void)
{
    __asm__ volatile("hlt");
}

static void __attribute__((noreturn)) x86_64_cpu_halt(void)
{
    __asm__ volatile("cli; hlt");
    while (1) {
        __asm__ volatile("hlt");
    }
}

static uint64_t x86_64_cpu_timestamp(void)
{
    return x86_64_read_tsc();
}

void x86_64_cpu_pause(void)
{
    __asm__ volatile("pause" ::: "memory");
}

void x86_64_memory_barrier(void)
{
    __asm__ volatile("mfence" ::: "memory");
}

uint64_t x86_64_read_tsc(void)
{
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t x86_64_read_msr(uint32_t msr)
{
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void x86_64_write_msr(uint32_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

static void x86_64_cpu_memory_barrier(void)
{
    __asm__ volatile("mfence" ::: "memory");
}

static int x86_64_cpu_get_features(hal_cpu_features_t* features)
{
    if (!features) {
        return -EINVAL;
    }
    
    *features = x86_64_state.cpu_features;
    return HAL_SUCCESS;
}

/**
 * SMP Operations
 */
static int x86_64_smp_start_cpu(uint32_t cpu_id, void (*entry_point)(void))
{
    if (!x86_64_state.apic_available || !entry_point) {
        return -EINVAL;
    }
    
    // Implementation would use APIC to start secondary CPU
    // This is a complex process involving INIT-SIPI-SIPI sequence
    return HAL_ERR_NOT_SUPPORTED; // Placeholder
}

static uint32_t x86_64_smp_get_cpu_id(void)
{
    if (!x86_64_state.apic_available) {
        return 0;
    }
    
    // Read APIC ID from Local APIC
    return 0; // Placeholder
}

static uint32_t x86_64_smp_get_cpu_count(void)
{
    return x86_64_state.num_cpus;
}

static void x86_64_smp_send_ipi(uint32_t cpu_id, uint32_t vector)
{
    if (!x86_64_state.apic_available) {
        return;
    }
    
    // Send IPI via Local APIC
    // Implementation would write to APIC registers
}

/**
 * Memory Operations
 */
static void* x86_64_mem_alloc_pages(size_t pages, uint32_t flags)
{
    // Convert pages to order (log2 of pages)
    unsigned int order = 0;
    size_t size = pages;
    while (size > 1) {
        size >>= 1;
        order++;
    }
    
    // Convert HAL flags to PMM flags
    unsigned int pmm_flags = GFP_KERNEL;
    if (flags & HAL_MEM_ATOMIC) pmm_flags |= GFP_ATOMIC;
    if (flags & HAL_MEM_ZERO) pmm_flags |= GFP_ZERO;
    
    // Use existing PMM implementation with proper signature
    return pmm_alloc_pages(order, pmm_flags, -1); // Any NUMA node
}

static void x86_64_mem_free_pages(void* addr, size_t pages)
{
    // Convert pages to order (log2 of pages)
    unsigned int order = 0;
    size_t size = pages;
    while (size > 1) {
        size >>= 1;
        order++;
    }
    
    // Use existing PMM implementation with proper signature
    pmm_free_pages(addr, order);
}

static int x86_64_mem_map_physical(phys_addr_t phys, void* virt, size_t size, uint32_t flags)
{
    // Convert HAL flags to MM protection flags
    uint32_t prot = 0;
    if (flags & HAL_MEM_READ) prot |= MM_PROT_READ;
    if (flags & HAL_MEM_WRITE) prot |= MM_PROT_WRITE;
    if (flags & HAL_MEM_EXECUTE) prot |= MM_PROT_EXEC;
    if (flags & HAL_MEM_USER) prot |= MM_PROT_USER;
    if (flags & HAL_MEM_KERNEL) prot |= MM_PROT_KERNEL;
    if (flags & HAL_MEM_NOCACHE) prot |= MM_PROT_NOCACHE;
    
    // Use memory interface through operations table
    extern memory_operations_t* mem_ops;
    if (mem_ops && mem_ops->vmm_map) {
        return mem_ops->vmm_map(virt, phys, size, prot);
    }
    
    return -ENOSYS;
}

static int x86_64_mem_unmap(void* virt, size_t size)
{
    // Use memory interface through operations table
    extern memory_operations_t* mem_ops;
    if (mem_ops && mem_ops->vmm_unmap) {
        return mem_ops->vmm_unmap(virt, size);
    }
    
    return -ENOSYS;
}

static int x86_64_mem_protect(void* virt, size_t size, uint32_t flags)
{
    // Convert HAL flags to MM protection flags
    uint32_t prot = 0;
    if (flags & HAL_MEM_READ) prot |= MM_PROT_READ;
    if (flags & HAL_MEM_WRITE) prot |= MM_PROT_WRITE;
    if (flags & HAL_MEM_EXECUTE) prot |= MM_PROT_EXEC;
    if (flags & HAL_MEM_USER) prot |= MM_PROT_USER;
    if (flags & HAL_MEM_KERNEL) prot |= MM_PROT_KERNEL;
    
    // Use memory interface through operations table
    extern memory_operations_t* mem_ops;
    if (mem_ops && mem_ops->vmm_protect) {
        return mem_ops->vmm_protect(virt, size, prot);
    }
    
    return -ENOSYS;
}

static phys_addr_t x86_64_mem_virt_to_phys(void* virt)
{
    // Use memory interface through operations table
    extern memory_operations_t* mem_ops;
    if (mem_ops && mem_ops->vmm_virt_to_phys) {
        return mem_ops->vmm_virt_to_phys(virt);
    }
    
    return 0; // Invalid physical address
}

static void* x86_64_mem_phys_to_virt(phys_addr_t phys)
{
    // Use memory interface through operations table
    extern memory_operations_t* mem_ops;
    if (mem_ops && mem_ops->vmm_phys_to_virt) {
        return mem_ops->vmm_phys_to_virt(phys);
    }
    
    return NULL; // Invalid virtual address
}

static int x86_64_mem_get_regions(hal_memory_region_t* regions, size_t* count)
{
    // Get memory regions from PMM
    // This would interface with the multiboot memory map
    return HAL_ERR_NOT_SUPPORTED; // Placeholder
}

/**
 * Cache Operations
 */
static void x86_64_cache_flush_all(void)
{
    x86_64_wbinvd();
}

static void x86_64_cache_flush_range(void* start, size_t size)
{
    uintptr_t addr = (uintptr_t)start;
    uintptr_t end = addr + size;
    
    // Round down to cache line boundary
    addr &= ~(X86_64_CACHE_LINE_SIZE - 1);
    
    while (addr < end) {
        __asm__ volatile("clflush (%0)" :: "r"(addr) : "memory");
        addr += X86_64_CACHE_LINE_SIZE;
    }
    
    x86_64_memory_barrier();
}

static void x86_64_cache_invalidate_range(void* start, size_t size)
{
    // On x86-64, cache invalidation is the same as flush
    x86_64_cache_flush_range(start, size);
}

static void x86_64_cache_clean_range(void* start, size_t size)
{
    // On x86-64, cache clean is the same as flush
    x86_64_cache_flush_range(start, size);
}

/**
 * Interrupt Management
 */
static int x86_64_irq_init(void)
{
    // IRQ initialization is handled by the IDT implementation
    return HAL_SUCCESS;
}

static int x86_64_irq_register(int irq, hal_irq_handler_t handler, uint32_t flags, const char* name, void* data)
{
    // This would interface with the existing interrupt system
    return HAL_ERR_NOT_SUPPORTED; // Placeholder
}

static int x86_64_irq_unregister(int irq, void* data)
{
    return HAL_ERR_NOT_SUPPORTED; // Placeholder
}

static void x86_64_irq_enable(int irq)
{
    // Implementation would enable specific IRQ
}

static void x86_64_irq_disable(int irq)
{
    // Implementation would disable specific IRQ
}

static void x86_64_irq_mask(int irq)
{
    // Implementation would mask specific IRQ
}

static void x86_64_irq_unmask(int irq)
{
    // Implementation would unmask specific IRQ
}

static void x86_64_irq_end(int irq)
{
    // Send EOI to interrupt controller
}

static int x86_64_irq_get_pending(void)
{
    return 0; // Placeholder
}

static unsigned long x86_64_irq_save(void)
{
    unsigned long flags;
    __asm__ volatile("pushfq; popq %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

static void x86_64_irq_restore(unsigned long flags)
{
    __asm__ volatile("pushq %0; popfq" :: "r"(flags) : "memory");
}

/**
 * I/O Port Operations
 */
static uint8_t x86_64_io_read8(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static uint16_t x86_64_io_read16(uint16_t port)
{
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static uint32_t x86_64_io_read32(uint16_t port)
{
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void x86_64_io_write8(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" :: "a"(value), "Nd"(port));
}

static void x86_64_io_write16(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" :: "a"(value), "Nd"(port));
}

static void x86_64_io_write32(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" :: "a"(value), "Nd"(port));
}

/**
 * Memory-Mapped I/O Operations
 */
static uint8_t x86_64_mmio_read8(volatile void* addr)
{
    return *(volatile uint8_t*)addr;
}

static uint16_t x86_64_mmio_read16(volatile void* addr)
{
    return *(volatile uint16_t*)addr;
}

static uint32_t x86_64_mmio_read32(volatile void* addr)
{
    return *(volatile uint32_t*)addr;
}

static uint64_t x86_64_mmio_read64(volatile void* addr)
{
    return *(volatile uint64_t*)addr;
}

static void x86_64_mmio_write8(volatile void* addr, uint8_t value)
{
    *(volatile uint8_t*)addr = value;
}

static void x86_64_mmio_write16(volatile void* addr, uint16_t value)
{
    *(volatile uint16_t*)addr = value;
}

static void x86_64_mmio_write32(volatile void* addr, uint32_t value)
{
    *(volatile uint32_t*)addr = value;
}

static void x86_64_mmio_write64(volatile void* addr, uint64_t value)
{
    *(volatile uint64_t*)addr = value;
}

/**
 * Timer Operations
 */
static int x86_64_timer_init(void)
{
    return HAL_SUCCESS;
}

static uint64_t x86_64_timer_get_ticks(void)
{
    return x86_64_read_tsc();
}

static uint64_t x86_64_timer_get_frequency(void)
{
    return x86_64_state.tsc_frequency;
}

/**
 * Debug Support
 */
static void x86_64_debug_putchar(char c)
{
    // Use existing VGA or serial output
    x86_64_io_write8(0x3F8, c); // COM1 serial port
}

static char x86_64_debug_getchar(void)
{
    // Read from serial port
    while (!(x86_64_io_read8(0x3FD) & 1)) {
        // Wait for data
    }
    return x86_64_io_read8(0x3F8);
}

static void x86_64_debug_puts(const char* str)
{
    if (!str) return;
    while (*str) {
        x86_64_debug_putchar(*str++);
    }
}

static int x86_64_debug_early_init(void)
{
    // Initialize serial port for early debug output
    x86_64_io_write8(0x3F9, 0x00); // Disable interrupts
    x86_64_io_write8(0x3FB, 0x80); // Enable DLAB
    x86_64_io_write8(0x3F8, 0x03); // Divisor low (38400 baud)
    x86_64_io_write8(0x3F9, 0x00); // Divisor high
    x86_64_io_write8(0x3FB, 0x03); // 8N1, disable DLAB
    x86_64_io_write8(0x3FC, 0x0B); // Enable FIFO
    x86_64_io_write8(0x3FE, 0x0F); // Enable loopback
    
    return HAL_SUCCESS;
}

/**
 * x86-64 HAL Operations Structure
 */
static hal_operations_t x86_64_hal_ops = {
    .init = x86_64_init,
    .shutdown = x86_64_shutdown,
    
    // CPU Management
    .cpu_init = x86_64_cpu_init,
    .cpu_idle = x86_64_cpu_idle,
    .cpu_halt = x86_64_cpu_halt,
    .cpu_timestamp = x86_64_cpu_timestamp,
    .cpu_pause = x86_64_cpu_pause,
    .cpu_memory_barrier = x86_64_cpu_memory_barrier,
    .cpu_get_features = x86_64_cpu_get_features,
    
    // SMP Support
    .smp_start_cpu = x86_64_smp_start_cpu,
    .smp_get_cpu_id = x86_64_smp_get_cpu_id,
    .smp_get_cpu_count = x86_64_smp_get_cpu_count,
    .smp_send_ipi = x86_64_smp_send_ipi,
    
    // Memory Operations
    .mem_alloc_pages = x86_64_mem_alloc_pages,
    .mem_free_pages = x86_64_mem_free_pages,
    .mem_map_physical = x86_64_mem_map_physical,
    .mem_unmap = x86_64_mem_unmap,
    .mem_protect = x86_64_mem_protect,
    .mem_virt_to_phys = x86_64_mem_virt_to_phys,
    .mem_phys_to_virt = x86_64_mem_phys_to_virt,
    .mem_get_regions = x86_64_mem_get_regions,
    
    // Cache Operations
    .cache_flush_all = x86_64_cache_flush_all,
    .cache_flush_range = x86_64_cache_flush_range,
    .cache_invalidate_range = x86_64_cache_invalidate_range,
    .cache_clean_range = x86_64_cache_clean_range,
    
    // Interrupt Management
    .irq_init = x86_64_irq_init,
    .irq_register = x86_64_irq_register,
    .irq_unregister = x86_64_irq_unregister,
    .irq_enable = x86_64_irq_enable,
    .irq_disable = x86_64_irq_disable,
    .irq_mask = x86_64_irq_mask,
    .irq_unmask = x86_64_irq_unmask,
    .irq_end = x86_64_irq_end,
    .irq_get_pending = x86_64_irq_get_pending,
    .irq_save = x86_64_irq_save,
    .irq_restore = x86_64_irq_restore,
    
    // I/O Operations
    .io_read8 = x86_64_io_read8,
    .io_read16 = x86_64_io_read16,
    .io_read32 = x86_64_io_read32,
    .io_write8 = x86_64_io_write8,
    .io_write16 = x86_64_io_write16,
    .io_write32 = x86_64_io_write32,
    
    // Memory-Mapped I/O
    .mmio_read8 = x86_64_mmio_read8,
    .mmio_read16 = x86_64_mmio_read16,
    .mmio_read32 = x86_64_mmio_read32,
    .mmio_read64 = x86_64_mmio_read64,
    .mmio_write8 = x86_64_mmio_write8,
    .mmio_write16 = x86_64_mmio_write16,
    .mmio_write32 = x86_64_mmio_write32,
    .mmio_write64 = x86_64_mmio_write64,
    
    // Timer Operations
    .timer_init = x86_64_timer_init,
    .timer_get_ticks = x86_64_timer_get_ticks,
    .timer_get_frequency = x86_64_timer_get_frequency,
    
    // Debug Support
    .debug_putchar = x86_64_debug_putchar,
    .debug_getchar = x86_64_debug_getchar,
    .debug_puts = x86_64_debug_puts,
    .debug_early_init = x86_64_debug_early_init,
    
    .platform_data = &x86_64_state
};

/**
 * Initialize x86-64 HAL
 */
int hal_x86_64_init(hal_operations_t** ops)
{
    if (!ops) {
        return -EINVAL;
    }
    
    // Fill in any missing default implementations
    extern void hal_fill_defaults(hal_operations_t* ops);
    hal_fill_defaults(&x86_64_hal_ops);
    
    *ops = &x86_64_hal_ops;
    return HAL_SUCCESS;
}

/**
 * Helper Functions
 */

static void detect_cpu_features(void)
{
    hal_cpu_features_t* features = &x86_64_state.cpu_features;
    uint32_t eax, ebx, ecx, edx;
    
    // Basic CPU features
    features->has_mmu = true;
    features->has_atomic64 = true;
    features->page_size = X86_64_PAGE_SIZE;
    features->cache_line_size = X86_64_CACHE_LINE_SIZE;
    strcpy(features->arch_name, "x86_64");
    
    // CPUID leaf 1: Processor Info and Feature Bits
    x86_64_cpuid(1, &eax, &ebx, &ecx, &edx);
    
    features->has_fpu = (edx & CPUID_FEAT_EDX_FPU) != 0;
    features->has_simd = (edx & CPUID_FEAT_EDX_SSE) != 0;
    features->has_crypto = (ecx & CPUID_FEAT_ECX_AES) != 0;
    features->has_apic = (edx & CPUID_FEAT_EDX_APIC) != 0;
    
    // Check for virtualization support
    x86_64_cpuid(1, &eax, &ebx, &ecx, &edx);
    features->has_virtualization = (ecx & (1 << 5)) != 0; // VMX bit
    
    // Get number of logical processors
    x86_64_state.num_cpus = (ebx >> 16) & 0xFF;
    if (x86_64_state.num_cpus == 0) {
        x86_64_state.num_cpus = 1;
    }
    features->num_cores = x86_64_state.num_cpus;
}

static void init_apic(void)
{
    if (!x86_64_state.cpu_features.has_apic) {
        return;
    }
    
    // Read APIC base address from MSR
    x86_64_state.apic_base = x86_64_read_msr(MSR_IA32_APIC_BASE);
    x86_64_state.apic_available = (x86_64_state.apic_base & (1ULL << 11)) != 0;
}

static uint64_t calibrate_tsc(void)
{
    // Simple TSC calibration using PIT
    // This is a simplified implementation
    return 2000000000ULL; // Assume 2 GHz for now
}

static void setup_smp(void)
{
    // SMP setup would be implemented here
    // This involves APIC configuration and CPU startup
}