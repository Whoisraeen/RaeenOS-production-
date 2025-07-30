#ifndef DRIVER_FRAMEWORK_H
#define DRIVER_FRAMEWORK_H

/**
 * @file driver_framework.h
 * @brief Comprehensive Driver Framework Interface for RaeenOS
 * 
 * This interface defines the standardized driver API for all hardware types
 * in RaeenOS, providing a unified framework for driver development and management.
 * 
 * Version: 1.0
 * API Version: 1
 */

#include "types.h"
#include "errno.h"
#include "hal_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Driver framework API version
#define DRIVER_API_VERSION 1

// Driver name and description limits
#define DRIVER_NAME_MAX     64
#define DRIVER_DESC_MAX     256
#define DRIVER_AUTHOR_MAX   128
#define DRIVER_LICENSE_MAX  32
#define DRIVER_VERSION_MAX  16

// Device ID constants
#define DEVICE_ID_ANY       0xFFFFFFFF
#define VENDOR_ID_ANY       0xFFFFFFFF
#define CLASS_ID_ANY        0xFFFFFFFF

// Maximum number of dependencies and resources
#define MAX_DEPENDENCIES    16
#define MAX_RESOURCES       8
#define MAX_IRQ_HANDLERS    8

// Driver types
typedef enum {
    DRIVER_TYPE_UNKNOWN,
    DRIVER_TYPE_BUS,           // Bus controllers (PCI, USB, etc.)
    DRIVER_TYPE_STORAGE,       // Storage devices (SATA, NVMe, etc.)
    DRIVER_TYPE_NETWORK,       // Network interfaces
    DRIVER_TYPE_AUDIO,         // Audio devices
    DRIVER_TYPE_VIDEO,         // Video/Graphics devices
    DRIVER_TYPE_INPUT,         // Input devices (keyboard, mouse, etc.)
    DRIVER_TYPE_SENSOR,        // Sensors and measurement devices
    DRIVER_TYPE_POWER,         // Power management devices
    DRIVER_TYPE_CRYPTO,        // Cryptographic accelerators
    DRIVER_TYPE_AI,            // AI/ML accelerators (NPU, TPU)
    DRIVER_TYPE_VIRTUAL,       // Virtual devices
    DRIVER_TYPE_PLATFORM,      // Platform-specific devices
    DRIVER_TYPE_MISC           // Miscellaneous devices
} driver_type_t;

// Device states
typedef enum {
    DEVICE_STATE_UNKNOWN,
    DEVICE_STATE_UNINITIALIZED,
    DEVICE_STATE_INITIALIZING,
    DEVICE_STATE_ACTIVE,
    DEVICE_STATE_SUSPENDED,
    DEVICE_STATE_ERROR,
    DEVICE_STATE_REMOVED
} device_state_t;

// Power states
typedef enum {
    POWER_STATE_D0,     // Fully powered
    POWER_STATE_D1,     // Low power
    POWER_STATE_D2,     // Lower power
    POWER_STATE_D3_HOT, // Power off, context preserved
    POWER_STATE_D3_COLD // Power off, context lost
} power_state_t;

// Forward declarations
typedef struct device device_t;
typedef struct driver driver_t;
typedef struct bus_type bus_type_t;
typedef struct driver_class driver_class_t;

// Device resource structure
typedef struct device_resource {
    enum {
        RESOURCE_MEM,       // Memory region
        RESOURCE_IO,        // I/O port range
        RESOURCE_IRQ,       // Interrupt request
        RESOURCE_DMA        // DMA channel
    } type;
    
    union {
        struct {
            phys_addr_t start;
            size_t size;
            uint32_t flags;
        } mem;
        
        struct {
            uint16_t start;
            uint16_t end;
        } io;
        
        struct {
            int irq;
            uint32_t flags;
        } irq;
        
        struct {
            int channel;
            uint32_t flags;
        } dma;
    };
} device_resource_t;

// Device identification structure
typedef struct device_id {
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_vendor_id;
    uint32_t subsystem_device_id;
    uint32_t class_id;
    uint32_t class_mask;
    void* driver_data;      // Driver-specific data
} device_id_t;

// Match function type for custom device matching
typedef int (*match_func_t)(device_t* dev, driver_t* drv);

// Device structure
struct device {
    // Device identification
    char name[DRIVER_NAME_MAX];
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t subsystem_vendor_id;
    uint32_t subsystem_device_id;
    uint32_t class_id;
    uint32_t revision;
    
    // Device hierarchy
    device_t* parent;           // Parent device
    device_t* children;         // Child devices (linked list)
    device_t* sibling;          // Next sibling device
    bus_type_t* bus;            // Bus this device is on
    
    // Device state
    device_state_t state;
    power_state_t power_state;
    atomic_t ref_count;
    
