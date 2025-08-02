#ifndef DNS_H
#define DNS_H

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.h"

// Initialize the DNS client
void dns_init(void);

// Resolve a hostname to an IPv4 address
ipv4_addr_t dns_resolve_hostname(const char* hostname);

#endif // DNS_H