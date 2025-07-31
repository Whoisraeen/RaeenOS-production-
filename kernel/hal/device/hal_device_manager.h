/**
 * @file hal_device_manager.h
 * @brief HAL Device Management System Header
 * 
 * This header defines structures and functions for unified device discovery,
 * enumeration, and management across different hardware platforms.
 * 
 * @version 1.0
 * @date 2025-07-31
 * @author RaeenOS Development Team
 */

#ifndef HAL_DEVICE_MANAGER_H
#define HAL_DEVICE_MANAGER_H

#include "../../include/hal_interface.h"
#include "../../include/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Device states
typedef enum {
    HAL_DEVICE_STATE_UNKNOWN,
    HAL_DEVICE_STATE_DISCOVERED,
    HAL_DEVICE_STATE_INITIALIZING,
    HAL_DEVICE_STATE_ACTIVE,
    HAL_DEVICE_STATE_SUSPENDED,
    HAL_DEVICE_STATE_ERROR,
    HAL_DEVICE_STATE_SHUTTING_DOWN,
    HAL_DEVICE_STATE_REMOVED
} hal_device_state_t;

// Bus types
typedef enum {
    HAL_BUS_TYPE_UNKNOWN,
    HAL_BUS_TYPE_PCI,
    HAL_BUS_TYPE_PCIE,
    HAL_BUS_TYPE_USB,
    HAL_BUS_TYPE_I2C,
    HAL_BUS_TYPE_SPI,
    HAL_BUS_TYPE_UART,
    HAL_BUS_TYPE_PLATFORM,
    HAL_BUS_TYPE_DEVICE_TREE,
    HAL_BUS_TYPE_ACPI,
    HAL_BUS_TYPE_ISA,
    HAL_BUS_TYPE_MEMORY_MAPPED
} hal_bus_type_t;

// Power states
typedef enum {
    HAL_POWER_STATE_D0,    // Fully on
    HAL_POWER_STATE_D1,    // Low power
    HAL_POWER_STATE_D2,    // Lower power
    HAL_POWER_STATE_D3_HOT, // Off, but can be awakened
    HAL_POWER_STATE_D3_COLD // Off, cannot be awakened
} hal_power_state_t;

// Device capability flags
#define HAL_DEVICE_CAP_DMA              (1 << 0)
#define HAL_DEVICE_CAP_BUS_MASTER       (1 << 1)
#define HAL_DEVICE_CAP_POWER_MGMT       (1 << 2)
#define HAL_DEVICE_CAP_MSI              (1 << 3)
#define HAL_DEVICE_CAP_MSIX             (1 << 4)
#define HAL_DEVICE_CAP_64BIT_ADDR       (1 << 5)
#define HAL_DEVICE_CAP_HOTPLUG          (1 << 6)
#define HAL_DEVICE_CAP_WAKE             (1 << 7)
#define HAL_DEVICE_CAP_RESET            (1 << 8)
#define HAL_DEVICE_CAP_VIRTUALIZATION   (1 << 9)

// Extended device structure with additional fields
typedef struct hal_device_extended {
    // Base device information (from hal_interface.h)
    char name[64];
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t class_id;
    uint32_t subclass_id;
    uint32_t revision;
    phys_addr_t base_addr;
    size_t mem_size;
    int irq;
    void* private_data;
    
    // Extended information
    hal_device_state_t state;
    hal_bus_type_t bus_type;
    uint32_t bus_address;
    char class_name[32];
    char vendor_name[64];
    
    // Capabilities and features
    uint32_t capabilities;
    hal_power_state_t power_state;
    uint32_t max_power_consumption; // in mW
    
    // Resource information
    struct {
        phys_addr_t base;
        size_t size;
        uint32_t flags;
    } memory_regions[6]; // Up to 6 BARs for PCI devices
    
    struct {
        uint16_t base;
        uint16_t size;
    } io_regions[6];
    
    int irq_vectors[32]; // Support for MSI-X
    size_t irq_count;
    
    // Parent/child relationships
    struct hal_device_extended* parent;
    struct hal_device_extended* children[16];
    size_t child_count;
    
    // Driver binding
    void* driver;
    char driver_name[32];
    
    // Reference counting
    uint32_t ref_count;
    
    // Synchronization
    void* lock; // Would be a mutex in real implementation
    
    // Platform-specific data
    union {
        struct {
            uint8_t bus;
            uint8_t device;
            uint8_t function;
            uint16_t domain;
        } pci;
        
        struct {
            uint8_t hub_port;
            uint8_t device_address;
            uint16_t device_descriptor;
        } usb;
        
        struct {
            uint32_t reg_offset;
            uint32_t reg_size;
            char compatible[64];
        } device_tree;
        
        struct {
            char hid[16];
            char uid[16];
            uint32_t adr;
        } acpi;
    } bus_info;
} hal_device_extended_t;

