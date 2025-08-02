#ifndef UDP_H
#define UDP_H

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.h"

// Initialize the UDP stack
void udp_init(void);

// Send a UDP packet
int udp_send_packet(ipv4_addr_t dest_ip, uint16_t dest_port, uint16_t src_port, const uint8_t* data, uint32_t size);

// Receive a UDP packet (callback registration)
typedef void (*udp_receive_callback_t)(ipv4_addr_t src_ip, uint16_t src_port, uint16_t dest_port, const uint8_t* data, uint32_t size);
void udp_register_receive_callback(uint16_t port, udp_receive_callback_t callback);

#endif // UDP_H
