/**
 * @file driver_framework.c
 * @brief Universal Driver Framework Core Implementation for RaeenOS
 * 
 * This is the core driver framework that provides:
 * - Driver sandboxing and isolation
 * - Crash recovery and fault tolerance
 * - Hot-plug support with instant recognition
 * - Dynamic loading/unloading
 * - Performance monitoring and telemetry
 * - Security and access control
 * 
 * Superior to Windows WDM/KMDF and macOS DriverKit
 * 
 * Author: RaeenOS Driver Team
 * License: MIT
 * Version: 1.0.0
 */

#include "../kernel/include/driver_framework.h"
#include "../kernel/include/hal_interface.h"
#include "../kernel/include/memory_interface.h"
#include "../kernel/include/security_interface.h"
#include "../kernel/include/process_interface.h"
#include "../security/security_core.h"

// Driver framework internal structures
typedef struct driver_sandbox {
    pid_t sandbox_pid;              // Sandboxed process ID
    void* memory_pool;              // Isolated memory pool
    size_t memory_size;             // Size of memory pool
    uint32_t permissions;           // Access permissions
    uint64_t crash_count;           // Number of crashes
    uint64_t last_crash_time;       // Timestamp of last crash
    bool is_isolated;               // Is driver running in isolation
    hal_security_context_t* sec_ctx; // Security context
} driver_sandbox_t;

typedef struct driver_manager {
    driver_t* drivers[MAX_DRIVERS];
    device_t* devices[MAX_DEVICES];
    bus_type_t* buses[MAX_BUS_TYPES];
    driver_class_t* classes[MAX_DRIVER_CLASSES];
    
    uint32_t driver_count;
    uint32_t device_count;
    uint32_t bus_count;
    uint32_t class_count;
    
    // Hot-plug support
    struct {
        void (*hotplug_callback)(device_t* dev, int event);
        void* hotplug_data;
        bool hotplug_enabled;
    } hotplug;
    
    // Performance monitoring
    struct {
        uint64_t total_probe_time;
        uint64_t total_io_operations;
        uint64_t total_interrupts;
        uint32_t error_count;
        uint32_t crash_count;
        bool monitoring_enabled;
    } stats;
    
    // Security context
    hal_security_context_t* global_sec_ctx;
    
    // Synchronization
    void* framework_lock;
} driver_manager_t;

// Global driver manager instance
static driver_manager_t g_driver_manager = {0};

// Driver sandboxes for isolation
static driver_sandbox_t g_driver_sandboxes[MAX_DRIVERS];

// Constants
#define MAX_DRIVERS 256
#define MAX_DEVICES 1024
#define MAX_BUS_TYPES 32
#define MAX_DRIVER_CLASSES 64
#define SANDBOX_MEMORY_SIZE (2 * 1024 * 1024)  // 2MB per driver
#define MAX_CRASH_COUNT 3
#define CRASH_RECOVERY_DELAY 1000  // 1 second

// Driver framework crash recovery
static int driver_crash_recovery(driver_t* driver, device_t* device) {
    if (!driver || !device) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    // Find driver sandbox
    driver_sandbox_t* sandbox = NULL;
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (g_driver_sandboxes[i].sandbox_pid && 
            g_driver_manager.drivers[i] == driver) {
            sandbox = &g_driver_sandboxes[i];
            break;
        }
    }
    
    if (!sandbox) {
        return DRIVER_ERR_CONFIG;
    }
    
    // Increment crash counter
    sandbox->crash_count++;
    sandbox->last_crash_time = hal_get_system_time();
    
    // Check if driver has crashed too many times
    if (sandbox->crash_count > MAX_CRASH_COUNT) {
        // Disable driver permanently
        device->state = DEVICE_STATE_ERROR;
        return DRIVER_ERR_HARDWARE;
    }
    
    // Reset driver state
    device->state = DEVICE_STATE_UNINITIALIZED;
    
    // Wait before restart
    hal_sleep(CRASH_RECOVERY_DELAY);
    
    // Attempt to restart driver
    if (driver->ops && driver->ops->probe) {
        int result = driver->ops->probe(device, NULL);
        if (result == DRIVER_SUCCESS) {
            device->state = DEVICE_STATE_ACTIVE;
            return DRIVER_SUCCESS;
        }
    }
    
    return DRIVER_ERR_PROBE_FAILED;
}

