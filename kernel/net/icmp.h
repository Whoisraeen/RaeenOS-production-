#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.h"

// ICMP message types
#define ICMP_ECHO_REPLY     0
#define ICMP_DEST_UNREACH   3
#define ICMP_ECHO_REQUEST   8
#define ICMP_TIME_EXCEEDED  11

// Initialize the ICMP module
void icmp_init(void);

// Send an ICMP echo request (ping)
int icmp_send_echo_request(ipv4_addr_t dest_ip, uint16_t identifier, uint16_t sequence_number, const uint8_t* data, uint32_t size);

// Handle an incoming ICMP packet
void icmp_handle_ipv4_packet(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size);

#endif // ICMP_H
