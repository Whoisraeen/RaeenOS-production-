#ifndef DRIVER_H
#define DRIVER_H

#include "types.h"
// Using types.h instead of stddef.h for kernel build

// Forward declaration of device_t (if we implement a device manager later)
struct device;

// Generic driver initialization function pointer
typedef int (*driver_init_t)(void);

// Generic driver probe function pointer (for bus drivers like PCI)
// This function would be called to detect and initialize devices on a bus.
// It might return a pointer to a device structure or a status code.
typedef int (*driver_probe_t)(struct device* dev);

// Generic driver structure
typedef struct {
    const char* name;           // Name of the driver
    driver_init_t init;         // Pointer to the driver's initialization function
    driver_probe_t probe;       // Pointer to the driver's probe function (optional)
    // Add more common function pointers as needed (e.g., suspend, resume, remove)
} driver_t;

// Function to register a driver with the kernel
void register_driver(driver_t* driver);

#endif // DRIVER_H