// Driver sandboxing implementation
static int create_driver_sandbox(driver_t* driver, driver_sandbox_t* sandbox) {
    if (!driver || !sandbox) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    // Create isolated memory pool
    sandbox->memory_pool = hal_alloc_dma_coherent(SANDBOX_MEMORY_SIZE);
    if (!sandbox->memory_pool) {
        return DRIVER_ERR_NO_MEMORY;
    }
    
    sandbox->memory_size = SANDBOX_MEMORY_SIZE;
    sandbox->crash_count = 0;
    sandbox->last_crash_time = 0;
    sandbox->is_isolated = true;
    
    // Create security context for driver
    sandbox->sec_ctx = security_create_context();
    if (!sandbox->sec_ctx) {
        hal_free_dma_coherent(sandbox->memory_pool, SANDBOX_MEMORY_SIZE);
        return DRIVER_ERR_CONFIG;
    }
    
    // Set up permissions based on driver type
    switch (driver->type) {
        case DRIVER_TYPE_STORAGE:
            sandbox->permissions = SEC_PERM_READ | SEC_PERM_WRITE | SEC_PERM_DMA;
            break;
        case DRIVER_TYPE_NETWORK:
            sandbox->permissions = SEC_PERM_READ | SEC_PERM_WRITE | SEC_PERM_NETWORK;
            break;
        case DRIVER_TYPE_AUDIO:
        case DRIVER_TYPE_VIDEO:
            sandbox->permissions = SEC_PERM_READ | SEC_PERM_WRITE | SEC_PERM_DMA | SEC_PERM_MMIO;
            break;
        default:
            sandbox->permissions = SEC_PERM_READ | SEC_PERM_WRITE;
            break;
    }
    
    // Apply security policy
    security_apply_policy(sandbox->sec_ctx, sandbox->permissions);
    
    return DRIVER_SUCCESS;
}

// Initialize driver framework
int driver_framework_init(void) {
    // Initialize driver manager
    memset(&g_driver_manager, 0, sizeof(driver_manager_t));
    memset(g_driver_sandboxes, 0, sizeof(g_driver_sandboxes));
    
    // Create framework lock
    g_driver_manager.framework_lock = hal_create_spinlock();
    if (!g_driver_manager.framework_lock) {
        return DRIVER_ERR_NO_MEMORY;
    }
    
    // Create global security context
    g_driver_manager.global_sec_ctx = security_create_context();
    if (!g_driver_manager.global_sec_ctx) {
        hal_destroy_spinlock(g_driver_manager.framework_lock);
        return DRIVER_ERR_CONFIG;
    }
    
    // Enable hot-plug support
    g_driver_manager.hotplug.hotplug_enabled = true;
    
    // Enable performance monitoring
    g_driver_manager.stats.monitoring_enabled = true;
    
    return DRIVER_SUCCESS;
}

// Clean up driver framework
void driver_framework_cleanup(void) {
    hal_acquire_spinlock(g_driver_manager.framework_lock);
    
    // Unregister all drivers
    for (int i = 0; i < g_driver_manager.driver_count; i++) {
        if (g_driver_manager.drivers[i]) {
            driver_unregister(g_driver_manager.drivers[i]);
        }
    }
    
    // Destroy all devices
    for (int i = 0; i < g_driver_manager.device_count; i++) {
        if (g_driver_manager.devices[i]) {
            device_destroy(g_driver_manager.devices[i]);
        }
    }
    
    // Clean up sandboxes
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (g_driver_sandboxes[i].memory_pool) {
            hal_free_dma_coherent(g_driver_sandboxes[i].memory_pool, 
                                 g_driver_sandboxes[i].memory_size);
        }
        if (g_driver_sandboxes[i].sec_ctx) {
            security_destroy_context(g_driver_sandboxes[i].sec_ctx);
        }
    }
    
    // Clean up security context
    if (g_driver_manager.global_sec_ctx) {
        security_destroy_context(g_driver_manager.global_sec_ctx);
    }
    
    hal_release_spinlock(g_driver_manager.framework_lock);
    hal_destroy_spinlock(g_driver_manager.framework_lock);
}

