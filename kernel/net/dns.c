#include "dns.h"
#include "udp.h"
#include "../vga.h"
#include "../string.h"

static ipv4_addr_t dns_server_ip = 0; // Default to 0, set by DHCP or manually

void dns_init(void) {
    debug_print("DNS client initialized (placeholder).\n");
    // Register DNS client with UDP port 53
    udp_register_receive_callback(53, dns_handle_udp_packet);
}

void dns_set_server(ipv4_addr_t server_ip) {
    dns_server_ip = server_ip;
    debug_print("DNS: Server set to ");
    vga_put_hex(dns_server_ip);
    debug_print("\n");
}

ipv4_addr_t dns_resolve_hostname(const char* hostname) {
    debug_print("DNS: Resolving hostname ");
    debug_print(hostname);
    debug_print(" (simulated).\n");

    if (dns_server_ip == 0) {
        debug_print("DNS: No DNS server configured.\n");
        return 0; // No DNS server
    }

    // Simulate DNS query (sending UDP packet to DNS server on port 53)
    // This would involve constructing a DNS query packet.

    // For now, return a dummy IP for any hostname
    if (strcmp(hostname, "google.com") == 0) return 0x08080808; // 8.8.8.8
    if (strcmp(hostname, "raeenos.org") == 0) return 0xC0A80101; // 192.168.1.1

    return 0; // Host not found
}

void dns_handle_udp_packet(ipv4_addr_t src_ip, uint16_t src_port, uint16_t dest_port, const uint8_t* data, uint32_t size) {
    if (src_port == 53) { // DNS server reply
        debug_print("DNS: Received DNS reply (simulated).\n");
        // In a real implementation, this would parse the DNS reply
        // and provide the resolved IP address.
    }
}