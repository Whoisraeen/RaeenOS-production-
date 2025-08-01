/**
 * @file hal_device_manager.c
 * @brief HAL Device Management System
 * 
 * This module provides unified device discovery, enumeration, and management
 * across different hardware platforms and bus types (PCI, USB, etc.).
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#include "../../include/hal_interface.h"
#include "../../include/types.h"
#include "../../include/errno.h"
#include "../../pmm_production.h"
#include "../../vmm_production.h"
#include "hal_device_manager.h"
#include <stddef.h>
#include <string.h>

// Maximum number of devices and buses
#define MAX_DEVICES             1024
#define MAX_BUSES               64
#define MAX_DEVICE_CLASSES      32
#define MAX_VENDORS             256

// Device registry
static struct {
    hal_device_t devices[MAX_DEVICES];
    hal_bus_t buses[MAX_BUSES];
    hal_device_class_t device_classes[MAX_DEVICE_CLASSES];
    hal_vendor_info_t vendors[MAX_VENDORS];
    
    size_t device_count;
    size_t bus_count;
    size_t class_count;
    size_t vendor_count;
    
    bool initialized;
    
    // Device discovery callbacks
    hal_device_discovery_callback_t discovery_callbacks[16];
    size_t discovery_callback_count;
    
    // Device state change callbacks
    hal_device_state_callback_t state_callbacks[16];
    size_t state_callback_count;
} device_manager = {0};

// Forward declarations
static int discover_pci_devices(void);
static int discover_acpi_devices(void);
static int discover_dt_devices(void);
static int discover_platform_devices(void);
static hal_device_t* allocate_device(void);
static hal_bus_t* allocate_bus(void);
static int classify_device(hal_device_t* device);
static void notify_device_discovery(hal_device_t* device);
static void notify_device_state_change(hal_device_t* device, hal_device_state_t old_state);

/**
 * Initialize the HAL device management system
 */
int hal_device_manager_init(void)
{
    if (device_manager.initialized) {
        return HAL_SUCCESS;
    }
    
    // Initialize builtin device classes
    init_builtin_device_classes();
    
    // Initialize builtin vendor database
    init_builtin_vendor_database();
    
    // Discover devices based on platform
    hal_arch_t arch = hal_get_architecture();
    
    switch (arch) {
        case HAL_ARCH_X86_64:
            // x86-64 platforms typically use PCI and ACPI
            discover_pci_devices();
            discover_acpi_devices();
            break;
            
        case HAL_ARCH_ARM64:
            // ARM64 platforms typically use device tree
            discover_dt_devices();
            discover_platform_devices();
            break;
            
        default:
            // Try all discovery methods
            discover_pci_devices();
            discover_acpi_devices();
            discover_dt_devices();
            discover_platform_devices();
            break;
    }
    
    device_manager.initialized = true;
    return HAL_SUCCESS;
}

/**
 * Shutdown the device management system
 */
void hal_device_manager_shutdown(void)
{
    if (!device_manager.initialized) {
        return;
    }
    
    // Notify all devices of shutdown
    for (size_t i = 0; i < device_manager.device_count; i++) {
        hal_device_t* device = &device_manager.devices[i];
        if (device->state == HAL_DEVICE_STATE_ACTIVE) {
            device->state = HAL_DEVICE_STATE_SHUTTING_DOWN;
            notify_device_state_change(device, HAL_DEVICE_STATE_ACTIVE);
        }
    }
    
    device_manager.initialized = false;
}

/**
 * Register a device discovery callback
 */
int hal_device_register_discovery_callback(hal_device_discovery_callback_t callback)
{
    if (!callback || device_manager.discovery_callback_count >= 16) {
        return -EINVAL;
    }
    
    device_manager.discovery_callbacks[device_manager.discovery_callback_count++] = callback;
    return HAL_SUCCESS;
}

/**
 * Register a device state change callback
 */
int hal_device_register_state_callback(hal_device_state_callback_t callback)
{
    if (!callback || device_manager.state_callback_count >= 16) {
        return -EINVAL;
    }
    
    device_manager.state_callbacks[device_manager.state_callback_count++] = callback;
    return HAL_SUCCESS;
}

/**
 * Find devices by class
 */
int hal_device_find_by_class(uint32_t class_id, hal_device_t** devices, size_t* count)
{
    if (!devices || !count) {
        return -EINVAL;
    }
    
    size_t found = 0;
    size_t max_count = *count;
    
    for (size_t i = 0; i < device_manager.device_count && found < max_count; i++) {
        if (device_manager.devices[i].class_id == class_id) {
            devices[found++] = &device_manager.devices[i];
        }
    }
    
    *count = found;
    return HAL_SUCCESS;
}

