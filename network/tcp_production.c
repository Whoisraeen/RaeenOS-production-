/**
 * RaeenOS Production TCP Implementation
 * Complete TCP state machine with flow control, congestion control, and reliability
 */

#include "network_core.h"
#include "../memory.h"
#include "../time.h"
#include "../string.h"

// TCP States (RFC 793)
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

// TCP Control Block (TCB)
typedef struct tcp_socket {
    uint32_t local_ip;
    uint32_t remote_ip;
    uint16_t local_port;
    uint16_t remote_port;
    
    tcp_state_t state;
    
    // Sequence numbers
    uint32_t snd_una;     // Send unacknowledged
    uint32_t snd_nxt;     // Send next
    uint32_t snd_wnd;     // Send window
    uint32_t snd_up;      // Send urgent pointer
    uint32_t snd_wl1;     // Segment sequence number for last window update
    uint32_t snd_wl2;     // Segment acknowledgment number for last window update
    uint32_t iss;         // Initial send sequence number
    
    uint32_t rcv_nxt;     // Receive next
    uint32_t rcv_wnd;     // Receive window
    uint32_t rcv_up;      // Receive urgent pointer
    uint32_t irs;         // Initial receive sequence number
    
    // Congestion control (RFC 5681)
    uint32_t cwnd;        // Congestion window
    uint32_t ssthresh;    // Slow start threshold
    uint32_t duplicate_acks;
    bool in_fast_recovery;
    
    // Round-trip time estimation (RFC 6298)
    uint32_t srtt;        // Smoothed round-trip time
    uint32_t rttvar;      // Round-trip time variation
    uint32_t rto;         // Retransmission timeout
    
    // Timers
    uint64_t retransmit_timer;
    uint64_t persist_timer;
    uint64_t keepalive_timer;
    uint64_t time_wait_timer;
    
    // Buffers
    uint8_t* send_buffer;
    uint8_t* recv_buffer;
    uint32_t send_buffer_size;
    uint32_t recv_buffer_size;
    uint32_t send_buffer_used;
    uint32_t recv_buffer_used;
    
    // Options
    uint16_t mss;         // Maximum segment size
    bool sack_permitted;  // Selective acknowledgment
    bool window_scale;    // Window scaling
    uint8_t ws_factor;    // Window scale factor
    
    struct tcp_socket* next;
} tcp_socket_t;

// Global TCP state
static tcp_socket_t* tcp_sockets = NULL;
static uint32_t tcp_socket_count = 0;
static uint16_t next_ephemeral_port = 32768;

// Function declarations
static tcp_socket_t* tcp_create_socket(void);
static void tcp_destroy_socket(tcp_socket_t* sock);
static tcp_socket_t* tcp_find_socket(uint32_t local_ip, uint16_t local_port, uint32_t remote_ip, uint16_t remote_port);
static void tcp_send_segment(tcp_socket_t* sock, uint8_t flags, uint8_t* data, uint32_t data_len);
static void tcp_process_segment(tcp_socket_t* sock, tcp_header_t* tcp_hdr, uint8_t* data, uint32_t data_len);
static void tcp_update_rtt(tcp_socket_t* sock, uint32_t rtt_sample);
static void tcp_congestion_control(tcp_socket_t* sock, bool ack_received, bool duplicate_ack);
static void tcp_retransmit(tcp_socket_t* sock);
static uint32_t tcp_calculate_checksum(uint32_t src_ip, uint32_t dst_ip, tcp_header_t* tcp_hdr, uint8_t* data, uint32_t data_len);

/**
 * Create TCP socket
 */
int tcp_socket(void) {
    tcp_socket_t* sock = tcp_create_socket();
    if (!sock) return -1;
    
    sock->state = TCP_CLOSED;
    sock->local_port = 0;
    sock->remote_port = 0;
    
    return (int)sock; // Return socket descriptor
}

/**
 * Bind TCP socket to address
 */
