#include "tcp.h"
#include "../vga.h"
#include "../memory.h"
#include "../libs/libc/include/string.h"
#include "ipv4.h"

#define TCP_MAX_CONNECTIONS 16
#define TCP_RECEIVE_BUFFER_SIZE 2048
#define TCP_SEND_BUFFER_SIZE 2048
#define TCP_RETRANSMISSION_TIMEOUT_MS 5000 // 5 seconds

static tcp_connection_t* tcp_connections[TCP_MAX_CONNECTIONS];
static uint32_t next_tcp_port = 49152; // Ephemeral port range

void tcp_init(void) {
    debug_print("TCP stack initialized (placeholder).\n");
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        tcp_connections[i] = NULL;
    }
    ipv4_register_receive_callback(6, tcp_handle_ipv4_packet); // Protocol 6 for TCP
}

tcp_connection_t* tcp_connect(ipv4_addr_t remote_ip, uint16_t remote_port, uint16_t local_port) {
    debug_print("TCP: Connecting to ");
    vga_put_hex(remote_ip);
    debug_print(":");
    vga_put_dec(remote_port);
    debug_print(" from local port ");
    vga_put_dec(local_port);
    debug_print(" (simulated).\n");

    tcp_connection_t* conn = (tcp_connection_t*)kmalloc(sizeof(tcp_connection_t));
    if (!conn) return NULL;

    memset(conn, 0, sizeof(tcp_connection_t));
    conn->local_ip = 0; // TODO: Get actual local IP
    conn->local_port = (local_port == 0) ? next_tcp_port++ : local_port;
    conn->remote_ip = remote_ip;
    conn->remote_port = remote_port;
    conn->state = TCP_STATE_SYN_SENT; // Simulate SYN_SENT
    conn->sequence_number = 0x12345678; // Initial sequence number
    conn->acknowledge_number = 0;
    conn->window_size = TCP_RECEIVE_BUFFER_SIZE;
    conn->receive_buffer = (uint8_t*)kmalloc(TCP_RECEIVE_BUFFER_SIZE);
    if (!conn->receive_buffer) {
        kfree(conn);
        return NULL;
    }
    conn->receive_buffer_head = 0;
    conn->receive_buffer_tail = 0;

    conn->send_buffer = (uint8_t*)kmalloc(TCP_SEND_BUFFER_SIZE);
    if (!conn->send_buffer) {
        kfree(conn->receive_buffer);
        kfree(conn);
        return NULL;
    }
    conn->send_buffer_head = 0;
    conn->send_buffer_tail = 0;
    conn->retransmission_timer = 0;

    // Add to connection list
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        if (tcp_connections[i] == NULL) {
            tcp_connections[i] = conn;
            break;
        }
    }

    // Simulate sending SYN packet
    // In a real implementation, this would construct a TCP SYN segment and pass to IP layer.
    // For now, directly transition to ESTABLISHED for simulation.
    conn->state = TCP_STATE_ESTABLISHED;
    debug_print("TCP: Connection established (simulated).\n");

    return conn;
}

tcp_connection_t* tcp_listen(uint16_t port) {
    debug_print("TCP: Listening on port ");
    vga_put_dec(port);
    debug_print(" (simulated).\n");

    tcp_connection_t* conn = (tcp_connection_t*)kmalloc(sizeof(tcp_connection_t));
    if (!conn) return NULL;

    memset(conn, 0, sizeof(tcp_connection_t));
    conn->local_ip = 0; // TODO: Get actual local IP
    conn->local_port = port;
    conn->remote_ip = 0;
    conn->remote_port = 0;
    conn->state = TCP_STATE_LISTEN;
    conn->sequence_number = 0x87654321; // Initial sequence number
    conn->acknowledge_number = 0;
    conn->window_size = TCP_RECEIVE_BUFFER_SIZE;
    conn->receive_buffer = (uint8_t*)kmalloc(TCP_RECEIVE_BUFFER_SIZE);
    if (!conn->receive_buffer) {
        kfree(conn);
        return NULL;
    }
    conn->receive_buffer_head = 0;
    conn->receive_buffer_tail = 0;

    conn->send_buffer = (uint8_t*)kmalloc(TCP_SEND_BUFFER_SIZE);
    if (!conn->send_buffer) {
        kfree(conn->receive_buffer);
        kfree(conn);
        return NULL;
    }
    conn->send_buffer_head = 0;
    conn->send_buffer_tail = 0;
    conn->retransmission_timer = 0;

    // Add to connection list
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        if (tcp_connections[i] == NULL) {
            tcp_connections[i] = conn;
            break;
        }
    }

    return conn;
}

int tcp_send(tcp_connection_t* conn, const uint8_t* data, uint32_t size) {
    if (!conn || conn->state != TCP_STATE_ESTABLISHED) return -1;
    debug_print("TCP: Sending ");
    vga_put_dec(size);
    debug_print(" bytes to ");
    vga_put_hex(conn->remote_ip);
    debug_print(":");
    vga_put_dec(conn->remote_port);
    debug_print(" (simulated).\n");

    // Copy data to send buffer (simulated flow control)
    uint32_t bytes_to_copy = (size < TCP_SEND_BUFFER_SIZE - conn->send_buffer_tail) ? size : (TCP_SEND_BUFFER_SIZE - conn->send_buffer_tail);
    memcpy(conn->send_buffer + conn->send_buffer_tail, data, bytes_to_copy);
    conn->send_buffer_tail += bytes_to_copy;

    // Simulate sending segment and starting retransmission timer
    conn->sequence_number += bytes_to_copy; // Update sequence number
    conn->retransmission_timer = TCP_RETRANSMISSION_TIMEOUT_MS; // Start timer

    return bytes_to_copy;
}