/**
 * Find devices by vendor
 */
int hal_device_find_by_vendor(uint32_t vendor_id, hal_device_t** devices, size_t* count)
{
    if (!devices || !count) {
        return -EINVAL;
    }
    
    size_t found = 0;
    size_t max_count = *count;
    
    for (size_t i = 0; i < device_manager.device_count && found < max_count; i++) {
        if (device_manager.devices[i].vendor_id == vendor_id) {
            devices[found++] = &device_manager.devices[i];
        }
    }
    
    *count = found;
    return HAL_SUCCESS;
}

/**
 * Find device by ID
 */
hal_device_t* hal_device_find_by_id(uint32_t device_id)
{
    for (size_t i = 0; i < device_manager.device_count; i++) {
        if (device_manager.devices[i].device_id == device_id) {
            return &device_manager.devices[i];
        }
    }
    return NULL;
}

/**
 * Get device information
 */
int hal_device_get_info(hal_device_t* device, hal_device_info_t* info)
{
    if (!device || !info) {
        return -EINVAL;
    }
    
    info->device_id = device->device_id;
    info->vendor_id = device->vendor_id;
    info->class_id = device->class_id;
    info->subclass_id = device->subclass_id;
    info->revision = device->revision;
    info->state = device->state;
    info->bus_type = device->bus_type;
    info->bus_address = device->bus_address;
    info->base_addr = device->base_addr;
    info->mem_size = device->mem_size;
    info->irq = device->irq;
    
    strncpy(info->name, device->name, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    
    return HAL_SUCCESS;
}

/**
 * Set device state
 */
int hal_device_set_state(hal_device_t* device, hal_device_state_t new_state)
{
    if (!device) {
        return -EINVAL;
    }
    
    hal_device_state_t old_state = device->state;
    device->state = new_state;
    
    notify_device_state_change(device, old_state);
    return HAL_SUCCESS;
}

/**
 * Map device memory
 */
int hal_device_map_memory(hal_device_t* device, void** virt_addr)
{
    if (!device || !virt_addr || device->mem_size == 0) {
        return -EINVAL;
    }
    
    // Map device memory as non-cacheable device memory
    uint32_t flags = HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_DEVICE | HAL_MEM_NOCACHE;
    
    // Allocate virtual address space
    void* virt = vmm_alloc_kernel_space(device->mem_size);
    if (!virt) {
        return -ENOMEM;
    }
    
    // Map physical to virtual
    int result = hal->mem_map_physical(device->base_addr, virt, device->mem_size, flags);
    if (result != HAL_SUCCESS) {
        vmm_free_kernel_space(virt, device->mem_size);
        return result;
    }
    
    *virt_addr = virt;
    return HAL_SUCCESS;
}

/**
 * Unmap device memory
 */
int hal_device_unmap_memory(hal_device_t* device, void* virt_addr)
{
    if (!device || !virt_addr || device->mem_size == 0) {
        return -EINVAL;
    }
    
    hal->mem_unmap(virt_addr, device->mem_size);
    vmm_free_kernel_space(virt_addr, device->mem_size);
    
    return HAL_SUCCESS;
}

/**
 * Enable device interrupts
 */
int hal_device_enable_interrupts(hal_device_t* device)
{
    if (!device || device->irq < 0) {
        return -EINVAL;
    }
    
    hal->irq_enable(device->irq);
    return HAL_SUCCESS;
}

/**
 * Disable device interrupts
 */
int hal_device_disable_interrupts(hal_device_t* device)
{
    if (!device || device->irq < 0) {
        return -EINVAL;
    }
    
    hal->irq_disable(device->irq);
    return HAL_SUCCESS;
}

/**
 * Get all devices
 */
int hal_device_get_all(hal_device_t** devices, size_t* count)
{
    if (!devices || !count) {
        return -EINVAL;
    }
    
    size_t max_count = *count;
    size_t actual_count = device_manager.device_count < max_count ? 
                         device_manager.device_count : max_count;
    
    for (size_t i = 0; i < actual_count; i++) {
        devices[i] = &device_manager.devices[i];
    }
    
    *count = actual_count;
    return HAL_SUCCESS;
}

/**
 * PCI device discovery
 */
static int discover_pci_devices(void)
{
    // External PCI functions
    extern int pci_enumerate_devices(void);
    extern int pci_get_device_count(void);
    extern int pci_get_device_info(int index, uint8_t* bus, uint8_t* device, uint8_t* function);
    extern uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
    
    int pci_available = pci_enumerate_devices();
    if (pci_available != 0) {
        return HAL_ERR_NOT_SUPPORTED;
    }
    
    int pci_device_count = pci_get_device_count();
    
    for (int i = 0; i < pci_device_count; i++) {
        uint8_t bus, device, function;
        if (pci_get_device_info(i, &bus, &device, &function) != 0) {
            continue;
        }
        
        // Read PCI configuration space
        uint32_t vendor_device = pci_read_config_dword(bus, device, function, 0x00);
        uint32_t class_rev = pci_read_config_dword(bus, device, function, 0x08);
        uint32_t bar0 = pci_read_config_dword(bus, device, function, 0x10);
        uint32_t interrupt = pci_read_config_dword(bus, device, function, 0x3C);
        
        uint16_t vendor_id = vendor_device & 0xFFFF;
        uint16_t device_id = (vendor_device >> 16) & 0xFFFF;
        uint32_t class_code = (class_rev >> 24) & 0xFF;
        uint32_t subclass = (class_rev >> 16) & 0xFF;
        uint32_t revision = class_rev & 0xFF;
        uint8_t irq_line = interrupt & 0xFF;
        
        // Skip invalid devices
        if (vendor_id == 0xFFFF || device_id == 0xFFFF) {
            continue;
        }
        
        // Allocate device structure
        hal_device_t* hal_device = allocate_device();
        if (!hal_device) {
            continue;
        }
        
        // Fill device information
        hal_device->device_id = device_id;
        hal_device->vendor_id = vendor_id;
        hal_device->class_id = class_code;
        hal_device->subclass_id = subclass;
        hal_device->revision = revision;
        hal_device->state = HAL_DEVICE_STATE_DISCOVERED;
        hal_device->bus_type = HAL_BUS_TYPE_PCI;
        hal_device->bus_address = (bus << 16) | (device << 8) | function;
        hal_device->irq = (irq_line != 0xFF) ? irq_line : -1;
        
        // Decode BAR0 for memory mapping
        if (bar0 & 0x1) {
            // I/O space
            hal_device->base_addr = bar0 & 0xFFFFFFFC;
            hal_device->mem_size = 0; // I/O ports don't have size in BAR
        } else {
            // Memory space
            hal_device->base_addr = bar0 & 0xFFFFFFF0;
            // Size determination would require writing to BAR and reading back
            hal_device->mem_size = 0x1000; // Default 4KB, should be determined properly
        }
        
        // Generate device name
        snprintf(hal_device->name, sizeof(hal_device->name), 
                "pci_%04x_%04x", vendor_id, device_id);
        
        // Classify device
        classify_device(hal_device);
        
        // Notify discovery
        notify_device_discovery(hal_device);
    }
    
    return HAL_SUCCESS;
}

/**
 * ACPI device discovery
 */
static int discover_acpi_devices(void)
{
    // External ACPI functions
    extern int acpi_is_available(void);
    extern int acpi_enumerate_devices(void);
    
    if (!acpi_is_available()) {
        return HAL_ERR_NOT_SUPPORTED;
    }
    
    return acpi_enumerate_devices();
}

/**
 * Device Tree device discovery
 */
static int discover_dt_devices(void)
{
    // Device tree discovery for ARM64 platforms
    if (hal_get_architecture() != HAL_ARCH_ARM64) {
        return HAL_ERR_NOT_SUPPORTED;
    }
    
    // Implementation would parse device tree and create device entries
    return HAL_ERR_NOT_SUPPORTED; // Placeholder
}

/**
 * Platform device discovery
 */
static int discover_platform_devices(void)
{
    // Platform-specific device discovery
    // This could include built-in devices, legacy devices, etc.
    return HAL_SUCCESS;
}

/**
 * Allocate a device structure
 */
static hal_device_t* allocate_device(void)
{
    if (device_manager.device_count >= MAX_DEVICES) {
        return NULL;
    }
    
    hal_device_t* device = &device_manager.devices[device_manager.device_count++];
    memset(device, 0, sizeof(hal_device_t));
    
    return device;
}

/**
 * Allocate a bus structure
 */
static hal_bus_t* allocate_bus(void)
{
    if (device_manager.bus_count >= MAX_BUSES) {
        return NULL;
    }
    
    hal_bus_t* bus = &device_manager.buses[device_manager.bus_count++];
    memset(bus, 0, sizeof(hal_bus_t));
    
    return bus;
}

/**
 * Classify a device based on its IDs
 */
static int classify_device(hal_device_t* device)
{
    // This would use a comprehensive database to classify devices
    // For now, use basic PCI class codes
    
    switch (device->class_id) {
        case 0x01: // Storage controller
            if (device->subclass_id == 0x01) {
                strcpy(device->class_name, "IDE Controller");
            } else if (device->subclass_id == 0x06) {
                strcpy(device->class_name, "SATA Controller");
            } else if (device->subclass_id == 0x08) {
                strcpy(device->class_name, "NVMe Controller");
            } else {
                strcpy(device->class_name, "Storage Controller");
            }
            break;
            
        case 0x02: // Network controller
            strcpy(device->class_name, "Network Controller");
            break;
            
        case 0x03: // Display controller
            strcpy(device->class_name, "Graphics Controller");
            break;
            
        case 0x04: // Multimedia controller
            strcpy(device->class_name, "Audio Controller");
            break;
            
        case 0x0C: // Serial bus controller
            if (device->subclass_id == 0x03) {
                strcpy(device->class_name, "USB Controller");
            } else {
                strcpy(device->class_name, "Serial Bus Controller");
            }
            break;
            
        default:
            strcpy(device->class_name, "Unknown Device");
            break;
    }
    
    return HAL_SUCCESS;
}

/**
 * Notify device discovery callbacks
 */
static void notify_device_discovery(hal_device_t* device)
{
    for (size_t i = 0; i < device_manager.discovery_callback_count; i++) {
        device_manager.discovery_callbacks[i](device);
    }
}

/**
 * Notify device state change callbacks
 */
static void notify_device_state_change(hal_device_t* device, hal_device_state_t old_state)
{
    for (size_t i = 0; i < device_manager.state_callback_count; i++) {
        device_manager.state_callbacks[i](device, old_state, device->state);
    }
}

/**
 * Initialize builtin device classes
 */
void init_builtin_device_classes(void)
{
    // Initialize common device classes
    struct {
        uint32_t class_id;
        const char* name;
        const char* description;
    } classes[] = {
        {0x01, "Storage", "Mass Storage Controller"},
        {0x02, "Network", "Network Controller"},
        {0x03, "Display", "Display Controller"},
        {0x04, "Multimedia", "Multimedia Controller"},
        {0x05, "Memory", "Memory Controller"},
        {0x06, "Bridge", "Bridge Device"},
        {0x07, "Communication", "Communication Controller"},
        {0x08, "System", "Generic System Peripheral"},
        {0x09, "Input", "Input Device Controller"},
        {0x0A, "Docking", "Docking Station"},
        {0x0B, "Processor", "Processor"},
        {0x0C, "Serial", "Serial Bus Controller"},
        {0x0D, "Wireless", "Wireless Controller"},
        {0x0E, "Intelligent", "Intelligent Controller"},
        {0x0F, "Satellite", "Satellite Communication Controller"},
        {0x10, "Encryption", "Encryption Controller"},
        {0x11, "Signal", "Signal Processing Controller"}
    };
    
    size_t num_classes = sizeof(classes) / sizeof(classes[0]);
    
    for (size_t i = 0; i < num_classes && device_manager.class_count < MAX_DEVICE_CLASSES; i++) {
        hal_device_class_t* class = &device_manager.device_classes[device_manager.class_count++];
        class->class_id = classes[i].class_id;
        strncpy(class->name, classes[i].name, sizeof(class->name) - 1);
        strncpy(class->description, classes[i].description, sizeof(class->description) - 1);
    }
}

/**
 * Initialize builtin vendor database
 */
void init_builtin_vendor_database(void)
{
    // Initialize common vendors
    struct {
        uint32_t vendor_id;
        const char* name;
    } vendors[] = {
        {0x8086, "Intel Corporation"},
        {0x1022, "Advanced Micro Devices"},
        {0x10DE, "NVIDIA Corporation"},
        {0x1002, "Advanced Micro Devices (ATI)"},
        {0x14E4, "Broadcom Corporation"},
        {0x168C, "Qualcomm Atheros"},
        {0x8EC8, "Realtek Semiconductor"},
        {0x1106, "VIA Technologies"},
        {0x1B21, "ASMedia Technology"},
        {0x1912, "Renesas Technology"},
        {0x104C, "Texas Instruments"},
        {0x11AB, "Marvell Technology Group"},
        {0x1969, "Qualcomm Atheros Communications"},
        {0x15B3, "Mellanox Technologies"},
        {0x1AF4, "Red Hat, Inc."}
    };
    
    size_t num_vendors = sizeof(vendors) / sizeof(vendors[0]);
    
    for (size_t i = 0; i < num_vendors && device_manager.vendor_count < MAX_VENDORS; i++) {
        hal_vendor_info_t* vendor = &device_manager.vendors[device_manager.vendor_count++];
        vendor->vendor_id = vendors[i].vendor_id;
        strncpy(vendor->name, vendors[i].name, sizeof(vendor->name) - 1);
    }
}