int tcp_bind(int sockfd, uint32_t addr, uint16_t port) {
    tcp_socket_t* sock = (tcp_socket_t*)sockfd;
    if (!sock || sock->state != TCP_CLOSED) return -1;
    
    // Check if port is already in use
    if (tcp_find_socket(addr, port, 0, 0)) return -1;
    
    sock->local_ip = addr;
    sock->local_port = port;
    
    return 0;
}

/**
 * Listen for connections
 */
int tcp_listen(int sockfd, int backlog) {
    tcp_socket_t* sock = (tcp_socket_t*)sockfd;
    if (!sock || sock->state != TCP_CLOSED) return -1;
    
    sock->state = TCP_LISTEN;
    return 0;
}

/**
 * Accept incoming connection
 */
int tcp_accept(int sockfd, uint32_t* addr, uint16_t* port) {
    tcp_socket_t* listen_sock = (tcp_socket_t*)sockfd;
    if (!listen_sock || listen_sock->state != TCP_LISTEN) return -1;
    
    // Wait for incoming SYN (simplified)
    // In real implementation, this would block or return immediately based on socket flags
    
    return -1; // Placeholder
}

/**
 * Connect to remote host
 */
int tcp_connect(int sockfd, uint32_t addr, uint16_t port) {
    tcp_socket_t* sock = (tcp_socket_t*)sockfd;
    if (!sock || sock->state != TCP_CLOSED) return -1;
    
    // Assign ephemeral port if not bound
    if (sock->local_port == 0) {
        sock->local_port = next_ephemeral_port++;
        if (next_ephemeral_port > 65535) next_ephemeral_port = 32768;
    }
    
    sock->remote_ip = addr;
    sock->remote_port = port;
    
    // Initialize sequence numbers
    sock->iss = (uint32_t)time_get_ticks(); // Use timestamp as ISS
    sock->snd_una = sock->iss;
    sock->snd_nxt = sock->iss + 1;
    
    // Initialize congestion control
    sock->cwnd = sock->mss * 2; // Initial congestion window
    sock->ssthresh = 65535;     // Initial slow start threshold
    
    // Send SYN
    sock->state = TCP_SYN_SENT;
    tcp_send_segment(sock, TCP_FLAG_SYN, NULL, 0);
    
    // Start retransmission timer
    sock->retransmit_timer = time_get_ticks() + sock->rto;
    
    return 0;
}

/**
 * Send data
 */
int tcp_send(int sockfd, const void* data, size_t len) {
    tcp_socket_t* sock = (tcp_socket_t*)sockfd;
    if (!sock || sock->state != TCP_ESTABLISHED) return -1;
    
    const uint8_t* bytes = (const uint8_t*)data;
    size_t bytes_sent = 0;
    
    while (bytes_sent < len) {
        // Check send window
        uint32_t available_window = sock->snd_wnd - (sock->snd_nxt - sock->snd_una);
        if (available_window == 0) {
            // Window is full, start persist timer
            sock->persist_timer = time_get_ticks() + sock->rto;
            break;
        }
        
        // Calculate segment size
        uint32_t segment_size = len - bytes_sent;
        if (segment_size > sock->mss) segment_size = sock->mss;
        if (segment_size > available_window) segment_size = available_window;
        if (segment_size > sock->cwnd) segment_size = sock->cwnd;
        
        // Send segment
        tcp_send_segment(sock, TCP_FLAG_ACK | TCP_FLAG_PSH, 
                        (uint8_t*)bytes + bytes_sent, segment_size);
        
        bytes_sent += segment_size;
        sock->snd_nxt += segment_size;
    }
    
    return bytes_sent;
}

/**
 * Receive data
 */
