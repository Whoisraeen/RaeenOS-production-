#ifndef TCPIP_H
#define TCPIP_H

#include "../include/types.h"

// Protocol numbers
#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6  
#define IP_PROTOCOL_UDP     17

// IP address structures
typedef uint32_t ipv4_addr_t;
typedef uint16_t port_t;

// IPv4 header structure
typedef struct PACKED {
    uint8_t  version_ihl;      // Version (4 bits) + IHL (4 bits)
    uint8_t  dscp_ecn;         // DSCP (6 bits) + ECN (2 bits)
    uint16_t total_length;     // Total length in bytes
    uint16_t identification;   // Identification
    uint16_t flags_fragment;   // Flags (3 bits) + Fragment offset (13 bits)
    uint8_t  ttl;             // Time to live
    uint8_t  protocol;        // Next level protocol
    uint16_t header_checksum; // Header checksum
    ipv4_addr_t src_ip;       // Source IP address
    ipv4_addr_t dest_ip;      // Destination IP address
} ipv4_header_t;

// TCP header structure
typedef struct PACKED {
    port_t   src_port;        // Source port
    port_t   dest_port;       // Destination port
    uint32_t seq_num;         // Sequence number
    uint32_t ack_num;         // Acknowledgment number
    uint8_t  data_offset_flags; // Data offset (4 bits) + Reserved (3 bits) + Flags (9 bits)
    uint8_t  flags;           // Control flags
    uint16_t window_size;     // Window size
    uint16_t checksum;        // Checksum
    uint16_t urgent_ptr;      // Urgent pointer
} tcp_header_t;

// UDP header structure
typedef struct PACKED {
    port_t   src_port;        // Source port
    port_t   dest_port;       // Destination port
    uint16_t length;          // Length
    uint16_t checksum;        // Checksum
} udp_header_t;

// ICMP header structure
typedef struct PACKED {
    uint8_t  type;            // ICMP type
    uint8_t  code;            // ICMP code
    uint16_t checksum;        // Checksum
    uint32_t rest_of_header;  // Varies by ICMP type
} icmp_header_t;

// TCP connection states
typedef enum {
    TCP_CLOSED = 0,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,    
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

// TCP socket structure
typedef struct {
    ipv4_addr_t local_ip;
    port_t local_port;
    ipv4_addr_t remote_ip;
    port_t remote_port;
    tcp_state_t state;
    uint32_t seq_num;
    uint32_t ack_num;
    uint16_t window_size;
    bool in_use;
} tcp_socket_t;

// TCP/IP packet structure (more detailed than basic network packet)
typedef struct {
    uint8_t* data;
    uint32_t length;
    ipv4_addr_t src_ip;
    ipv4_addr_t dest_ip;
    uint8_t protocol;
} tcpip_packet_t;

// Initialize the TCP/IP stack
void tcpip_init(void);

// IPv4 functions
int ipv4_send_packet(ipv4_addr_t dest_ip, uint8_t protocol, const void* data, uint32_t size);
int ipv4_receive_packet(tcpip_packet_t* packet);
uint16_t ipv4_checksum(const void* data, uint32_t length);

// TCP functions
int tcp_socket_create(void);
int tcp_bind(int socket_fd, ipv4_addr_t ip, port_t port);
int tcp_listen(int socket_fd, int backlog);
int tcp_connect(int socket_fd, ipv4_addr_t dest_ip, port_t dest_port);
int tcp_accept(int socket_fd);
int tcp_send(int socket_fd, const void* data, uint32_t length);
int tcp_receive(int socket_fd, void* buffer, uint32_t buffer_size);
int tcp_close(int socket_fd);
void tcp_handle_packet(const tcpip_packet_t* packet);

// UDP functions
int udp_socket_create(void);
int udp_bind(int socket_fd, ipv4_addr_t ip, port_t port);
int udp_send_to(int socket_fd, ipv4_addr_t dest_ip, port_t dest_port, const void* data, uint32_t length);
int udp_receive_from(int socket_fd, void* buffer, uint32_t buffer_size, ipv4_addr_t* src_ip, port_t* src_port);
void udp_handle_packet(const tcpip_packet_t* packet);

// ICMP functions
void icmp_send_echo_reply(ipv4_addr_t dest_ip, uint16_t id, uint16_t seq, const void* data, uint32_t length);
void icmp_handle_packet(const tcpip_packet_t* packet);

// Utility functions
ipv4_addr_t ipv4_addr_from_bytes(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void ipv4_addr_to_bytes(ipv4_addr_t addr, uint8_t* bytes);
bool ipv4_addr_equal(ipv4_addr_t addr1, ipv4_addr_t addr2);

// Legacy compatibility
int tcpip_send_ipv4_packet(ipv4_addr_t dest_ip, uint8_t protocol, const void* data, uint32_t size);
int tcpip_receive_ipv4_packet(ipv4_addr_t* src_ip, uint8_t* protocol, void* buffer, uint32_t buffer_size);

#endif // TCPIP_H
