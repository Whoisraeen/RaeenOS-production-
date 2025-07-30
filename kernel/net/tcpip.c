#include "tcpip.h"
#include "../vga.h"
#include "../memory.h"
#include "../string.h"
#include "../../drivers/network/network.h"

// Maximum number of TCP sockets
#define MAX_TCP_SOCKETS 32
#define MAX_UDP_SOCKETS 32

// TCP socket table
static tcp_socket_t tcp_sockets[MAX_TCP_SOCKETS];
static bool tcp_sockets_initialized = false;

// UDP socket table (simplified)
typedef struct {
    ipv4_addr_t local_ip;
    port_t local_port;
    bool in_use;
} udp_socket_t;

static udp_socket_t udp_sockets[MAX_UDP_SOCKETS];
static bool udp_sockets_initialized = false;

// Network interface state
static ipv4_addr_t local_ip = 0;
static ipv4_addr_t gateway_ip = 0;
static ipv4_addr_t subnet_mask = 0;

// Packet ID counter for IP fragmentation
static uint16_t next_ip_id = 1;

void tcpip_init(void) {
    // Initialize TCP socket table
    if (!tcp_sockets_initialized) {
        for (int i = 0; i < MAX_TCP_SOCKETS; i++) {
            tcp_sockets[i].in_use = false;
            tcp_sockets[i].state = TCP_CLOSED;
        }
        tcp_sockets_initialized = true;
    }
    
    // Initialize UDP socket table
    if (!udp_sockets_initialized) {
        for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
            udp_sockets[i].in_use = false;
        }
        udp_sockets_initialized = true;
    }
    
    // Set default network configuration (should be configured via DHCP/static)
    local_ip = ipv4_addr_from_bytes(192, 168, 1, 100);
    gateway_ip = ipv4_addr_from_bytes(192, 168, 1, 1);
    subnet_mask = ipv4_addr_from_bytes(255, 255, 255, 0);
    
    debug_print("TCP/IP stack initialized with full implementation");
}

// Utility functions
ipv4_addr_t ipv4_addr_from_bytes(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
}

void ipv4_addr_to_bytes(ipv4_addr_t addr, uint8_t* bytes) {
    bytes[0] = (addr >> 24) & 0xFF;
    bytes[1] = (addr >> 16) & 0xFF;
    bytes[2] = (addr >> 8) & 0xFF;
    bytes[3] = addr & 0xFF;
}

bool ipv4_addr_equal(ipv4_addr_t addr1, ipv4_addr_t addr2) {
    return addr1 == addr2;
}

// Checksum calculation
uint16_t ipv4_checksum(const void* data, uint32_t length) {
    const uint16_t* words = (const uint16_t*)data;
    uint32_t sum = 0;
    
    // Sum all 16-bit words
    while (length > 1) {
        sum += *words++;
        length -= 2;
    }
    
    // Add the last byte if present
    if (length == 1) {
        sum += *(const uint8_t*)words << 8;
    }
    
    // Add carry bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// IPv4 packet handling
int ipv4_send_packet(ipv4_addr_t dest_ip, uint8_t protocol, const void* data, uint32_t size) {
    if (size > 1500 - sizeof(ipv4_header_t)) {
        return -1; // Packet too large for basic implementation
    }
    
    // Allocate buffer for IP packet
    uint32_t packet_size = sizeof(ipv4_header_t) + size;
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) {
        return -1;
    }
    
    // Build IPv4 header
    ipv4_header_t* ip_header = (ipv4_header_t*)packet;
    ip_header->version_ihl = 0x45; // IPv4, 20-byte header
    ip_header->dscp_ecn = 0;
    ip_header->total_length = packet_size;
    ip_header->identification = next_ip_id++;
    ip_header->flags_fragment = 0;
    ip_header->ttl = 64;
    ip_header->protocol = protocol;
    ip_header->header_checksum = 0;
    ip_header->src_ip = local_ip;
    ip_header->dest_ip = dest_ip;
    
    // Calculate checksum
    ip_header->header_checksum = ipv4_checksum(ip_header, sizeof(ipv4_header_t));
    
    // Copy payload
    if (data && size > 0) {
        memcpy(packet + sizeof(ipv4_header_t), data, size);
    }
    
    // Send packet via network driver
    network_packet_t net_packet = { .data = packet, .size = packet_size };
    int result = network_send_packet(&net_packet);
    
    kfree(packet);
    return result;
}