// Use extended device as the main device type
typedef hal_device_extended_t hal_device_t;

// Bus information structure
typedef struct {
    uint32_t bus_id;
    hal_bus_type_t type;
    char name[32];
    uint32_t max_devices;
    uint32_t current_devices;
    uint32_t capabilities;
    
    // Bus-specific operations
    int (*scan_devices)(void);
    int (*add_device)(hal_device_t* device);
    int (*remove_device)(hal_device_t* device);
    int (*configure_device)(hal_device_t* device);
    
    void* private_data;
} hal_bus_t;

// Device class information
typedef struct {
    uint32_t class_id;
    char name[32];
    char description[128];
    uint32_t device_count;
} hal_device_class_t;

// Vendor information
typedef struct {
    uint32_t vendor_id;
    char name[64];
    char short_name[16];
    uint32_t device_count;
} hal_vendor_info_t;

// Device information structure for queries
typedef struct {
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t class_id;
    uint32_t subclass_id;
    uint32_t revision;
    hal_device_state_t state;
    hal_bus_type_t bus_type;
    uint32_t bus_address;
    phys_addr_t base_addr;
    size_t mem_size;
    int irq;
    char name[64];
    char class_name[32];
    char vendor_name[64];
    uint32_t capabilities;
} hal_device_info_t;

// Callback function types
typedef void (*hal_device_discovery_callback_t)(hal_device_t* device);
typedef void (*hal_device_state_callback_t)(hal_device_t* device, hal_device_state_t old_state, hal_device_state_t new_state);
typedef int (*hal_device_driver_probe_t)(hal_device_t* device);
typedef void (*hal_device_driver_remove_t)(hal_device_t* device);

// Device driver structure
typedef struct {
    char name[32];
    char description[128];
    uint32_t version;
    
    // Supported devices (vendor:device pairs or class matches)
    struct {
        uint32_t vendor_id;
        uint32_t device_id;
        uint32_t class_mask;
        uint32_t class_value;
    } id_table[32];
    size_t id_count;
    
    // Driver operations
    hal_device_driver_probe_t probe;
    hal_device_driver_remove_t remove;
    
    int (*suspend)(hal_device_t* device, hal_power_state_t state);
    int (*resume)(hal_device_t* device);
    int (*reset)(hal_device_t* device);
    
    void* private_data;
} hal_device_driver_t;

// Function prototypes

// Device manager initialization
int hal_device_manager_init(void);
void hal_device_manager_shutdown(void);

// Device discovery and enumeration
int hal_device_rescan(void);
int hal_device_rescan_bus(hal_bus_type_t bus_type);

// Device lookup functions
hal_device_t* hal_device_find_by_id(uint32_t device_id);
int hal_device_find_by_class(uint32_t class_id, hal_device_t** devices, size_t* count);
int hal_device_find_by_vendor(uint32_t vendor_id, hal_device_t** devices, size_t* count);
int hal_device_find_by_name(const char* name, hal_device_t** devices, size_t* count);
int hal_device_get_all(hal_device_t** devices, size_t* count);

