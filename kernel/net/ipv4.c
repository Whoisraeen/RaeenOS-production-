#include "ipv4.h"
#include "../../drivers/network/ethernet.h"
#include "../vga.h"
#include "../string.h"

// Placeholder for IPv4 receive callbacks
static ipv4_receive_callback_t ipv4_callbacks[256]; // Indexed by protocol number

void ipv4_init(void) {
    debug_print("IPv4 layer initialized (placeholder).\n");
    memset(ipv4_callbacks, 0, sizeof(ipv4_callbacks));
}

int ipv4_send_packet(ipv4_addr_t dest_ip, uint8_t protocol, const uint8_t* data, uint32_t size) {
    debug_print("IPv4: Sending packet to ");
    vga_put_hex(dest_ip);
    debug_print(" (Protocol: ");
    vga_put_hex(protocol);
    debug_print(", Size: ");
    vga_put_dec(size);
    debug_print(")\n");

    // In a real implementation, this would construct an IP header,
    // resolve MAC address via ARP, and then call ethernet_send_packet.
    // For now, just simulate.

    // Simulate ARP lookup (placeholder)
    uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast for now

    // Simulate Ethernet frame construction (placeholder)
    uint8_t ethernet_frame[1500]; // Max Ethernet frame size
    // Copy destination MAC, source MAC, EtherType (0x0800 for IPv4)
    // Copy IP packet data

    return ethernet_send_packet(ethernet_frame, size + 14); // IP packet size + Ethernet header
}

void ipv4_register_receive_callback(uint8_t protocol, ipv4_receive_callback_t callback) {
    if (protocol < 256) {
        ipv4_callbacks[protocol] = callback;
        debug_print("IPv4: Registered callback for protocol ");
        vga_put_hex(protocol);
        debug_print("\n");
    }
}

// This function would be called by the Ethernet driver when an IP packet is received
void ipv4_handle_ethernet_packet(const uint8_t* packet, uint32_t size) {
    // Simulate parsing IP header
    uint8_t protocol = packet[9]; // Placeholder: assuming protocol is at offset 9
    ipv4_addr_t src_ip = *((ipv4_addr_t*)(packet + 12)); // Placeholder: assuming src IP at offset 12
    const uint8_t* data = packet + 20; // Placeholder: assuming IP header is 20 bytes
    uint32_t data_size = size - 20;

    debug_print("IPv4: Received packet from ");
    vga_put_hex(src_ip);
    debug_print(" (Protocol: ");
    vga_put_hex(protocol);
    debug_print(", Size: ");
    vga_put_dec(data_size);
    debug_print(")\n");

    if (protocol < 256 && ipv4_callbacks[protocol]) {
        ipv4_callbacks[protocol](src_ip, protocol, data, data_size);
    }
}