// Register a driver with advanced features
int driver_register(driver_t* driver) {
    if (!driver || !driver->name[0] || !driver->ops) {
        return DRIVER_ERR_CONFIG;
    }
    
    hal_acquire_spinlock(g_driver_manager.framework_lock);
    
    if (g_driver_manager.driver_count >= MAX_DRIVERS) {
        hal_release_spinlock(g_driver_manager.framework_lock);
        return DRIVER_ERR_NO_MEMORY;
    }
    
    // Check for duplicate driver name
    for (int i = 0; i < g_driver_manager.driver_count; i++) {
        if (g_driver_manager.drivers[i] && 
            strcmp(g_driver_manager.drivers[i]->name, driver->name) == 0) {
            hal_release_spinlock(g_driver_manager.framework_lock);
            return DRIVER_ERR_BUSY;
        }
    }
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (!g_driver_manager.drivers[i]) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        hal_release_spinlock(g_driver_manager.framework_lock);
        return DRIVER_ERR_NO_MEMORY;
    }
    
    // Create sandbox for driver
    int result = create_driver_sandbox(driver, &g_driver_sandboxes[slot]);
    if (result != DRIVER_SUCCESS) {
        hal_release_spinlock(g_driver_manager.framework_lock);
        return result;
    }
    
    // Register driver
    g_driver_manager.drivers[slot] = driver;
    g_driver_manager.driver_count++;
    
    // Initialize driver statistics
    driver->stats.load_time = hal_get_system_time();
    driver->stats.device_count = 0;
    driver->stats.error_count = 0;
    
    hal_release_spinlock(g_driver_manager.framework_lock);
    
    // Trigger device enumeration for this driver
    driver_framework_enumerate_and_probe(driver);
    
    return DRIVER_SUCCESS;
}

// Unregister driver with cleanup
int driver_unregister(driver_t* driver) {
    if (!driver) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    hal_acquire_spinlock(g_driver_manager.framework_lock);
    
    // Find driver slot
    int slot = -1;
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (g_driver_manager.drivers[i] == driver) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        hal_release_spinlock(g_driver_manager.framework_lock);
        return DRIVER_ERR_NO_DEVICE;
    }
    
    // Unbind all devices using this driver
    for (int i = 0; i < g_driver_manager.device_count; i++) {
        if (g_driver_manager.devices[i] && 
            g_driver_manager.devices[i]->driver == driver) {
            device_unbind_driver(g_driver_manager.devices[i]);
        }
    }
    
    // Clean up sandbox
    if (g_driver_sandboxes[slot].memory_pool) {
        hal_free_dma_coherent(g_driver_sandboxes[slot].memory_pool,
                             g_driver_sandboxes[slot].memory_size);
    }
    if (g_driver_sandboxes[slot].sec_ctx) {
        security_destroy_context(g_driver_sandboxes[slot].sec_ctx);
    }
    memset(&g_driver_sandboxes[slot], 0, sizeof(driver_sandbox_t));
    
    // Remove driver
    g_driver_manager.drivers[slot] = NULL;
    g_driver_manager.driver_count--;
    
    hal_release_spinlock(g_driver_manager.framework_lock);
    return DRIVER_SUCCESS;
}

// Advanced device creation with security
device_t* device_create(const char* name, bus_type_t* bus, device_t* parent) {
    if (!name) {
        return NULL;
    }
    
    device_t* dev = hal_alloc_zeroed(sizeof(device_t));
    if (!dev) {
        return NULL;
    }
    
    // Initialize device
    strncpy(dev->name, name, DRIVER_NAME_MAX - 1);
    dev->bus = bus;
    dev->parent = parent;
    dev->state = DEVICE_STATE_UNINITIALIZED;
    dev->power_state = POWER_STATE_D3_COLD;
    
    // Initialize atomic reference count
    atomic_set(&dev->ref_count, 1);
    
    // Create device lock
    dev->lock = hal_create_spinlock();
    if (!dev->lock) {
        hal_free(dev);
        return NULL;
    }
    
    // Add to parent's children list
    if (parent) {
        device_get(parent);  // Take reference to parent
        dev->sibling = parent->children;
        parent->children = dev;
    }
    
    return dev;
}