// Device information
int hal_device_get_info(hal_device_t* device, hal_device_info_t* info);
const char* hal_device_get_class_name(uint32_t class_id);
const char* hal_device_get_vendor_name(uint32_t vendor_id);
const char* hal_device_state_to_string(hal_device_state_t state);
const char* hal_bus_type_to_string(hal_bus_type_t bus_type);

// Device state management
int hal_device_set_state(hal_device_t* device, hal_device_state_t new_state);
hal_device_state_t hal_device_get_state(hal_device_t* device);

// Device resource management
int hal_device_map_memory(hal_device_t* device, void** virt_addr);
int hal_device_unmap_memory(hal_device_t* device, void* virt_addr);
int hal_device_map_memory_region(hal_device_t* device, int region, void** virt_addr);
int hal_device_unmap_memory_region(hal_device_t* device, int region, void* virt_addr);

// Interrupt management
int hal_device_enable_interrupts(hal_device_t* device);
int hal_device_disable_interrupts(hal_device_t* device);
int hal_device_register_interrupt(hal_device_t* device, hal_irq_handler_t handler, void* data);
int hal_device_unregister_interrupt(hal_device_t* device);

// Power management
int hal_device_set_power_state(hal_device_t* device, hal_power_state_t state);
hal_power_state_t hal_device_get_power_state(hal_device_t* device);
int hal_device_suspend(hal_device_t* device);
int hal_device_resume(hal_device_t* device);

// DMA support
int hal_device_enable_dma(hal_device_t* device);
int hal_device_disable_dma(hal_device_t* device);
hal_dma_buffer_t* hal_device_alloc_dma_buffer(hal_device_t* device, size_t size, uint32_t flags);
void hal_device_free_dma_buffer(hal_device_t* device, hal_dma_buffer_t* buffer);

// Device driver management
int hal_device_register_driver(hal_device_driver_t* driver);
int hal_device_unregister_driver(hal_device_driver_t* driver);
int hal_device_bind_driver(hal_device_t* device, hal_device_driver_t* driver);
int hal_device_unbind_driver(hal_device_t* device);

// Bus management
int hal_bus_register(hal_bus_t* bus);
int hal_bus_unregister(hal_bus_t* bus);
hal_bus_t* hal_bus_find_by_type(hal_bus_type_t type);
int hal_bus_get_all(hal_bus_t** buses, size_t* count);

// Callback registration
int hal_device_register_discovery_callback(hal_device_discovery_callback_t callback);
int hal_device_unregister_discovery_callback(hal_device_discovery_callback_t callback);
int hal_device_register_state_callback(hal_device_state_callback_t callback);
int hal_device_unregister_state_callback(hal_device_state_callback_t callback);

// Reference counting
void hal_device_get(hal_device_t* device);
void hal_device_put(hal_device_t* device);

// Device tree functions (for ARM64)
int hal_device_dt_parse(void* dt_base);
hal_device_t* hal_device_dt_find_by_compatible(const char* compatible);
hal_device_t* hal_device_dt_find_by_path(const char* path);

// ACPI functions (for x86-64)
int hal_device_acpi_enumerate(void);
hal_device_t* hal_device_acpi_find_by_hid(const char* hid);

// Platform device functions
int hal_device_platform_register(const char* name, phys_addr_t base, size_t size, int irq);
int hal_device_platform_unregister(const char* name);

// Utility functions
bool hal_device_is_compatible(hal_device_t* device, uint32_t vendor_id, uint32_t device_id);
bool hal_device_has_capability(hal_device_t* device, uint32_t capability);
int hal_device_enable_capability(hal_device_t* device, uint32_t capability);
int hal_device_disable_capability(hal_device_t* device, uint32_t capability);

// Debug and diagnostic functions
void hal_device_dump_info(hal_device_t* device);
void hal_device_dump_all(void);
int hal_device_get_statistics(hal_device_info_t* stats);

// Internal functions (exposed for platform-specific implementations)
void init_builtin_device_classes(void);
void init_builtin_vendor_database(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_DEVICE_MANAGER_H