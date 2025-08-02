#include "udp.h"
#include "ipv4.h"
#include "../vga.h"
#include "../libs/libc/include/string.h"

// Placeholder for UDP receive callbacks
static udp_receive_callback_t udp_callbacks[65536]; // Indexed by port number

// Simple checksum calculation (placeholder)
static uint16_t udp_checksum(const uint8_t* data, uint32_t size, ipv4_addr_t src_ip, ipv4_addr_t dest_ip) {
    // In a real implementation, this would calculate the UDP checksum
    // including a pseudo-header.
    (void)data;
    (void)size;
    (void)src_ip;
    (void)dest_ip;
    return 0; // Dummy checksum
}

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
    uint8_t udp_packet[2048]; // Max UDP packet size
    // Copy UDP header (8 bytes)
    // src_port, dest_port, length, checksum
    udp_packet[0] = (src_port >> 8) & 0xFF;
    udp_packet[1] = src_port & 0xFF;
    udp_packet[2] = (dest_port >> 8) & 0xFF;
    udp_packet[3] = dest_port & 0xFF;
    uint16_t total_len = size + 8; // UDP header + data
    udp_packet[4] = (total_len >> 8) & 0xFF;
    udp_packet[5] = total_len & 0xFF;
    // Checksum (placeholder)
    udp_packet[6] = 0;
    udp_packet[7] = 0;

    memcpy(udp_packet + 8, data, size);

    // Calculate and set checksum
    uint16_t csum = udp_checksum(udp_packet, total_len, 0, dest_ip); // TODO: Get actual src_ip
    udp_packet[6] = (csum >> 8) & 0xFF;
    udp_packet[7] = csum & 0xFF;

    return ipv4_send_packet(dest_ip, 17, udp_packet, total_len); // Protocol 17 for UDP
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
    uint16_t src_port = (data[0] << 8) | data[1];
    uint16_t dest_port = (data[2] << 8) | data[3];
    uint16_t length = (data[4] << 8) | data[5];
    uint16_t checksum = (data[6] << 8) | data[7];
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
    debug_print(", Checksum: ");
    vga_put_hex(checksum);
    debug_print(")\n");

    // Verify checksum (placeholder)
    if (checksum != 0) { // 0 means checksum is optional
        uint16_t calculated_checksum = udp_checksum(data, length, src_ip, 0); // TODO: Get actual dest_ip
        if (calculated_checksum != checksum) {
            debug_print("UDP: Checksum mismatch!\n");
            return; // Drop packet
        }
    }

    if (dest_port < 65536 && udp_callbacks[dest_port]) {
        udp_callbacks[dest_port](src_ip, src_port, dest_port, udp_data, udp_data_size);
    }
}