    // Resources
    device_resource_t resources[MAX_RESOURCES];
    uint32_t num_resources;
    
    // Driver binding
    driver_t* driver;           // Bound driver
    void* driver_data;          // Driver private data
    
    // Hardware information
    phys_addr_t base_addr;
    size_t mem_size;
    int irq;
    
    // Synchronization
    void* lock;                 // Device-specific lock
    
    // Platform-specific data
    void* platform_data;
    
    // Device tree/ACPI information
    void* of_node;              // Device tree node (if applicable)
    void* acpi_handle;          // ACPI handle (if applicable)
};

// Driver operations structure
typedef struct driver_operations {
    // Core lifecycle
    int (*probe)(device_t* dev, const device_id_t* id);
    int (*remove)(device_t* dev);
    
    // Power management
    int (*suspend)(device_t* dev, power_state_t state);
    int (*resume)(device_t* dev);
    int (*set_power_state)(device_t* dev, power_state_t state);
    
    // I/O operations
    ssize_t (*read)(device_t* dev, void* buf, size_t count, off_t offset);
    ssize_t (*write)(device_t* dev, const void* buf, size_t count, off_t offset);
    int (*ioctl)(device_t* dev, unsigned long cmd, void* arg);
    int (*mmap)(device_t* dev, void** vaddr, size_t size, uint32_t flags);
    
    // Interrupt handling
    void (*interrupt_handler)(device_t* dev, int irq, void* data);
    
    // DMA operations
    int (*dma_alloc)(device_t* dev, size_t size, hal_dma_buffer_t** buffer);
    void (*dma_free)(device_t* dev, hal_dma_buffer_t* buffer);
    
    // Configuration
    int (*get_config)(device_t* dev, void* config, size_t size);
    int (*set_config)(device_t* dev, const void* config, size_t size);
    
    // Status and diagnostics
    int (*get_status)(device_t* dev, uint32_t* status);
    int (*self_test)(device_t* dev);
    int (*reset)(device_t* dev);
    
    // Hotplug support
    int (*hotplug_add)(device_t* dev);
    int (*hotplug_remove)(device_t* dev);
} driver_ops_t;

// Driver structure
struct driver {
    // Driver identification
    char name[DRIVER_NAME_MAX];
    char description[DRIVER_DESC_MAX];
    char author[DRIVER_AUTHOR_MAX];
    char license[DRIVER_LICENSE_MAX];
    char version[DRIVER_VERSION_MAX];
    uint32_t api_version;
    
    // Driver classification
    driver_type_t type;
    uint32_t flags;
    
    // Device matching
    device_id_t* id_table;      // Supported device IDs
    size_t id_table_size;
    match_func_t match;         // Custom match function
    
    // Dependencies
    char dependencies[MAX_DEPENDENCIES][DRIVER_NAME_MAX];
    size_t dependency_count;
    
    // Operations
    driver_ops_t* ops;
    
    // Driver class (for similar drivers)
    driver_class_t* class;
    
    // Module information
    void* module;               // Module handle (if loadable)
    
    // Statistics
    struct {
        uint64_t load_time;
        uint32_t device_count;
        uint32_t error_count;
    } stats;
    
    // Private data
    void* private_data;
};

// Bus type structure for different bus types (PCI, USB, etc.)
struct bus_type {
    char name[DRIVER_NAME_MAX];
    
    // Bus operations
    int (*match)(device_t* dev, driver_t* drv);
    int (*uevent)(device_t* dev, char** envp);
    int (*probe)(device_t* dev);
    int (*remove)(device_t* dev);
    
    // Power management
    int (*suspend)(device_t* dev, power_state_t state);
    int (*resume)(device_t* dev);
    
    // Bus-specific attributes
    void* private_data;
};

// Driver class structure for grouping similar drivers
struct driver_class {
    char name[DRIVER_NAME_MAX];
    driver_type_t type;
    
    // Class operations
    int (*init)(driver_t* drv);
    void (*cleanup)(driver_t* drv);
    
    // Default operations (can be overridden by drivers)
    driver_ops_t* default_ops;
    
    void* private_data;
};

// Driver registration information
typedef struct driver_registration {
    driver_t* driver;
    uint32_t flags;
    int priority;               // Loading priority
    char module_path[256];      // Path to driver module (if loadable)
} driver_registration_t;

// Driver framework flags
#define DRIVER_FLAG_LOADABLE    (1 << 0)  // Driver can be loaded/unloaded
#define DRIVER_FLAG_BUILTIN     (1 << 1)  // Built into kernel
#define DRIVER_FLAG_HOTPLUG     (1 << 2)  // Supports hotplug
#define DRIVER_FLAG_EXCLUSIVE   (1 << 3)  // Exclusive device access
#define DRIVER_FLAG_SHARED      (1 << 4)  // Shared device access
#define DRIVER_FLAG_REAL_TIME   (1 << 5)  // Real-time driver
#define DRIVER_FLAG_POWER_MANAGED (1 << 6) // Supports power management

