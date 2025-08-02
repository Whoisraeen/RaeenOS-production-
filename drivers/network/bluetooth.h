#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the Bluetooth driver
void bluetooth_init(void);

// Scan for Bluetooth devices
int bluetooth_scan_devices(void);

// Connect to a Bluetooth device
int bluetooth_connect(uint64_t address);

// Disconnect from a Bluetooth device
int bluetooth_disconnect(uint64_t address);

#endif // BLUETOOTH_H