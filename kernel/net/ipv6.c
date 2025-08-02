#include "ipv6.h"
#include "../vga.h"
#include "../libs/libc/include/string.h"
#include "../../drivers/network/ethernet.h"


// Placeholder for IPv6 receive callbacks
static ipv6_receive_callback_t ipv6_callbacks[256]; // Indexed by next header

void ipv6_init(void) {
    debug_print("IPv6 layer initialized (placeholder).\n");
    memset(ipv6_callbacks, 0, sizeof(ipv6_callbacks));
}

int ipv6_send_packet(ipv6_addr_t dest_ip, uint8_t next_header, const uint8_t* data, uint32_t size) {
    debug_print("IPv6: Sending packet (simulated).\n");
    // In a real implementation, this would construct an IPv6 header
    // and pass it to the Ethernet layer.
    return 0; // Success
}

void ipv6_register_receive_callback(uint8_t next_header, ipv6_receive_callback_t callback) {
    if (next_header < 256) {
        ipv6_callbacks[next_header] = callback;
    }
}

// This function would be called by the Ethernet driver when an IPv6 packet is received
void ipv6_handle_ethernet_packet(const uint8_t* packet, uint32_t size) {
    debug_print("IPv6: Received packet (simulated).\n");
    // Simulate parsing IPv6 header
    uint8_t next_header = packet[6]; // Placeholder
    ipv6_addr_t src_ip; // Placeholder
    ipv6_addr_t dest_ip; // Placeholder
    const uint8_t* ipv6_data = packet + 40; // IPv6 header is 40 bytes
    uint32_t ipv6_data_size = size - 40;

    if (next_header < 256 && ipv6_callbacks[next_header]) {
        ipv6_callbacks[next_header](src_ip, next_header, ipv6_data, ipv6_data_size);
    }
}
