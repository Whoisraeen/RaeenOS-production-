#include "icmp.h"
#include "ipv4.h"
#include "../vga.h"
#include "../libs/libc/include/string.h"

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
    // Type (8), Code (0), Checksum, Identifier, Sequence Number
    icmp_packet[0] = ICMP_ECHO_REQUEST;
    icmp_packet[1] = 0;
    icmp_packet[2] = 0; // Checksum high
    icmp_packet[3] = 0; // Checksum low
    icmp_packet[4] = (identifier >> 8) & 0xFF;
    icmp_packet[5] = identifier & 0xFF;
    icmp_packet[6] = (sequence_number >> 8) & 0xFF;
    icmp_packet[7] = sequence_number & 0xFF;
    memcpy(icmp_packet + 8, data, size);

    // TODO: Calculate and set checksum

    return ipv4_send_packet(dest_ip, 1, icmp_packet, size + 8); // Protocol 1 for ICMP
}

void icmp_handle_ipv4_packet(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size) {
    if (protocol != 1) return; // Not an ICMP packet

    // Simulate parsing ICMP header
    uint8_t type = data[0];
    uint8_t code = data[1];
    uint16_t checksum = (data[2] << 8) | data[3];

    debug_print("ICMP: Received packet from ");
    vga_put_hex(src_ip);
    debug_print(" (Type: ");
    vga_put_dec(type);
    debug_print(", Code: ");
    vga_put_dec(code);
    debug_print(", Checksum: ");
    vga_put_hex(checksum);
    debug_print(")\n");

    // TODO: Verify checksum

    switch (type) {
        case ICMP_ECHO_REQUEST:
            debug_print("ICMP: Responding to Echo Request.\n");
            // Simulate sending an Echo Reply
            uint16_t identifier = (data[4] << 8) | data[5];
            uint16_t sequence_number = (data[6] << 8) | data[7];
            const uint8_t* echo_data = data + 8;
            uint32_t echo_data_size = size - 8;

            // Construct Echo Reply
            uint8_t reply_packet[2048];
            reply_packet[0] = ICMP_ECHO_REPLY; // Type
            reply_packet[1] = 0; // Code
            reply_packet[2] = 0; // Checksum high
            reply_packet[3] = 0; // Checksum low
            reply_packet[4] = (identifier >> 8) & 0xFF;
            reply_packet[5] = identifier & 0xFF;
            reply_packet[6] = (sequence_number >> 8) & 0xFF;
            reply_packet[7] = sequence_number & 0xFF;
            memcpy(reply_packet + 8, echo_data, echo_data_size);

            // TODO: Calculate and set checksum

            ipv4_send_packet(src_ip, 1, reply_packet, echo_data_size + 8); // Send reply
            break;
        case ICMP_DEST_UNREACH:
            debug_print("ICMP: Destination Unreachable received.\n");
            // Handle unreachable destination
            break;
        case ICMP_TIME_EXCEEDED:
            debug_print("ICMP: Time Exceeded received.\n");
            // Handle time exceeded
            break;
        default:
            debug_print("ICMP: Unknown message type.\n");
            break;
    }
}
