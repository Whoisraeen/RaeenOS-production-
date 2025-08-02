#ifndef ARP_H
#define ARP_H

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.h"

// Initialize the ARP module
void arp_init(void);

// Resolve an IPv4 address to a MAC address (blocking for now)
bool arp_resolve(ipv4_addr_t ip_address, uint8_t* mac_address);

// Handle an incoming ARP packet
void arp_handle_packet(const uint8_t* packet, uint32_t size);

#endif // ARP_H
