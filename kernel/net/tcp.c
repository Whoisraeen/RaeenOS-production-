#include "tcp.h"
#include "../vga.h"
#include "../memory.h"

void tcp_init(void) {
    debug_print("TCP stack initialized (placeholder).\n");
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

    conn->local_ip = 0; // TODO: Get actual local IP
    conn->local_port = local_port;
    conn->remote_ip = remote_ip;
    conn->remote_port = remote_port;
    conn->state = TCP_STATE_SYN_SENT; // Simulate SYN_SENT

    return conn;
}

tcp_connection_t* tcp_listen(uint16_t port) {
    debug_print("TCP: Listening on port ");
    vga_put_dec(port);
    debug_print(" (simulated).\n");

    tcp_connection_t* conn = (tcp_connection_t*)kmalloc(sizeof(tcp_connection_t));
    if (!conn) return NULL;

    conn->local_ip = 0; // TODO: Get actual local IP
    conn->local_port = port;
    conn->remote_ip = 0;
    conn->remote_port = 0;
    conn->state = TCP_STATE_LISTEN;

    return conn;
}

int tcp_send(tcp_connection_t* conn, const uint8_t* data, uint32_t size) {
    if (!conn) return -1;
    debug_print("TCP: Sending ");
    vga_put_dec(size);
    debug_print(" bytes to ");
    vga_put_hex(conn->remote_ip);
    debug_print(":");
    vga_put_dec(conn->remote_port);
    debug_print(" (simulated).\n");
    // In a real implementation, this would segment data, add TCP header, and pass to IP layer.
    return size;
}

int tcp_receive(tcp_connection_t* conn, uint8_t* buffer, uint32_t buffer_size) {
    if (!conn) return -1;
    debug_print("TCP: Receiving data (simulated).\n");
    // In a real implementation, this would retrieve data from receive buffer.
    return 0; // Simulate no data received for now
}

int tcp_close(tcp_connection_t* conn) {
    if (!conn) return -1;
    debug_print("TCP: Closing connection (simulated).\n");
    conn->state = TCP_STATE_CLOSED;
    kfree(conn);
    return 0;
}
