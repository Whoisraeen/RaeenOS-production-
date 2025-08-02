#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the Ethernet driver
void ethernet_init(void);

// Send an Ethernet packet
int ethernet_send_packet(const uint8_t* data, uint32_t size);

// Receive an Ethernet packet (blocking for now)
int ethernet_receive_packet(uint8_t* buffer, uint32_t buffer_size);

// Get the MAC address of the Ethernet interface
void ethernet_get_mac_address(uint8_t* mac_address);

#endif // ETHERNET_H
