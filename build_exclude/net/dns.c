#include "dns.h"
#include "../vga.h"

void dns_init(void) {
    vga_puts("DNS client initialized (placeholder).\n");
}

int dns_resolve(const char* hostname, ipv4_addr_t* ip_address) {
    (void)hostname;
    (void)ip_address;
    vga_puts("Resolving hostname via DNS (placeholder).\n");
    return -1; // Not implemented
}