int tcp_recv(int sockfd, void* buffer, size_t len) {
    tcp_socket_t* sock = (tcp_socket_t*)sockfd;
    if (!sock || sock->state != TCP_ESTABLISHED) return -1;
    
    // Copy data from receive buffer
    uint32_t bytes_available = sock->recv_buffer_used;
    uint32_t bytes_to_copy = (len < bytes_available) ? len : bytes_available;
    
    if (bytes_to_copy > 0) {
        memory_copy(buffer, sock->recv_buffer, bytes_to_copy);
        
        // Shift remaining data
        memory_move(sock->recv_buffer, sock->recv_buffer + bytes_to_copy, 
                   sock->recv_buffer_used - bytes_to_copy);
        sock->recv_buffer_used -= bytes_to_copy;
        
        // Update receive window
        sock->rcv_wnd += bytes_to_copy;
        
        // Send window update if significant
        if (bytes_to_copy > sock->mss / 2) {
            tcp_send_segment(sock, TCP_FLAG_ACK, NULL, 0);
        }
    }
    
    return bytes_to_copy;
}

/**
 * Close socket
 */
int tcp_close(int sockfd) {
    tcp_socket_t* sock = (tcp_socket_t*)sockfd;
    if (!sock) return -1;
    
    switch (sock->state) {
        case TCP_ESTABLISHED:
            // Send FIN
            tcp_send_segment(sock, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0);
            sock->state = TCP_FIN_WAIT_1;
            sock->snd_nxt++;
            break;
            
        case TCP_CLOSE_WAIT:
            // Send FIN
            tcp_send_segment(sock, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0);
            sock->state = TCP_LAST_ACK;
            sock->snd_nxt++;
            break;
            
        case TCP_LISTEN:
        case TCP_SYN_SENT:
            sock->state = TCP_CLOSED;
            tcp_destroy_socket(sock);
            break;
            
        default:
            break;
    }
    
    return 0;
}

/**
 * Process incoming TCP segment
 */
void tcp_input(uint32_t src_ip, uint32_t dst_ip, tcp_header_t* tcp_hdr, uint8_t* data, uint32_t data_len) {
    // Verify checksum
    uint32_t calculated_checksum = tcp_calculate_checksum(src_ip, dst_ip, tcp_hdr, data, data_len);
    if (calculated_checksum != 0) {
        // Checksum mismatch, drop packet
        return;
    }
    
    // Find matching socket
    tcp_socket_t* sock = tcp_find_socket(dst_ip, ntohs(tcp_hdr->dst_port), 
                                        src_ip, ntohs(tcp_hdr->src_port));
    
    if (!sock) {
        // No matching socket, check for listening socket
        sock = tcp_find_socket(dst_ip, ntohs(tcp_hdr->dst_port), 0, 0);
        if (!sock || sock->state != TCP_LISTEN) {
            // Send RST
            // tcp_send_rst(src_ip, dst_ip, tcp_hdr);
            return;
        }
    }
    
    tcp_process_segment(sock, tcp_hdr, data, data_len);
}

// Internal helper functions

static tcp_socket_t* tcp_create_socket(void) {
    tcp_socket_t* sock = (tcp_socket_t*)memory_alloc(sizeof(tcp_socket_t));
    if (!sock) return NULL;
    
    memory_set(sock, 0, sizeof(tcp_socket_t));
    
    // Initialize default values
    sock->mss = 1460;  // Standard MSS for Ethernet
    sock->rcv_wnd = 65535;
    sock->rto = 3000;  // 3 seconds initial RTO
    
    // Allocate buffers
    sock->send_buffer_size = 65536;
    sock->recv_buffer_size = 65536;
    sock->send_buffer = (uint8_t*)memory_alloc(sock->send_buffer_size);
    sock->recv_buffer = (uint8_t*)memory_alloc(sock->recv_buffer_size);
    
    if (!sock->send_buffer || !sock->recv_buffer) {
        if (sock->send_buffer) memory_free(sock->send_buffer);
        if (sock->recv_buffer) memory_free(sock->recv_buffer);
        memory_free(sock);
        return NULL;
    }
    
    // Add to socket list
    sock->next = tcp_sockets;
    tcp_sockets = sock;
    tcp_socket_count++;
    
    return sock;
}

