#ifndef TCP_H
#define TCP_H

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.h"

// TCP connection states
typedef enum {
    TCP_STATE_CLOSED,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSING,
    TCP_STATE_TIME_WAIT,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_LAST_ACK
} tcp_state_t;

// TCP connection structure (placeholder)
typedef struct {
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    tcp_state_t state;
    uint32_t sequence_number; // Our next sequence number
    uint32_t acknowledge_number; // Next sequence number we expect from remote
    uint16_t window_size; // Our receive window size
    uint8_t* receive_buffer; // Buffer for incoming data
    uint32_t receive_buffer_size;
    uint32_t receive_buffer_head;
    uint32_t receive_buffer_tail;
    // Add more for retransmission, flow control, congestion control
} tcp_connection_t;

// Initialize the TCP stack
void tcp_init(void);

// Establish a TCP connection
tcp_connection_t* tcp_connect(ipv4_addr_t remote_ip, uint16_t remote_port, uint16_t local_port);

// Listen for incoming TCP connections
tcp_connection_t* tcp_listen(uint16_t port);

// Send data over a TCP connection
int tcp_send(tcp_connection_t* conn, const uint8_t* data, uint32_t size);

// Receive data from a TCP connection
int tcp_receive(tcp_connection_t* conn, uint8_t* buffer, uint32_t buffer_size);

// Close a TCP connection
int tcp_close(tcp_connection_t* conn);

// Handle incoming TCP segment from IP layer
void tcp_handle_ipv4_packet(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size);

#endif // TCP_H
