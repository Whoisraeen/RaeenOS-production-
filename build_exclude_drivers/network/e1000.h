#ifndef E1000_H
#define E1000_H

#include <stdint.h>
#include <stddef.h>

// Initialize the E1000 network driver
void e1000_init(uint8_t bus, uint8_t device, uint8_t function);

// Send a packet using the E1000 driver
int e1000_send_packet(const uint8_t* data, uint32_t size);

// Receive a packet using the E1000 driver
int e1000_receive_packet(uint8_t* buffer, uint32_t buffer_size);

#endif // E1000_H
