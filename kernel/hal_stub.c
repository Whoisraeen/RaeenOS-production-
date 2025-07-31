/**
 * @file hal_stub.c
 * @brief Hardware Abstraction Layer Stub Implementation
 * 
 * This provides stub implementations for HAL functions used by the kernel.
 * In a production system, these would be implemented per-architecture.
 * 
 * @version 1.0
 * @date 2025-07-31
 */

#include "include/hal_interface.h"
#include "include/types.h"

// Stub HAL operations
static int stub_cpu_init(void) {
    return 0;
}

static void stub_cpu_idle(void) {
    __asm__ volatile("hlt");
}

static void stub_cpu_halt(void) {
    __asm__ volatile("hlt");
}

static uint64_t stub_cpu_timestamp(void) {
    // Simple counter for now
    static uint64_t counter = 0;
    return ++counter;
}

static void* stub_mem_alloc_pages(size_t pages) {
    // This would interact with the PMM
    return NULL;
}

static void stub_mem_free_pages(void* addr, size_t pages) {
    // This would interact with the PMM
}

static int stub_mem_map_physical(uint64_t phys, uint64_t virt, size_t size, uint32_t flags) {
    // This would set up page tables
    return 0;
}

static int stub_irq_register(int irq, void (*handler)(void)) {
    // This would set up interrupt handlers
    return 0;
}

static void stub_irq_enable(int irq) {
    // This would enable specific interrupt
}

static void stub_irq_disable(int irq) {
    // This would disable specific interrupt
}

static uint8_t stub_io_read8(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static uint16_t stub_io_read16(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static uint32_t stub_io_read32(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void stub_io_write8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void stub_io_write16(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static void stub_io_write32(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Global HAL operations structure
static hal_ops_t stub_hal_ops = {
    .cpu_init = stub_cpu_init,
    .cpu_idle = stub_cpu_idle,
    .cpu_halt = stub_cpu_halt,
    .cpu_timestamp = stub_cpu_timestamp,
    .mem_alloc_pages = stub_mem_alloc_pages,
    .mem_free_pages = stub_mem_free_pages,
    .mem_map_physical = stub_mem_map_physical,
    .irq_register = stub_irq_register,
    .irq_enable = stub_irq_enable,
    .irq_disable = stub_irq_disable,
    .io_read8 = stub_io_read8,
    .io_read16 = stub_io_read16,
    .io_read32 = stub_io_read32,
    .io_write8 = stub_io_write8,
    .io_write16 = stub_io_write16,
    .io_write32 = stub_io_write32,
    .platform_data = NULL
};

// Global HAL pointer
hal_ops_t* hal = &stub_hal_ops;