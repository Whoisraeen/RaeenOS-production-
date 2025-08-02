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

// Send data over a Bluetooth connection
int bluetooth_send_data(uint64_t address, const uint8_t* data, uint32_t size);

// Receive data from a Bluetooth connection
int bluetooth_receive_data(uint64_t address, uint8_t* buffer, uint32_t buffer_size);

#endif // BLUETOOTH_H
