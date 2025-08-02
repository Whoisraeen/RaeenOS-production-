#ifndef DHCP_H
#define DHCP_H

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.h"

// Initialize the DHCP client
void dhcp_init(void);

// Request an IP address via DHCP
int dhcp_request_ip(uint8_t* mac_address, ipv4_addr_t* assigned_ip, ipv4_addr_t* gateway, ipv4_addr_t* dns_server);

#endif // DHCP_H