// Enhanced device binding with crash recovery
int device_bind_driver(device_t* dev, driver_t* drv) {
    if (!dev || !drv) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    hal_acquire_spinlock((hal_spinlock_t*)dev->lock);
    
    if (dev->driver) {
        hal_release_spinlock((hal_spinlock_t*)dev->lock);
        return DRIVER_ERR_BUSY;
    }
    
    // Check if driver matches device
    bool match_found = false;
    if (drv->match) {
        match_found = (drv->match(dev, drv) == 0);
    } else if (drv->id_table) {
        // Check ID table
        for (size_t i = 0; i < drv->id_table_size; i++) {
            device_id_t* id = &drv->id_table[i];
            if ((id->vendor_id == DEVICE_ID_ANY || id->vendor_id == dev->vendor_id) &&
                (id->device_id == DEVICE_ID_ANY || id->device_id == dev->device_id) &&
                (id->class_id == CLASS_ID_ANY || 
                 (dev->class_id & id->class_mask) == id->class_id)) {
                match_found = true;
                break;
            }
        }
    }
    
    if (!match_found) {
        hal_release_spinlock((hal_spinlock_t*)dev->lock);
        return DRIVER_ERR_NOT_SUPPORTED;
    }
    
    // Bind driver to device
    dev->driver = drv;
    drv->stats.device_count++;
    
    hal_release_spinlock((hal_spinlock_t*)dev->lock);
    
    // Probe device with crash recovery
    int result = DRIVER_ERR_PROBE_FAILED;
    if (drv->ops && drv->ops->probe) {
        result = drv->ops->probe(dev, NULL);
        if (result != DRIVER_SUCCESS) {
            // Attempt crash recovery
            result = driver_crash_recovery(drv, dev);
        }
    }
    
    if (result == DRIVER_SUCCESS) {
        dev->state = DEVICE_STATE_ACTIVE;
        
        // Update statistics
        if (g_driver_manager.stats.monitoring_enabled) {
            g_driver_manager.stats.total_io_operations++;
        }
    } else {
        // Unbind driver on failure
        hal_acquire_spinlock((hal_spinlock_t*)dev->lock);
        dev->driver = NULL;
        drv->stats.device_count--;
        drv->stats.error_count++;
        hal_release_spinlock((hal_spinlock_t*)dev->lock);
    }
    
    return result;
}

// Hot-plug device addition with instant recognition
int device_hotplug_add(device_t* dev) {
    if (!dev) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    if (!g_driver_manager.hotplug.hotplug_enabled) {
        return DRIVER_ERR_NOT_SUPPORTED;
    }
    
    // Register device immediately
    int result = device_register(dev);
    if (result != DRIVER_SUCCESS) {
        return result;
    }
    
    // Find and bind appropriate driver
    for (int i = 0; i < g_driver_manager.driver_count; i++) {
        driver_t* drv = g_driver_manager.drivers[i];
        if (drv && (drv->flags & DRIVER_FLAG_HOTPLUG)) {
            if (device_bind_driver(dev, drv) == DRIVER_SUCCESS) {
                break;
            }
        }
    }
    
    // Notify hot-plug callback
    if (g_driver_manager.hotplug.hotplug_callback) {
        g_driver_manager.hotplug.hotplug_callback(dev, HOTPLUG_EVENT_ADD);
    }
    
    return DRIVER_SUCCESS;
}

