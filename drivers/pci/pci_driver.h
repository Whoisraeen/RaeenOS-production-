#ifndef PCI_DRIVER_H
#define PCI_DRIVER_H

#include <stdint.h>
#include "../../kernel/include/driver.h"

// PCI device structure (simplified)
typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision_id;
    // Add more PCI configuration space fields as needed
} pci_device_t;

// Function to initialize the PCI bus driver
void pci_driver_init(void);

// Function to enumerate PCI devices and call probe functions for matching drivers
void pci_enumerate_devices(void);

#endif // PCI_DRIVER_H
