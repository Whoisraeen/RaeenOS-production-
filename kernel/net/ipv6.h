#ifndef IPV6_H
#define IPV6_H

#include <stdint.h>
#include <stdbool.h>

// IPv6 address structure
typedef struct {
    uint8_t addr[16];
} ipv6_addr_t;

// Initialize the IPv6 layer
void ipv6_init(void);

// Send an IPv6 packet
int ipv6_send_packet(ipv6_addr_t dest_ip, uint8_t next_header, const uint8_t* data, uint32_t size);

// Receive an IPv6 packet (callback registration)
typedef void (*ipv6_receive_callback_t)(ipv6_addr_t src_ip, uint8_t next_header, const uint8_t* data, uint32_t size);
void ipv6_register_receive_callback(uint8_t next_header, ipv6_receive_callback_t callback);

#endif // IPV6_H
