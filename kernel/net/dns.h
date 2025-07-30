#ifndef DNS_H
#define DNS_H

#include <stdint.h>
#include "tcpip.h"

// DNS packet structure (simplified)
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    // Followed by questions, answers, authority, and additional sections
} __attribute__((packed)) dns_header_t;

// Initialize DNS client
void dns_init(void);

// Resolve a hostname to an IP address
int dns_resolve(const char* hostname, ipv4_addr_t* ip_address);

#endif // DNS_H
