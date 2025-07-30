#include "dhcp.h"
#include "../vga.h"

void dhcp_init(void) {
    vga_puts("DHCP client initialized (placeholder).\n");
}

int dhcp_request_ip(ipv4_addr_t* assigned_ip) {
    (void)assigned_ip;
    vga_puts("Requesting IP via DHCP (placeholder).\n");
    return -1; // Not implemented
}