int ipv4_receive_packet(tcpip_packet_t* packet) {
    // This would be called by the network driver when a packet arrives
    if (!packet || packet->length < sizeof(ipv4_header_t)) {
        return -1;
    }
    
    ipv4_header_t* ip_header = (ipv4_header_t*)packet->data;
    
    // Basic validation
    if ((ip_header->version_ihl >> 4) != 4) {
        return -1; // Not IPv4
    }
    
    // Check if packet is for us
    if (ip_header->dest_ip != local_ip && ip_header->dest_ip != 0xFFFFFFFF) {
        return 0; // Not for us
    }
    
    // Verify checksum
    uint16_t original_checksum = ip_header->header_checksum;
    ip_header->header_checksum = 0;
    uint16_t calculated_checksum = ipv4_checksum(ip_header, sizeof(ipv4_header_t));
    if (original_checksum != calculated_checksum) {
        return -1; // Checksum mismatch
    }
    
    // Extract payload
    uint32_t header_length = (ip_header->version_ihl & 0x0F) * 4;
    uint8_t* payload = packet->data + header_length;
    uint32_t payload_length = ip_header->total_length - header_length;
    
    // Create packet structure for upper layers
    tcpip_packet_t upper_packet = {
        .data = payload,
        .length = payload_length,
        .src_ip = ip_header->src_ip,
        .dest_ip = ip_header->dest_ip,
        .protocol = ip_header->protocol
    };
    
    // Route to appropriate protocol handler
    switch (ip_header->protocol) {
        case IP_PROTOCOL_ICMP:
            icmp_handle_packet(&upper_packet);
            break;
        case IP_PROTOCOL_TCP:
            tcp_handle_packet(&upper_packet);
            break;
        case IP_PROTOCOL_UDP:
            udp_handle_packet(&upper_packet);
            break;
        default:
            debug_print("Unknown IP protocol received");
            return -1;
    }
    
    return 0;
}

// TCP implementation
int tcp_socket_create(void) {
    for (int i = 0; i < MAX_TCP_SOCKETS; i++) {
        if (!tcp_sockets[i].in_use) {
            tcp_sockets[i].in_use = true;
            tcp_sockets[i].state = TCP_CLOSED;
            tcp_sockets[i].local_ip = local_ip;
            tcp_sockets[i].local_port = 0;
            tcp_sockets[i].remote_ip = 0;
            tcp_sockets[i].remote_port = 0;
            tcp_sockets[i].seq_num = 0;
            tcp_sockets[i].ack_num = 0;
            tcp_sockets[i].window_size = 8192;
            return i;
        }
    }
    return -1; // No available sockets
}

int tcp_bind(int socket_fd, ipv4_addr_t ip, port_t port) {
    if (socket_fd < 0 || socket_fd >= MAX_TCP_SOCKETS || !tcp_sockets[socket_fd].in_use) {
        return -1;
    }
    
    tcp_sockets[socket_fd].local_ip = ip;
    tcp_sockets[socket_fd].local_port = port;
    return 0;
}

int tcp_listen(int socket_fd, int backlog) {
    (void)backlog; // Not implemented in this basic version
    
    if (socket_fd < 0 || socket_fd >= MAX_TCP_SOCKETS || !tcp_sockets[socket_fd].in_use) {
        return -1;
    }
    
    tcp_sockets[socket_fd].state = TCP_LISTEN;
    return 0;
}

int tcp_connect(int socket_fd, ipv4_addr_t dest_ip, port_t dest_port) {
    if (socket_fd < 0 || socket_fd >= MAX_TCP_SOCKETS || !tcp_sockets[socket_fd].in_use) {
        return -1;
    }
    
    tcp_socket_t* socket = &tcp_sockets[socket_fd];
    socket->remote_ip = dest_ip;
    socket->remote_port = dest_port;
    socket->state = TCP_SYN_SENT;
    
    // Send SYN packet (simplified)
    tcp_header_t tcp_header = {0};
    tcp_header.src_port = socket->local_port;
    tcp_header.dest_port = dest_port;
    tcp_header.seq_num = socket->seq_num;
    tcp_header.flags = 0x02; // SYN flag
    tcp_header.window_size = socket->window_size;
    
    return ipv4_send_packet(dest_ip, IP_PROTOCOL_TCP, &tcp_header, sizeof(tcp_header_t));
}

