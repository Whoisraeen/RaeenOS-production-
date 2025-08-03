/**
 * RaeenOS Advanced Network Stack Header
 * Comprehensive networking API with TCP, UDP, DHCP, DNS support
 */

#ifndef NETWORK_ADVANCED_H
#define NETWORK_ADVANCED_H

#include "../kernel/include/types.h"

// Network byte order conversion
#define htons(x) ((uint16_t)((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF)))
#define ntohs(x) htons(x)
#define htonl(x) ((uint32_t)((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8) | (((x) >> 24) & 0xFF)))
#define ntohl(x) htonl(x)

// IP Protocol Numbers
#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17

// Forward declarations
typedef struct tcp_socket tcp_socket_t;
typedef struct udp_socket udp_socket_t;
typedef struct dhcp_config dhcp_config_t;

// Network utility functions
uint32_t inet_addr(const char* ip_str);
char* inet_ntoa(uint32_t ip);

// IP Layer Functions
int ip_send_packet(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, const void* data, uint16_t length);
void ip_process_packet(const void* packet, uint32_t packet_len);

// TCP Socket API
tcp_socket_t* tcp_socket_create(void);
bool tcp_socket_bind(tcp_socket_t* socket, uint32_t ip, uint16_t port);
bool tcp_socket_listen(tcp_socket_t* socket, int backlog);
tcp_socket_t* tcp_socket_accept(tcp_socket_t* socket);
bool tcp_socket_connect(tcp_socket_t* socket, uint32_t ip, uint16_t port);
int tcp_socket_send(tcp_socket_t* socket, const void* data, uint32_t size);
int tcp_socket_recv(tcp_socket_t* socket, void* buffer, uint32_t size);
void tcp_socket_close(tcp_socket_t* socket);
void tcp_process_packet(const void* packet, uint32_t packet_len, uint32_t src_ip, uint32_t dest_ip);

// UDP Socket API
udp_socket_t* udp_socket_create(void);
bool udp_socket_bind(udp_socket_t* socket, uint32_t ip, uint16_t port);
bool udp_socket_connect(udp_socket_t* socket, uint32_t ip, uint16_t port);
int udp_socket_send(udp_socket_t* socket, const void* data, uint32_t size, uint32_t dest_ip, uint16_t dest_port);
int udp_socket_recv(udp_socket_t* socket, void* buffer, uint32_t size, uint32_t* src_ip, uint16_t* src_port);
void udp_socket_close(udp_socket_t* socket);
void udp_process_packet(const void* packet, uint32_t packet_len, uint32_t src_ip, uint32_t dest_ip);

// DHCP Client API
bool dhcp_client_init(const uint8_t* mac_address);
bool dhcp_client_discover(void);
void dhcp_client_update(void);
bool dhcp_client_get_config(dhcp_config_t* config);
void dhcp_client_release(void);

// DNS Resolver API
bool dns_resolver_init(const uint32_t* dns_servers, uint8_t server_count);
uint32_t dns_resolve(const char* hostname);
bool dns_add_server(uint32_t server_ip);
void dns_clear_cache(void);
void dns_get_stats(uint32_t* queries, uint32_t* responses, uint32_t* cache_hits, uint32_t* timeouts);

// Network Stack Initialization
void network_stack_init(void);
void network_stack_update(void);

#endif // NETWORK_ADVANCED_H
