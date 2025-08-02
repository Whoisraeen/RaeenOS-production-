#include "udp.h"
#include "ipv4.h"
#include "../vga.h"
#include "../string.h"

// Placeholder for UDP receive callbacks
static udp_receive_callback_t udp_callbacks[65536]; // Indexed by port number

void udp_init(void) {
    debug_print("UDP stack initialized (placeholder).\n");
    memset(udp_callbacks, 0, sizeof(udp_callbacks));
    // Register UDP with IPv4 layer
    ipv4_register_receive_callback(17, (ipv4_receive_callback_t)udp_handle_ipv4_packet); // Protocol 17 for UDP
}

int udp_send_packet(ipv4_addr_t dest_ip, uint16_t dest_port, uint16_t src_port, const uint8_t* data, uint32_t size) {
    debug_print("UDP: Sending packet from port ");
    vga_put_dec(src_port);
    debug_print(" to ");
    vga_put_hex(dest_ip);
    debug_print(":");
    vga_put_dec(dest_port);
    debug_print(" (Size: ");
    vga_put_dec(size);
    debug_print(")\n");

    // In a real implementation, this would construct a UDP header and pass to IP layer.
    // For now, just simulate.
    uint8_t udp_packet[2048]; // Max UDP packet size
    // Copy UDP header (8 bytes)
    // Copy data

    return ipv4_send_packet(dest_ip, 17, udp_packet, size + 8); // Protocol 17 for UDP
}

void udp_register_receive_callback(uint16_t port, udp_receive_callback_t callback) {
    if (port < 65536) {
        udp_callbacks[port] = callback;
        debug_print("UDP: Registered callback for port ");
        vga_put_dec(port);
        debug_print("\n");
    }
}

// This function would be called by the IPv4 layer when a UDP packet is received
void udp_handle_ipv4_packet(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size) {
    if (protocol != 17) return; // Not a UDP packet

    // Simulate parsing UDP header
    uint16_t src_port = *((uint16_t*)data); // Placeholder
    uint16_t dest_port = *((uint16_t*)(data + 2)); // Placeholder
    const uint8_t* udp_data = data + 8; // UDP header is 8 bytes
    uint32_t udp_data_size = size - 8;

    debug_print("UDP: Received packet from ");
    vga_put_hex(src_ip);
    debug_print(":");
    vga_put_dec(src_port);
    debug_print(" to port ");
    vga_put_dec(dest_port);
    debug_print(" (Size: ");
    vga_put_dec(udp_data_size);
    debug_print(")\n");

    if (dest_port < 65536 && udp_callbacks[dest_port]) {
        udp_callbacks[dest_port](src_ip, src_port, dest_port, udp_data, udp_data_size);
    }
}