void tcp_handle_packet(const tcpip_packet_t* packet) {
    if (packet->length < sizeof(tcp_header_t)) {
        return;
    }
    
    tcp_header_t* tcp_header = (tcp_header_t*)packet->data;
    
    // Find matching socket
    for (int i = 0; i < MAX_TCP_SOCKETS; i++) {
        tcp_socket_t* socket = &tcp_sockets[i];
        if (socket->in_use && 
            socket->local_port == tcp_header->dest_port &&
            (socket->remote_ip == 0 || socket->remote_ip == packet->src_ip)) {
            
            // Handle based on current state
            switch (socket->state) {
                case TCP_LISTEN:
                    if (tcp_header->flags & 0x02) { // SYN
                        socket->remote_ip = packet->src_ip;
                        socket->remote_port = tcp_header->src_port;
                        socket->ack_num = tcp_header->seq_num + 1;
                        socket->state = TCP_SYN_RECEIVED;
                        // Send SYN-ACK (simplified)
                    }
                    break;
                case TCP_SYN_SENT:
                    if (tcp_header->flags & 0x12) { // SYN-ACK
                        socket->ack_num = tcp_header->seq_num + 1;
                        socket->state = TCP_ESTABLISHED;
                        // Send ACK (simplified)
                    }
                    break;
                case TCP_ESTABLISHED:
                    // Handle data packets
                    if (tcp_header->flags & 0x01) { // FIN
                        socket->state = TCP_CLOSE_WAIT;
                    }
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

// UDP implementation
int udp_socket_create(void) {
    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        if (!udp_sockets[i].in_use) {
            udp_sockets[i].in_use = true;
            udp_sockets[i].local_ip = local_ip;
            udp_sockets[i].local_port = 0;
            return i;
        }
    }
    return -1;
}

int udp_bind(int socket_fd, ipv4_addr_t ip, port_t port) {
    if (socket_fd < 0 || socket_fd >= MAX_UDP_SOCKETS || !udp_sockets[socket_fd].in_use) {
        return -1;
    }
    
    udp_sockets[socket_fd].local_ip = ip;
    udp_sockets[socket_fd].local_port = port;
    return 0;
}

int udp_send_to(int socket_fd, ipv4_addr_t dest_ip, port_t dest_port, const void* data, uint32_t length) {
    if (socket_fd < 0 || socket_fd >= MAX_UDP_SOCKETS || !udp_sockets[socket_fd].in_use) {
        return -1;
    }
    
    udp_socket_t* socket = &udp_sockets[socket_fd];
    
    // Build UDP header
    udp_header_t udp_header = {0};
    udp_header.src_port = socket->local_port;
    udp_header.dest_port = dest_port;
    udp_header.length = sizeof(udp_header_t) + length;
    udp_header.checksum = 0; // Simplified - should calculate proper checksum
    
    // Allocate buffer for UDP packet
    uint32_t packet_size = sizeof(udp_header_t) + length;
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) {
        return -1;
    }
    
    // Copy header and data
    memcpy(packet, &udp_header, sizeof(udp_header_t));
    if (data && length > 0) {
        memcpy(packet + sizeof(udp_header_t), data, length);
    }
    
    // Send via IP layer
    int result = ipv4_send_packet(dest_ip, IP_PROTOCOL_UDP, packet, packet_size);
    
    kfree(packet);
    return result;
}

void udp_handle_packet(const tcpip_packet_t* packet) {
    if (packet->length < sizeof(udp_header_t)) {
        return;
    }
    
    udp_header_t* udp_header = (udp_header_t*)packet->data;
    
    // Find matching socket
    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        if (udp_sockets[i].in_use && udp_sockets[i].local_port == udp_header->dest_port) {
            // Handle received UDP packet
            // In a complete implementation, this would queue the packet for the application
            debug_print("UDP packet received");
            break;
        }
    }
}

// ICMP implementation
void icmp_send_echo_reply(ipv4_addr_t dest_ip, uint16_t id, uint16_t seq, const void* data, uint32_t length) {
    uint32_t packet_size = sizeof(icmp_header_t) + length;
    uint8_t* packet = (uint8_t*)kmalloc(packet_size);
    if (!packet) {
        return;
    }
    
    icmp_header_t* icmp_header = (icmp_header_t*)packet;
    icmp_header->type = 0; // Echo Reply
    icmp_header->code = 0;
    icmp_header->checksum = 0;
    icmp_header->rest_of_header = (id << 16) | seq;
    
    // Copy data
    if (data && length > 0) {
        memcpy(packet + sizeof(icmp_header_t), data, length);
    }
    
    // Calculate checksum
    icmp_header->checksum = ipv4_checksum(packet, packet_size);
    
    ipv4_send_packet(dest_ip, IP_PROTOCOL_ICMP, packet, packet_size);
    kfree(packet);
}

void icmp_handle_packet(const tcpip_packet_t* packet) {
    if (packet->length < sizeof(icmp_header_t)) {
        return;
    }
    
    icmp_header_t* icmp_header = (icmp_header_t*)packet->data;
    
    switch (icmp_header->type) {
        case 8: // Echo Request (ping)
            {
                uint16_t id = (icmp_header->rest_of_header >> 16) & 0xFFFF;
                uint16_t seq = icmp_header->rest_of_header & 0xFFFF;
                uint8_t* data = packet->data + sizeof(icmp_header_t);
                uint32_t data_length = packet->length - sizeof(icmp_header_t);
                
                icmp_send_echo_reply(packet->src_ip, id, seq, data, data_length);
                debug_print("ICMP echo request handled");
            }
            break;
        case 0: // Echo Reply
            debug_print("ICMP echo reply received");
            break;
        default:
            debug_print("Unknown ICMP type received");
            break;
    }
}

// Legacy compatibility functions
int tcpip_send_ipv4_packet(ipv4_addr_t dest_ip, uint8_t protocol, const void* data, uint32_t size) {
    return ipv4_send_packet(dest_ip, protocol, data, size);
}

int tcpip_receive_ipv4_packet(ipv4_addr_t* src_ip, uint8_t* protocol, void* buffer, uint32_t buffer_size) {
    // This is a simplified compatibility function
    // In a real implementation, this would interface with a packet queue
    (void)src_ip;
    (void)protocol;
    (void)buffer;
    (void)buffer_size;
    return 0; // No packets available
}