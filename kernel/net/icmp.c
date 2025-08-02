#include "icmp.h"
#include "ipv4.h"
#include "../vga.h"
#include "../string.h"

void icmp_init(void) {
    debug_print("ICMP module initialized (placeholder).\n");
    // Register ICMP with IPv4 layer
    ipv4_register_receive_callback(1, icmp_handle_ipv4_packet); // Protocol 1 for ICMP
}

int icmp_send_echo_request(ipv4_addr_t dest_ip, uint16_t identifier, uint16_t sequence_number, const uint8_t* data, uint32_t size) {
    debug_print("ICMP: Sending Echo Request to ");
    vga_put_hex(dest_ip);
    debug_print(" (ID: ");
    vga_put_dec(identifier);
    debug_print(", Seq: ");
    vga_put_dec(sequence_number);
    debug_print(")\n");

    // In a real implementation, this would construct an ICMP echo request packet
    // and pass it to the IP layer.
    uint8_t icmp_packet[2048]; // Max ICMP packet size
    // Copy ICMP header (8 bytes for echo request/reply)
    // Copy identifier and sequence number
    // Copy data

    return ipv4_send_packet(dest_ip, 1, icmp_packet, size + 8); // Protocol 1 for ICMP
}

void icmp_handle_ipv4_packet(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size) {
    if (protocol != 1) return; // Not an ICMP packet

    // Simulate parsing ICMP header
    uint8_t type = data[0];
    uint8_t code = data[1];

    debug_print("ICMP: Received packet from ");
    vga_put_hex(src_ip);
    debug_print(" (Type: ");
    vga_put_dec(type);
    debug_print(", Code: ");
    vga_put_dec(code);
    debug_print(")\n");

    if (type == ICMP_ECHO_REQUEST) {
        debug_print("ICMP: Responding to Echo Request.\n");
        // Simulate sending an Echo Reply
        uint16_t identifier = *((uint16_t*)(data + 4));
        uint16_t sequence_number = *((uint16_t*)(data + 6));
        const uint8_t* echo_data = data + 8;
        uint32_t echo_data_size = size - 8;
        icmp_send_echo_request(src_ip, identifier, sequence_number, echo_data, echo_data_size); // Send reply
    }
}