// Error codes
#define DRIVER_SUCCESS           0
#define DRIVER_ERR_NO_DEVICE    -2001
#define DRIVER_ERR_PROBE_FAILED -2002
#define DRIVER_ERR_NO_MEMORY    -2003
#define DRIVER_ERR_BUSY         -2004
#define DRIVER_ERR_TIMEOUT      -2005
#define DRIVER_ERR_NOT_SUPPORTED -2006
#define DRIVER_ERR_CONFIG       -2007
#define DRIVER_ERR_HARDWARE     -2008

// Driver Framework API Functions

// Driver registration and management
int driver_register(driver_t* driver);
int driver_unregister(driver_t* driver);
driver_t* driver_find(const char* name);
int driver_load(const char* name);
int driver_unload(const char* name);

// Device management
device_t* device_create(const char* name, bus_type_t* bus, device_t* parent);
void device_destroy(device_t* dev);
int device_register(device_t* dev);
int device_unregister(device_t* dev);
device_t* device_find(const char* name);
device_t* device_find_by_id(uint32_t vendor_id, uint32_t device_id);

// Device binding
int device_bind_driver(device_t* dev, driver_t* drv);
int device_unbind_driver(device_t* dev);
int device_probe(device_t* dev);

// Resource management
int device_request_resources(device_t* dev);
void device_release_resources(device_t* dev);
device_resource_t* device_get_resource(device_t* dev, int type, int index);
int device_get_irq(device_t* dev, int index);

// Bus type management
int bus_register(bus_type_t* bus);
int bus_unregister(bus_type_t* bus);
bus_type_t* bus_find(const char* name);

// Driver class management
int driver_class_register(driver_class_t* class);
int driver_class_unregister(driver_class_t* class);
driver_class_t* driver_class_find(const char* name);

// Device enumeration and discovery
int driver_framework_scan_bus(bus_type_t* bus);
int driver_framework_enumerate_devices(device_t** devices, size_t* count);
int driver_framework_enumerate_drivers(driver_t** drivers, size_t* count);

// Power management
int device_set_power_state(device_t* dev, power_state_t state);
power_state_t device_get_power_state(device_t* dev);
int device_suspend(device_t* dev);
int device_resume(device_t* dev);

// Hotplug support
int device_hotplug_add(device_t* dev);
int device_hotplug_remove(device_t* dev);

// Utility functions
const char* device_state_to_string(device_state_t state);
const char* power_state_to_string(power_state_t state);
const char* driver_type_to_string(driver_type_t type);

// Reference counting
device_t* device_get(device_t* dev);
void device_put(device_t* dev);

// Lock management
int device_lock(device_t* dev);
int device_trylock(device_t* dev);
void device_unlock(device_t* dev);

// Initialization and cleanup
int driver_framework_init(void);
void driver_framework_cleanup(void);

// Macros for common operations
#define DEVICE_ATTR(_name, _show, _store) \
    struct device_attribute dev_attr_##_name = { \
        .attr = { .name = #_name }, \
        .show = _show, \
        .store = _store \
    }

#define DRIVER_MODULE_AUTHOR(name)    MODULE_AUTHOR(name)
#define DRIVER_MODULE_DESCRIPTION(desc) MODULE_DESCRIPTION(desc)
#define DRIVER_MODULE_LICENSE(license) MODULE_LICENSE(license)
#define DRIVER_MODULE_VERSION(version) MODULE_VERSION(version)

// Device ID table macros
#define DEVICE_ID_PCI(vend, dev) \
    { .vendor_id = (vend), .device_id = (dev), \
      .subsystem_vendor_id = DEVICE_ID_ANY, \
      .subsystem_device_id = DEVICE_ID_ANY }

#define DEVICE_ID_USB(vend, prod) \
    { .vendor_id = (vend), .device_id = (prod) }

#define DEVICE_ID_CLASS(class, mask) \
    { .class_id = (class), .class_mask = (mask), \
      .vendor_id = DEVICE_ID_ANY, .device_id = DEVICE_ID_ANY }

// Driver initialization helpers
#define DEFINE_DRIVER(name, type_val, id_tbl, ops_ptr) \
    static driver_t name##_driver = { \
        .name = #name, \
        .type = type_val, \
        .id_table = id_tbl, \
        .id_table_size = sizeof(id_tbl) / sizeof(device_id_t), \
        .ops = ops_ptr, \
        .api_version = DRIVER_API_VERSION \
    }

#ifdef __cplusplus
}
#endif

#endif // DRIVER_FRAMEWORK_H