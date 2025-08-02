/**
 * RaeenOS UDP Protocol Implementation
 * Complete UDP stack with socket management and packet handling
 */

#include "network_advanced.h"
#include "../memory.h"
#include "../string.h"

// UDP Header Structure
typedef struct __attribute__((packed)) {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} udp_header_t;

// UDP Socket Structure
typedef struct udp_socket {
    uint16_t local_port;
    uint32_t local_ip;
    uint16_t remote_port;
    uint32_t remote_ip;
    bool bound;
    bool connected;
    
    // Receive buffer
    uint8_t* recv_buffer;
    uint32_t recv_buffer_size;
    uint32_t recv_data_size;
    uint32_t recv_head;
    uint32_t recv_tail;
    
    // Socket options
    bool broadcast;
    bool reuse_addr;
    uint32_t recv_timeout;
    uint32_t send_timeout;
    
    struct udp_socket* next;
} udp_socket_t;

// UDP Statistics
typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t packets_dropped;
    uint32_t checksum_errors;
    uint32_t port_unreachable;
    uint32_t buffer_overflows;
} udp_stats_t;

// Global UDP state
static udp_socket_t* g_udp_sockets = NULL;
static uint16_t g_next_ephemeral_port = 32768;
static udp_stats_t g_udp_stats = {0};

// Function declarations
static uint16_t udp_checksum(const udp_header_t* header, const void* data, 
                            uint16_t data_len, uint32_t src_ip, uint32_t dest_ip);
static udp_socket_t* udp_find_socket(uint16_t port, uint32_t ip);
static uint16_t udp_allocate_port(void);
static bool udp_validate_packet(const udp_header_t* header, uint16_t packet_len);

/**
 * Initialize UDP stack
 */
void udp_init(void) {
    g_udp_sockets = NULL;
    g_next_ephemeral_port = 32768;
    memory_set(&g_udp_stats, 0, sizeof(udp_stats_t));
    
    printf("UDP: Protocol stack initialized\n");
}

/**
 * Create UDP socket
 */
udp_socket_t* udp_socket_create(void) {
    udp_socket_t* socket = (udp_socket_t*)memory_alloc(sizeof(udp_socket_t));
    if (!socket) {
        return NULL;
    }
    
    memory_set(socket, 0, sizeof(udp_socket_t));
    
    // Allocate receive buffer (64KB default)
    socket->recv_buffer_size = 65536;
    socket->recv_buffer = (uint8_t*)memory_alloc(socket->recv_buffer_size);
    if (!socket->recv_buffer) {
        memory_free(socket);
        return NULL;
    }
    
    // Set default options
    socket->recv_timeout = 5000; // 5 seconds
    socket->send_timeout = 5000;
    
    // Add to socket list
    socket->next = g_udp_sockets;
    g_udp_sockets = socket;
    
    printf("UDP: Socket created\n");
    return socket;
}

/**
 * Bind UDP socket to address
 */
bool udp_socket_bind(udp_socket_t* socket, uint32_t ip, uint16_t port) {
    if (!socket || socket->bound) {
        return false;
    }
    
    // Check if port is already in use
    if (port != 0 && udp_find_socket(port, ip)) {
        printf("UDP: Port %u already in use\n", port);
        return false;
    }
    
    // Allocate ephemeral port if needed
    if (port == 0) {
        port = udp_allocate_port();
        if (port == 0) {
            printf("UDP: Failed to allocate ephemeral port\n");
            return false;
        }
    }
    
    socket->local_ip = ip;
    socket->local_port = port;
    socket->bound = true;
    
    printf("UDP: Socket bound to %u.%u.%u.%u:%u\n",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF, ip & 0xFF, port);
    
    return true;
}

/**
 * Connect UDP socket (sets default destination)
 */
bool udp_socket_connect(udp_socket_t* socket, uint32_t ip, uint16_t port) {
    if (!socket) {
        return false;
    }
    
    socket->remote_ip = ip;
    socket->remote_port = port;
    socket->connected = true;
    
    printf("UDP: Socket connected to %u.%u.%u.%u:%u\n",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF, ip & 0xFF, port);
    
    return true;
}

/**
 * Send UDP packet
 */
