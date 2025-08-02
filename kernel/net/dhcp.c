#include "dhcp.h"
#include "udp.h"
#include "../vga.h"
#include "../string.h"

void dhcp_init(void) {
    debug_print("DHCP client initialized (placeholder).\n");
    // Register DHCP client with UDP port 68
    udp_register_receive_callback(68, dhcp_handle_udp_packet);
}

int dhcp_request_ip(uint8_t* mac_address, ipv4_addr_t* assigned_ip, ipv4_addr_t* gateway, ipv4_addr_t* dns_server) {
    debug_print("DHCP: Requesting IP address (simulated).\n");

    // Simulate DHCP Discover, Offer, Request, ACK process
    // This would involve sending UDP packets to DHCP server (255.255.255.255:67)

    // For now, assign a dummy IP
    if (assigned_ip) *assigned_ip = 0xC0A8010A; // 192.168.1.10
    if (gateway) *gateway = 0xC0A80101; // 192.168.1.1
    if (dns_server) *dns_server = 0x08080808; // 8.8.8.8

    debug_print("DHCP: Assigned IP ");
    vga_put_hex(*assigned_ip);
    debug_print(" (simulated).\n");

    return 0; // Success
}

void dhcp_handle_udp_packet(ipv4_addr_t src_ip, uint16_t src_port, uint16_t dest_port, const uint8_t* data, uint32_t size) {
    if (src_port == 67 && dest_port == 68) { // DHCP server to client
        debug_print("DHCP: Received DHCP packet (simulated).\n");
        // In a real implementation, this would parse the DHCP packet
        // and update network configuration.
    }
}