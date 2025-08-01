#ifndef PCI_DRIVER_H
#define PCI_DRIVER_H

#include <stdint.h>
#include "../../kernel/include/driver.h"
#include "pci.h"  // Use the main PCI header for pci_device_t definition

// Function to initialize the PCI bus driver
void pci_driver_init(void);

// Function to enumerate PCI devices and call probe functions for matching drivers
void pci_enumerate_devices(void);

#endif // PCI_DRIVER_H