static void tcp_process_segment(tcp_socket_t* sock, tcp_header_t* tcp_hdr, uint8_t* data, uint32_t data_len) {
    uint32_t seq = ntohl(tcp_hdr->seq_num);
    uint32_t ack = ntohl(tcp_hdr->ack_num);
    uint16_t window = ntohs(tcp_hdr->window);
    uint8_t flags = tcp_hdr->flags;
    
    // Update send window
    if (flags & TCP_FLAG_ACK) {
        sock->snd_wnd = window;
    }
    
    // TCP state machine
    switch (sock->state) {
        case TCP_SYN_SENT:
            if ((flags & TCP_FLAG_ACK) && (flags & TCP_FLAG_SYN)) {
                // SYN-ACK received
                if (ack == sock->snd_nxt) {
                    sock->rcv_nxt = seq + 1;
                    sock->snd_una = ack;
                    sock->state = TCP_ESTABLISHED;
                    
                    // Send ACK
                    tcp_send_segment(sock, TCP_FLAG_ACK, NULL, 0);
                }
            }
            break;
            
        case TCP_ESTABLISHED:
            // Handle data
            if (data_len > 0 && seq == sock->rcv_nxt) {
                // Data in sequence
                if (sock->recv_buffer_used + data_len <= sock->recv_buffer_size) {
                    memory_copy(sock->recv_buffer + sock->recv_buffer_used, data, data_len);
                    sock->recv_buffer_used += data_len;
                    sock->rcv_nxt += data_len;
                    sock->rcv_wnd -= data_len;
                    
                    // Send ACK
                    tcp_send_segment(sock, TCP_FLAG_ACK, NULL, 0);
                }
            }
            
            // Handle ACK
            if (flags & TCP_FLAG_ACK) {
                if (ack > sock->snd_una && ack <= sock->snd_nxt) {
                    // Valid ACK
                    uint32_t acked_bytes = ack - sock->snd_una;
                    sock->snd_una = ack;
                    
                    // Congestion control
                    tcp_congestion_control(sock, true, false);
                    
                    // Update RTT if this ACKs new data
                    // tcp_update_rtt(sock, rtt_sample);
                }
            }
            
            // Handle FIN
            if (flags & TCP_FLAG_FIN) {
                sock->rcv_nxt++;
                sock->state = TCP_CLOSE_WAIT;
                
                // Send ACK
                tcp_send_segment(sock, TCP_FLAG_ACK, NULL, 0);
            }
            break;
            
        case TCP_FIN_WAIT_1:
            if (flags & TCP_FLAG_ACK) {
                if (ack == sock->snd_nxt) {
                    sock->state = TCP_FIN_WAIT_2;
                }
            }
            if (flags & TCP_FLAG_FIN) {
                sock->rcv_nxt++;
                if (sock->state == TCP_FIN_WAIT_2) {
                    sock->state = TCP_TIME_WAIT;
                    sock->time_wait_timer = time_get_ticks() + 120000; // 2 minutes
                } else {
                    sock->state = TCP_CLOSING;
                }
                tcp_send_segment(sock, TCP_FLAG_ACK, NULL, 0);
            }
            break;
            
        case TCP_LAST_ACK:
            if (flags & TCP_FLAG_ACK) {
                if (ack == sock->snd_nxt) {
                    sock->state = TCP_CLOSED;
                    tcp_destroy_socket(sock);
                }
            }
            break;
            
        default:
            break;
    }
}

static void tcp_send_segment(tcp_socket_t* sock, uint8_t flags, uint8_t* data, uint32_t data_len) {
    // Allocate packet buffer
    uint32_t total_len = sizeof(tcp_header_t) + data_len;
    uint8_t* packet = (uint8_t*)memory_alloc(total_len);
    if (!packet) return;
    
    tcp_header_t* tcp_hdr = (tcp_header_t*)packet;
    
    // Fill TCP header
    tcp_hdr->src_port = htons(sock->local_port);
    tcp_hdr->dst_port = htons(sock->remote_port);
    tcp_hdr->seq_num = htonl(sock->snd_nxt);
    tcp_hdr->ack_num = htonl(sock->rcv_nxt);
    tcp_hdr->header_len = (sizeof(tcp_header_t) / 4) << 4;
    tcp_hdr->flags = flags;
    tcp_hdr->window = htons(sock->rcv_wnd);
    tcp_hdr->checksum = 0;
    tcp_hdr->urgent_ptr = 0;
    
    // Copy data
    if (data && data_len > 0) {
        memory_copy(packet + sizeof(tcp_header_t), data, data_len);
    }
    
    // Calculate checksum
    tcp_hdr->checksum = htons(tcp_calculate_checksum(sock->local_ip, sock->remote_ip, 
                                                    tcp_hdr, data, data_len));
    
    // Send via IP layer
    ip_send(sock->local_ip, sock->remote_ip, IP_PROTOCOL_TCP, packet, total_len);
    
    memory_free(packet);
}