int udp_socket_send(udp_socket_t* socket, const void* data, uint32_t size,
                   uint32_t dest_ip, uint16_t dest_port) {
    if (!socket || !data || size == 0) {
        return -1;
    }
    
    // Use connected destination if not specified
    if (dest_ip == 0 && socket->connected) {
        dest_ip = socket->remote_ip;
        dest_port = socket->remote_port;
    }
    
    if (dest_ip == 0 || dest_port == 0) {
        return -1;
    }
    
    // Ensure socket is bound
    if (!socket->bound) {
        if (!udp_socket_bind(socket, 0, 0)) {
            return -1;
        }
    }
    
    // Create UDP packet
    uint16_t total_len = sizeof(udp_header_t) + size;
    uint8_t* packet = (uint8_t*)memory_alloc(total_len);
    if (!packet) {
        return -1;
    }
    
    udp_header_t* header = (udp_header_t*)packet;
    header->source_port = htons(socket->local_port);
    header->dest_port = htons(dest_port);
    header->length = htons(total_len);
    header->checksum = 0; // Calculate later
    
    // Copy data
    memory_copy(packet + sizeof(udp_header_t), data, size);
    
    // Calculate checksum
    header->checksum = udp_checksum(header, data, size, socket->local_ip, dest_ip);
    
    // Send via IP layer
    int result = ip_send_packet(socket->local_ip, dest_ip, IP_PROTOCOL_UDP, packet, total_len);
    
    memory_free(packet);
    
    if (result > 0) {
        g_udp_stats.packets_sent++;
        return size;
    }
    
    return -1;
}

/**
 * Receive UDP packet
 */
int udp_socket_recv(udp_socket_t* socket, void* buffer, uint32_t size,
                   uint32_t* src_ip, uint16_t* src_port) {
    if (!socket || !buffer || size == 0) {
        return -1;
    }
    
    uint32_t timeout = socket->recv_timeout;
    
    while (timeout > 0) {
        // Check if data available
        if (socket->recv_data_size > 0) {
            // Calculate available data
            uint32_t available = socket->recv_data_size;
            if (available > size) {
                available = size;
            }
            
            // Copy data from circular buffer
            uint32_t bytes_copied = 0;
            while (bytes_copied < available && socket->recv_head != socket->recv_tail) {
                ((uint8_t*)buffer)[bytes_copied] = socket->recv_buffer[socket->recv_head];
                socket->recv_head = (socket->recv_head + 1) % socket->recv_buffer_size;
                bytes_copied++;
            }
            
            socket->recv_data_size -= bytes_copied;
            
            // TODO: Extract source IP/port from packet metadata
            if (src_ip) *src_ip = 0;
            if (src_port) *src_port = 0;
            
            return bytes_copied;
        }
        
        // Wait for data
        timeout--;
        // TODO: Implement proper blocking/event mechanism
        for (volatile int i = 0; i < 1000; i++);
    }
    
    return 0; // Timeout
}

/**
 * Close UDP socket
 */
void udp_socket_close(udp_socket_t* socket) {
    if (!socket) return;
    
    // Remove from socket list
    if (g_udp_sockets == socket) {
        g_udp_sockets = socket->next;
    } else {
        udp_socket_t* current = g_udp_sockets;
        while (current && current->next != socket) {
            current = current->next;
        }
        if (current) {
            current->next = socket->next;
        }
    }
    
    // Free resources
    if (socket->recv_buffer) {
        memory_free(socket->recv_buffer);
    }
    memory_free(socket);
    
    printf("UDP: Socket closed\n");
}

/**
 * Process incoming UDP packet
 */
