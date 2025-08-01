#ifndef DHCP_H
#define DHCP_H

#include "include/types.h"
#include "tcpip.h"

// DHCP message types
typedef enum {
    DHCP_DISCOVER = 1,
    DHCP_OFFER,
    DHCP_REQUEST,
    DHCP_DECLINE,
    DHCP_ACK,
    DHCP_NAK,
    DHCP_RELEASE,
    DHCP_INFORM
} dhcp_message_type_t;

// DHCP packet structure (simplified)
typedef struct {
    uint8_t op; // Message op code / message type
    uint8_t htype; // Hardware address type
    uint8_t hlen; // Hardware address length
    uint8_t hops; // Hops
    uint32_t xid; // Transaction ID
    uint16_t secs; // Seconds since beginning of transaction
    uint16_t flags; // Flags
    ipv4_addr_t ciaddr; // Client IP address
    ipv4_addr_t yiaddr; // Your (client) IP address
    ipv4_addr_t siaddr; // Server IP address
    ipv4_addr_t giaddr; // Gateway IP address
    uint8_t chaddr[16]; // Client hardware address
    uint8_t sname[64]; // Server host name
    uint8_t file[128]; // Boot file name
    uint8_t options[312]; // Optional parameters field
} __attribute__((packed)) dhcp_packet_t;

// Initialize DHCP client
void dhcp_init(void);

// Request an IP address via DHCP
int dhcp_request_ip(ipv4_addr_t* assigned_ip);

#endif // DHCP_H
