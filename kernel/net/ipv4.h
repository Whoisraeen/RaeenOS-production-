#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>
#include <stdbool.h>

// IPv4 address structure
typedef uint32_t ipv4_addr_t;

// Initialize the IPv4 layer
void ipv4_init(void);

// Send an IPv4 packet
int ipv4_send_packet(ipv4_addr_t dest_ip, uint8_t protocol, const uint8_t* data, uint32_t size);

// Receive an IPv4 packet (callback registration)
typedef void (*ipv4_receive_callback_t)(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size);
void ipv4_register_receive_callback(uint8_t protocol, ipv4_receive_callback_t callback);

#endif // IPV4_H