void udp_process_packet(const void* packet, uint32_t packet_len,
                       uint32_t src_ip, uint32_t dest_ip) {
    if (!packet || packet_len < sizeof(udp_header_t)) {
        g_udp_stats.packets_dropped++;
        return;
    }
    
    const udp_header_t* header = (const udp_header_t*)packet;
    uint16_t src_port = ntohs(header->source_port);
    uint16_t dest_port = ntohs(header->dest_port);
    uint16_t length = ntohs(header->length);
    
    // Validate packet
    if (!udp_validate_packet(header, packet_len)) {
        g_udp_stats.packets_dropped++;
        return;
    }
    
    // Verify checksum
    uint16_t received_checksum = header->checksum;
    uint16_t calculated_checksum = udp_checksum(header, 
        (const uint8_t*)packet + sizeof(udp_header_t),
        length - sizeof(udp_header_t), src_ip, dest_ip);
    
    if (received_checksum != 0 && received_checksum != calculated_checksum) {
        g_udp_stats.checksum_errors++;
        return;
    }
    
    // Find destination socket
    udp_socket_t* socket = udp_find_socket(dest_port, dest_ip);
    if (!socket) {
        g_udp_stats.port_unreachable++;
        // TODO: Send ICMP port unreachable
        return;
    }
    
    // Copy data to socket buffer
    uint32_t data_len = length - sizeof(udp_header_t);
    const uint8_t* data = (const uint8_t*)packet + sizeof(udp_header_t);
    
    // Check buffer space
    uint32_t available_space = socket->recv_buffer_size - socket->recv_data_size;
    if (data_len > available_space) {
        g_udp_stats.buffer_overflows++;
        return;
    }
    
    // Copy to circular buffer
    for (uint32_t i = 0; i < data_len; i++) {
        socket->recv_buffer[socket->recv_tail] = data[i];
        socket->recv_tail = (socket->recv_tail + 1) % socket->recv_buffer_size;
    }
    
    socket->recv_data_size += data_len;
    g_udp_stats.packets_received++;
}

/**
 * Get UDP statistics
 */
void udp_get_stats(udp_stats_t* stats) {
    if (stats) {
        memory_copy(stats, &g_udp_stats, sizeof(udp_stats_t));
    }
}

// Internal helper functions

static uint16_t udp_checksum(const udp_header_t* header, const void* data,
                            uint16_t data_len, uint32_t src_ip, uint32_t dest_ip) {
    // UDP uses IP pseudo-header for checksum calculation
    struct {
        uint32_t src_ip;
        uint32_t dest_ip;
        uint8_t zero;
        uint8_t protocol;
        uint16_t udp_length;
    } __attribute__((packed)) pseudo_header;
    
    pseudo_header.src_ip = src_ip;
    pseudo_header.dest_ip = dest_ip;
    pseudo_header.zero = 0;
    pseudo_header.protocol = IP_PROTOCOL_UDP;
    pseudo_header.udp_length = header->length;
    
    uint32_t sum = 0;
    
    // Checksum pseudo-header
    const uint16_t* pseudo = (const uint16_t*)&pseudo_header;
    for (int i = 0; i < sizeof(pseudo_header) / 2; i++) {
        sum += ntohs(pseudo[i]);
    }
    
    // Checksum UDP header (excluding checksum field)
    sum += ntohs(header->source_port);
    sum += ntohs(header->dest_port);
    sum += ntohs(header->length);
    
    // Checksum data
    const uint16_t* data_words = (const uint16_t*)data;
    for (int i = 0; i < data_len / 2; i++) {
        sum += ntohs(data_words[i]);
    }
    
    // Handle odd byte
    if (data_len & 1) {
        sum += ((const uint8_t*)data)[data_len - 1] << 8;
    }
    
    // Fold carry bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return htons(~sum);
}

static udp_socket_t* udp_find_socket(uint16_t port, uint32_t ip) {
    udp_socket_t* socket = g_udp_sockets;
    
    while (socket) {
        if (socket->local_port == port && 
            (socket->local_ip == 0 || socket->local_ip == ip)) {
            return socket;
        }
        socket = socket->next;
    }
    
    return NULL;
}

static uint16_t udp_allocate_port(void) {
    for (int attempts = 0; attempts < 1000; attempts++) {
        uint16_t port = g_next_ephemeral_port++;
        if (g_next_ephemeral_port > 65535) {
            g_next_ephemeral_port = 32768;
        }
        
        if (!udp_find_socket(port, 0)) {
            return port;
        }
    }
    
    return 0; // Failed to allocate
}

static bool udp_validate_packet(const udp_header_t* header, uint16_t packet_len) {
    uint16_t length = ntohs(header->length);
    
    if (length < sizeof(udp_header_t)) {
        return false;
    }
    
    if (length > packet_len) {
        return false;
    }
    
    return true;
}