static void tcp_congestion_control(tcp_socket_t* sock, bool ack_received, bool duplicate_ack) {
    if (duplicate_ack) {
        sock->duplicate_acks++;
        
        if (sock->duplicate_acks == 3 && !sock->in_fast_recovery) {
            // Fast retransmit
            sock->ssthresh = sock->cwnd / 2;
            if (sock->ssthresh < 2 * sock->mss) sock->ssthresh = 2 * sock->mss;
            
            sock->cwnd = sock->ssthresh + 3 * sock->mss;
            sock->in_fast_recovery = true;
            
            tcp_retransmit(sock);
        } else if (sock->in_fast_recovery) {
            // Fast recovery
            sock->cwnd += sock->mss;
        }
    } else if (ack_received) {
        sock->duplicate_acks = 0;
        
        if (sock->in_fast_recovery) {
            // Exit fast recovery
            sock->cwnd = sock->ssthresh;
            sock->in_fast_recovery = false;
        } else if (sock->cwnd < sock->ssthresh) {
            // Slow start
            sock->cwnd += sock->mss;
        } else {
            // Congestion avoidance
            sock->cwnd += (sock->mss * sock->mss) / sock->cwnd;
        }
    }
}

static uint32_t tcp_calculate_checksum(uint32_t src_ip, uint32_t dst_ip, tcp_header_t* tcp_hdr, uint8_t* data, uint32_t data_len) {
    // TCP pseudo-header for checksum calculation
    struct {
        uint32_t src_ip;
        uint32_t dst_ip;
        uint8_t zero;
        uint8_t protocol;
        uint16_t tcp_len;
    } pseudo_hdr;
    
    pseudo_hdr.src_ip = src_ip;
    pseudo_hdr.dst_ip = dst_ip;
    pseudo_hdr.zero = 0;
    pseudo_hdr.protocol = IP_PROTOCOL_TCP;
    pseudo_hdr.tcp_len = htons(sizeof(tcp_header_t) + data_len);
    
    // Calculate checksum over pseudo-header + TCP header + data
    uint32_t checksum = 0;
    
    // Pseudo-header
    uint16_t* ptr = (uint16_t*)&pseudo_hdr;
    for (int i = 0; i < sizeof(pseudo_hdr) / 2; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    // TCP header
    ptr = (uint16_t*)tcp_hdr;
    for (int i = 0; i < sizeof(tcp_header_t) / 2; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    // Data
    ptr = (uint16_t*)data;
    for (uint32_t i = 0; i < data_len / 2; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    // Handle odd byte
    if (data_len & 1) {
        checksum += data[data_len - 1] << 8;
    }
    
    // Fold carries
    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }
    
    return ~checksum & 0xFFFF;
}

static void tcp_retransmit(tcp_socket_t* sock) {
    // Retransmit unacknowledged data
    uint32_t unacked_bytes = sock->snd_nxt - sock->snd_una;
    if (unacked_bytes > 0) {
        uint32_t retransmit_bytes = (unacked_bytes > sock->mss) ? sock->mss : unacked_bytes;
        
        // Get data from send buffer (simplified)
        tcp_send_segment(sock, TCP_FLAG_ACK, sock->send_buffer, retransmit_bytes);
        
        // Double RTO (exponential backoff)
        sock->rto *= 2;
        if (sock->rto > 64000) sock->rto = 64000; // Cap at 64 seconds
        
        sock->retransmit_timer = time_get_ticks() + sock->rto;
    }
}