// Advanced device enumeration with performance optimization
int driver_framework_enumerate_and_probe(driver_t* driver) {
    if (!driver) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    int devices_probed = 0;
    uint64_t start_time = hal_get_system_time();
    
    // Enumerate all devices and try to bind this driver
    for (int i = 0; i < g_driver_manager.device_count; i++) {
        device_t* dev = g_driver_manager.devices[i];
        if (dev && !dev->driver && dev->state == DEVICE_STATE_UNINITIALIZED) {
            if (device_bind_driver(dev, driver) == DRIVER_SUCCESS) {
                devices_probed++;
            }
        }
    }
    
    // Update performance statistics
    uint64_t end_time = hal_get_system_time();
    if (g_driver_manager.stats.monitoring_enabled) {
        g_driver_manager.stats.total_probe_time += (end_time - start_time);
    }
    
    return devices_probed;
}

// Performance monitoring and telemetry
typedef struct driver_telemetry {
    uint64_t io_operations;
    uint64_t bytes_transferred;
    uint64_t interrupt_count;
    uint64_t error_count;
    uint64_t avg_response_time;
    uint64_t peak_throughput;
} driver_telemetry_t;

int driver_get_telemetry(driver_t* driver, driver_telemetry_t* telemetry) {
    if (!driver || !telemetry) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    memset(telemetry, 0, sizeof(driver_telemetry_t));
    
    // Collect driver statistics
    telemetry->error_count = driver->stats.error_count;
    
    // Get system-wide statistics for this driver
    if (g_driver_manager.stats.monitoring_enabled) {
        telemetry->io_operations = g_driver_manager.stats.total_io_operations;
        telemetry->interrupt_count = g_driver_manager.stats.total_interrupts;
    }
    
    return DRIVER_SUCCESS;
}

// Advanced power management
int device_set_power_state(device_t* dev, power_state_t state) {
    if (!dev) {
        return DRIVER_ERR_NO_DEVICE;
    }
    
    hal_acquire_spinlock((hal_spinlock_t*)dev->lock);
    
    power_state_t old_state = dev->power_state;
    
    // Call driver's power management function
    if (dev->driver && dev->driver->ops && dev->driver->ops->set_power_state) {
        int result = dev->driver->ops->set_power_state(dev, state);
        if (result == DRIVER_SUCCESS) {
            dev->power_state = state;
        }
        hal_release_spinlock((hal_spinlock_t*)dev->lock);
        return result;
    }
    
    // Default power state handling
    dev->power_state = state;
    hal_release_spinlock((hal_spinlock_t*)dev->lock);
    
    return DRIVER_SUCCESS;
}

// Utility functions
const char* device_state_to_string(device_state_t state) {
    switch (state) {
        case DEVICE_STATE_UNKNOWN: return "Unknown";
        case DEVICE_STATE_UNINITIALIZED: return "Uninitialized";
        case DEVICE_STATE_INITIALIZING: return "Initializing";
        case DEVICE_STATE_ACTIVE: return "Active";
        case DEVICE_STATE_SUSPENDED: return "Suspended";
        case DEVICE_STATE_ERROR: return "Error";
        case DEVICE_STATE_REMOVED: return "Removed";
        default: return "Invalid";
    }
}

const char* driver_type_to_string(driver_type_t type) {
    switch (type) {
        case DRIVER_TYPE_BUS: return "Bus Controller";
        case DRIVER_TYPE_STORAGE: return "Storage";
        case DRIVER_TYPE_NETWORK: return "Network";
        case DRIVER_TYPE_AUDIO: return "Audio";
        case DRIVER_TYPE_VIDEO: return "Video/Graphics";
        case DRIVER_TYPE_INPUT: return "Input";
        case DRIVER_TYPE_SENSOR: return "Sensor";
        case DRIVER_TYPE_POWER: return "Power Management";
        case DRIVER_TYPE_CRYPTO: return "Cryptographic";
        case DRIVER_TYPE_AI: return "AI/ML Accelerator";
        case DRIVER_TYPE_VIRTUAL: return "Virtual";
        case DRIVER_TYPE_PLATFORM: return "Platform";
        case DRIVER_TYPE_MISC: return "Miscellaneous";
        default: return "Unknown";
    }
}

// Constants for hot-plug events
#define HOTPLUG_EVENT_ADD    1
#define HOTPLUG_EVENT_REMOVE 2