int tcp_receive(tcp_connection_t* conn, uint8_t* buffer, uint32_t buffer_size) {
    if (!conn || conn->state != TCP_STATE_ESTABLISHED) return -1;

    uint32_t bytes_available = (conn->receive_buffer_tail >= conn->receive_buffer_head) ? 
                                (conn->receive_buffer_tail - conn->receive_buffer_head) : 
                                (TCP_RECEIVE_BUFFER_SIZE - conn->receive_buffer_head + conn->receive_buffer_tail);

    uint32_t bytes_to_read = (bytes_available < buffer_size) ? bytes_available : buffer_size;

    if (bytes_to_read == 0) {
        return 0; // No data available
    }

    // Read from circular buffer
    for (uint32_t i = 0; i < bytes_to_read; i++) {
        buffer[i] = conn->receive_buffer[conn->receive_buffer_head];
        conn->receive_buffer_head = (conn->receive_buffer_head + 1) % TCP_RECEIVE_BUFFER_SIZE;
    }

    debug_print("TCP: Received ");
    vga_put_dec(bytes_to_read);
    debug_print(" bytes.\n");
    return bytes_to_read;
}

int tcp_close(tcp_connection_t* conn) {
    if (!conn) return -1;
    debug_print("TCP: Closing connection (simulated).\n");
    conn->state = TCP_STATE_FIN_WAIT_1; // Simulate FIN_WAIT_1
    // In a real implementation, this would send FIN packet.
    // For now, directly transition to CLOSED.
    conn->state = TCP_STATE_CLOSED;
    if (conn->receive_buffer) {
        kfree(conn->receive_buffer);
    }
    if (conn->send_buffer) {
        kfree(conn->send_buffer);
    }
    // Remove from connection list
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        if (tcp_connections[i] == conn) {
            tcp_connections[i] = NULL;
            break;
        }
    }
    kfree(conn);
    return 0;
}

// This function would be called by the IP layer when a TCP packet is received
void tcp_handle_ipv4_packet(ipv4_addr_t src_ip, uint8_t protocol, const uint8_t* data, uint32_t size) {
    if (protocol != 6) return; // Not a TCP packet

    // Simulate parsing TCP header
    uint16_t src_port = (data[0] << 8) | data[1];
    uint16_t dest_port = (data[2] << 8) | data[3];
    uint32_t sequence_number = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
    uint32_t acknowledge_number = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];
    uint8_t flags = data[13]; // Assuming flags are at offset 13
    const uint8_t* tcp_data = data + ((data[12] >> 4) * 4); // Data starts after header length
    uint32_t tcp_data_size = size - ((data[12] >> 4) * 4);

    debug_print("TCP: Received packet from ");
    vga_put_hex(src_ip);
    debug_print(":");
    vga_put_dec(src_port);
    debug_print(" to port ");
    vga_put_dec(dest_port);
    debug_print(" (Seq: ");
    vga_put_hex(sequence_number);
    debug_print(", Ack: ");
    vga_put_hex(acknowledge_number);
    debug_print(", Flags: ");
    vga_put_hex(flags);
    debug_print(", Data Size: ");
    vga_put_dec(tcp_data_size);
    debug_print(")\n");

    // Find matching connection
    tcp_connection_t* conn = NULL;
    for (int i = 0; i < TCP_MAX_CONNECTIONS; i++) {
        if (tcp_connections[i] && 
            tcp_connections[i]->local_port == dest_port &&
            tcp_connections[i]->remote_ip == src_ip &&
            tcp_connections[i]->remote_port == src_port) {
            conn = tcp_connections[i];
            break;
        }
    }

    if (conn) {
        // Simulate state machine transitions and data handling
        if (flags & 0x02) { // SYN flag
            if (conn->state == TCP_STATE_LISTEN) {
                conn->remote_ip = src_ip;
                conn->remote_port = src_port;
                conn->acknowledge_number = sequence_number + 1;
                conn->state = TCP_STATE_SYN_RECEIVED; // Simulate SYN_RECEIVED
                debug_print("TCP: SYN received, sending SYN-ACK (simulated).\n");
                // In a real implementation, send SYN-ACK.
            }
        } else if (flags & 0x10) { // ACK flag
            if (conn->state == TCP_STATE_SYN_SENT) {
                conn->acknowledge_number = sequence_number + 1;
                conn->state = TCP_STATE_ESTABLISHED; // Simulate ESTABLISHED
                debug_print("TCP: SYN-ACK received, connection established (simulated).\n");
            } else if (conn->state == TCP_STATE_ESTABLISHED) {
                // Handle data and update acknowledge number
                if (tcp_data_size > 0) {
                    // Write data to receive buffer
                    for (uint32_t i = 0; i < tcp_data_size; i++) {
                        if (((conn->receive_buffer_tail + 1) % TCP_RECEIVE_BUFFER_SIZE) != conn->receive_buffer_head) {
                            conn->receive_buffer[conn->receive_buffer_tail] = tcp_data[i];
                            conn->receive_buffer_tail = (conn->receive_buffer_tail + 1) % TCP_RECEIVE_BUFFER_SIZE;
                        }
                    }
                    conn->acknowledge_number += tcp_data_size;
                    debug_print("TCP: Data received, ACK sent (simulated).\n");
                    // In a real implementation, send ACK.
                }
            }
        }
    } else {
        debug_print("TCP: No matching connection found for received packet.\n");
    }
}
