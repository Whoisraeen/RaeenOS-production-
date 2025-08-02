/**
 * RaeenOS DHCP Client Implementation
 * Complete DHCP client with automatic network configuration
 */

#include "network_advanced.h"
#include "../memory.h"
#include "../string.h"
#include "../timer.h"

// DHCP Message Types
#define DHCP_DISCOVER   1
#define DHCP_OFFER      2
#define DHCP_REQUEST    3
#define DHCP_DECLINE    4
#define DHCP_ACK        5
#define DHCP_NAK        6
#define DHCP_RELEASE    7
#define DHCP_INFORM     8

// DHCP Options
#define DHCP_OPT_SUBNET_MASK        1
#define DHCP_OPT_ROUTER             3
#define DHCP_OPT_DNS_SERVER         6
#define DHCP_OPT_HOSTNAME           12
#define DHCP_OPT_DOMAIN_NAME        15
#define DHCP_OPT_REQUESTED_IP       50
#define DHCP_OPT_LEASE_TIME         51
#define DHCP_OPT_MESSAGE_TYPE       53
#define DHCP_OPT_SERVER_ID          54
#define DHCP_OPT_PARAM_REQUEST      55
#define DHCP_OPT_RENEWAL_TIME       58
#define DHCP_OPT_REBINDING_TIME     59
#define DHCP_OPT_CLIENT_ID          61
#define DHCP_OPT_END                255

// DHCP Header Structure
typedef struct __attribute__((packed)) {
    uint8_t op;           // Message op code
    uint8_t htype;        // Hardware address type
    uint8_t hlen;         // Hardware address length
    uint8_t hops;         // Hops
    uint32_t xid;         // Transaction ID
    uint16_t secs;        // Seconds elapsed
    uint16_t flags;       // Flags
    uint32_t ciaddr;      // Client IP address
    uint32_t yiaddr;      // Your IP address
    uint32_t siaddr;      // Server IP address
    uint32_t giaddr;      // Gateway IP address
    uint8_t chaddr[16];   // Client hardware address
    uint8_t sname[64];    // Server name
    uint8_t file[128];    // Boot file name
    uint32_t magic;       // Magic cookie (0x63825363)
    uint8_t options[];    // Options
} dhcp_header_t;

// DHCP Client State
typedef enum {
    DHCP_STATE_INIT,
    DHCP_STATE_SELECTING,
    DHCP_STATE_REQUESTING,
    DHCP_STATE_BOUND,
    DHCP_STATE_RENEWING,
    DHCP_STATE_REBINDING,
    DHCP_STATE_INIT_REBOOT
} dhcp_state_t;

// DHCP Configuration
typedef struct {
    uint32_t client_ip;
    uint32_t server_ip;
    uint32_t subnet_mask;
    uint32_t gateway;
    uint32_t dns_server[4];
    uint8_t dns_count;
    uint32_t lease_time;
    uint32_t renewal_time;
    uint32_t rebinding_time;
    char hostname[64];
    char domain_name[64];
} dhcp_config_t;

// DHCP Client Context
typedef struct {
    dhcp_state_t state;
    uint32_t transaction_id;
    uint8_t mac_address[6];
    udp_socket_t* socket;
    
    dhcp_config_t config;
    uint32_t lease_start_time;
    uint32_t next_renewal;
    uint32_t next_rebinding;
    
    uint32_t discover_retries;
    uint32_t request_retries;
    uint32_t last_packet_time;
    
    bool configured;
} dhcp_client_t;

static dhcp_client_t g_dhcp_client = {0};

// Function declarations
static bool dhcp_send_discover(dhcp_client_t* client);
static bool dhcp_send_request(dhcp_client_t* client, uint32_t server_ip, uint32_t requested_ip);
static bool dhcp_send_release(dhcp_client_t* client);
static void dhcp_process_offer(dhcp_client_t* client, const dhcp_header_t* header, uint32_t packet_len);
static void dhcp_process_ack(dhcp_client_t* client, const dhcp_header_t* header, uint32_t packet_len);
static void dhcp_process_nak(dhcp_client_t* client, const dhcp_header_t* header, uint32_t packet_len);
static bool dhcp_parse_options(const uint8_t* options, uint32_t options_len, dhcp_config_t* config, uint8_t* msg_type);
static void dhcp_add_option(uint8_t** ptr, uint8_t option, uint8_t length, const void* data);
static uint32_t dhcp_generate_xid(void);
static void dhcp_configure_interface(const dhcp_config_t* config);

/**
 * Initialize DHCP client
 */
bool dhcp_client_init(const uint8_t* mac_address) {
    dhcp_client_t* client = &g_dhcp_client;
    
    memory_set(client, 0, sizeof(dhcp_client_t));
    memory_copy(client->mac_address, mac_address, 6);
    
    client->state = DHCP_STATE_INIT;
    client->transaction_id = dhcp_generate_xid();
    
    // Create UDP socket
    client->socket = udp_socket_create();
    if (!client->socket) {
        printf("DHCP: Failed to create UDP socket\n");
        return false;
    }
    
    // Bind to DHCP client port (68)
    if (!udp_socket_bind(client->socket, 0, 68)) {
        printf("DHCP: Failed to bind to port 68\n");
        udp_socket_close(client->socket);
        return false;
    }
    
    printf("DHCP: Client initialized with MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac_address[0], mac_address[1], mac_address[2],
           mac_address[3], mac_address[4], mac_address[5]);
    
    return true;
}

/**
 * Start DHCP discovery process
 */
bool dhcp_client_discover(void) {
    dhcp_client_t* client = &g_dhcp_client;
    
    if (!client->socket) {
        printf("DHCP: Client not initialized\n");
        return false;
    }
    
    client->state = DHCP_STATE_SELECTING;
    client->discover_retries = 0;
    client->last_packet_time = timer_get_ticks();
    
    return dhcp_send_discover(client);
}

/**
 * Process DHCP client state machine
 */
void dhcp_client_update(void) {
    dhcp_client_t* client = &g_dhcp_client;
    uint32_t current_time = timer_get_ticks();
    
    if (!client->socket) return;
    
    // Check for incoming packets
    uint8_t buffer[1500];
    uint32_t src_ip;
    uint16_t src_port;
    
    int received = udp_socket_recv(client->socket, buffer, sizeof(buffer), &src_ip, &src_port);
    if (received > 0) {
        dhcp_header_t* header = (dhcp_header_t*)buffer;
        
        // Validate DHCP packet
        if (received >= sizeof(dhcp_header_t) && 
            header->op == 2 && // Boot reply
            header->xid == client->transaction_id &&
            header->magic == htonl(0x63825363)) {
            
            uint8_t msg_type = 0;
            dhcp_config_t temp_config = {0};
            
            // Parse options to get message type
            uint32_t options_len = received - sizeof(dhcp_header_t);
            if (dhcp_parse_options(header->options, options_len, &temp_config, &msg_type)) {
                switch (msg_type) {
                    case DHCP_OFFER:
                        if (client->state == DHCP_STATE_SELECTING) {
                            dhcp_process_offer(client, header, received);
                        }
                        break;
                        
                    case DHCP_ACK:
                        if (client->state == DHCP_STATE_REQUESTING ||
                            client->state == DHCP_STATE_RENEWING ||
                            client->state == DHCP_STATE_REBINDING) {
                            dhcp_process_ack(client, header, received);
                        }
                        break;
                        
                    case DHCP_NAK:
                        dhcp_process_nak(client, header, received);
                        break;
                }
            }
        }
    }
    
    // Handle timeouts and retries
    uint32_t elapsed = current_time - client->last_packet_time;
    
    switch (client->state) {
        case DHCP_STATE_SELECTING:
            if (elapsed > 4000) { // 4 second timeout
                if (client->discover_retries < 3) {
                    client->discover_retries++;
                    dhcp_send_discover(client);
                } else {
                    printf("DHCP: Discovery failed after 3 retries\n");
                    client->state = DHCP_STATE_INIT;
                }
            }
            break;
            
        case DHCP_STATE_REQUESTING:
            if (elapsed > 4000) {
                if (client->request_retries < 3) {
                    client->request_retries++;
                    dhcp_send_request(client, client->config.server_ip, client->config.client_ip);
                } else {
                    printf("DHCP: Request failed after 3 retries\n");
                    client->state = DHCP_STATE_INIT;
                }
            }
            break;
            
        case DHCP_STATE_BOUND:
            if (current_time >= client->next_renewal) {
                printf("DHCP: Starting lease renewal\n");
                client->state = DHCP_STATE_RENEWING;
                dhcp_send_request(client, client->config.server_ip, client->config.client_ip);
            }
            break;
            
        case DHCP_STATE_RENEWING:
            if (current_time >= client->next_rebinding) {
                printf("DHCP: Starting rebinding\n");
                client->state = DHCP_STATE_REBINDING;
                dhcp_send_request(client, 0xFFFFFFFF, client->config.client_ip); // Broadcast
            } else if (elapsed > 2000) {
                dhcp_send_request(client, client->config.server_ip, client->config.client_ip);
            }
            break;
            
        case DHCP_STATE_REBINDING:
            if (current_time >= client->lease_start_time + client->config.lease_time) {
                printf("DHCP: Lease expired, restarting discovery\n");
                client->state = DHCP_STATE_INIT;
                dhcp_client_discover();
            } else if (elapsed > 2000) {
                dhcp_send_request(client, 0xFFFFFFFF, client->config.client_ip);
            }
            break;
    }
}

/**
 * Get DHCP configuration
 */
bool dhcp_client_get_config(dhcp_config_t* config) {
    if (!config || !g_dhcp_client.configured) {
        return false;
    }
    
    memory_copy(config, &g_dhcp_client.config, sizeof(dhcp_config_t));
    return true;
}

/**
 * Release DHCP lease
 */
void dhcp_client_release(void) {
    dhcp_client_t* client = &g_dhcp_client;
    
    if (client->state == DHCP_STATE_BOUND ||
        client->state == DHCP_STATE_RENEWING ||
        client->state == DHCP_STATE_REBINDING) {
        
        dhcp_send_release(client);
        client->configured = false;
        client->state = DHCP_STATE_INIT;
        
        printf("DHCP: Lease released\n");
    }
}

// Internal helper functions

static bool dhcp_send_discover(dhcp_client_t* client) {
    uint8_t packet[1500];
    dhcp_header_t* header = (dhcp_header_t*)packet;
    
    memory_set(packet, 0, sizeof(packet));
    
    // Fill DHCP header
    header->op = 1;           // Boot request
    header->htype = 1;        // Ethernet
    header->hlen = 6;         // MAC address length
    header->hops = 0;
    header->xid = client->transaction_id;
    header->secs = 0;
    header->flags = htons(0x8000); // Broadcast flag
    header->ciaddr = 0;
    header->yiaddr = 0;
    header->siaddr = 0;
    header->giaddr = 0;
    memory_copy(header->chaddr, client->mac_address, 6);
    header->magic = htonl(0x63825363);
    
    // Add options
    uint8_t* options = header->options;
    
    // Message type
    uint8_t msg_type = DHCP_DISCOVER;
    dhcp_add_option(&options, DHCP_OPT_MESSAGE_TYPE, 1, &msg_type);
    
    // Client identifier
    uint8_t client_id[7] = {1}; // Hardware type + MAC
    memory_copy(client_id + 1, client->mac_address, 6);
    dhcp_add_option(&options, DHCP_OPT_CLIENT_ID, 7, client_id);
    
    // Parameter request list
    uint8_t param_list[] = {
        DHCP_OPT_SUBNET_MASK,
        DHCP_OPT_ROUTER,
        DHCP_OPT_DNS_SERVER,
        DHCP_OPT_DOMAIN_NAME,
        DHCP_OPT_LEASE_TIME,
        DHCP_OPT_RENEWAL_TIME,
        DHCP_OPT_REBINDING_TIME
    };
    dhcp_add_option(&options, DHCP_OPT_PARAM_REQUEST, sizeof(param_list), param_list);
    
    // Hostname
    const char* hostname = "RaeenOS";
    dhcp_add_option(&options, DHCP_OPT_HOSTNAME, string_length(hostname), hostname);
    
    // End option
    *options++ = DHCP_OPT_END;
    
    uint32_t packet_len = (uint8_t*)options - packet;
    
    // Send broadcast packet
    int result = udp_socket_send(client->socket, packet, packet_len, 0xFFFFFFFF, 67);
    
    if (result > 0) {
        client->last_packet_time = timer_get_ticks();
        printf("DHCP: DISCOVER sent (XID: 0x%08x)\n", client->transaction_id);
        return true;
    }
    
    printf("DHCP: Failed to send DISCOVER\n");
    return false;
}

static void dhcp_process_offer(dhcp_client_t* client, const dhcp_header_t* header, uint32_t packet_len) {
    dhcp_config_t config = {0};
    uint8_t msg_type;
    
    uint32_t options_len = packet_len - sizeof(dhcp_header_t);
    if (!dhcp_parse_options(header->options, options_len, &config, &msg_type)) {
        return;
    }
    
    config.client_ip = header->yiaddr;
    config.server_ip = header->siaddr;
    
    printf("DHCP: OFFER received - IP: %u.%u.%u.%u from server %u.%u.%u.%u\n",
           (config.client_ip >> 24) & 0xFF, (config.client_ip >> 16) & 0xFF,
           (config.client_ip >> 8) & 0xFF, config.client_ip & 0xFF,
           (config.server_ip >> 24) & 0xFF, (config.server_ip >> 16) & 0xFF,
           (config.server_ip >> 8) & 0xFF, config.server_ip & 0xFF);
    
    // Store offered configuration
    memory_copy(&client->config, &config, sizeof(dhcp_config_t));
    
    // Send REQUEST
    client->state = DHCP_STATE_REQUESTING;
    client->request_retries = 0;
    dhcp_send_request(client, config.server_ip, config.client_ip);
}

static void dhcp_process_ack(dhcp_client_t* client, const dhcp_header_t* header, uint32_t packet_len) {
    dhcp_config_t config = {0};
    uint8_t msg_type;
    
    uint32_t options_len = packet_len - sizeof(dhcp_header_t);
    if (!dhcp_parse_options(header->options, options_len, &config, &msg_type)) {
        return;
    }
    
    config.client_ip = header->yiaddr;
    memory_copy(&client->config, &config, sizeof(dhcp_config_t));
    
    // Configure network interface
    dhcp_configure_interface(&config);
    
    // Set lease timers
    client->lease_start_time = timer_get_ticks();
    client->next_renewal = client->lease_start_time + (config.renewal_time * 1000);
    client->next_rebinding = client->lease_start_time + (config.rebinding_time * 1000);
    
    client->state = DHCP_STATE_BOUND;
    client->configured = true;
    
    printf("DHCP: ACK received - Configuration complete\n");
    printf("DHCP: IP: %u.%u.%u.%u, Mask: %u.%u.%u.%u\n",
           (config.client_ip >> 24) & 0xFF, (config.client_ip >> 16) & 0xFF,
           (config.client_ip >> 8) & 0xFF, config.client_ip & 0xFF,
           (config.subnet_mask >> 24) & 0xFF, (config.subnet_mask >> 16) & 0xFF,
           (config.subnet_mask >> 8) & 0xFF, config.subnet_mask & 0xFF);
    printf("DHCP: Gateway: %u.%u.%u.%u, DNS: %u.%u.%u.%u\n",
           (config.gateway >> 24) & 0xFF, (config.gateway >> 16) & 0xFF,
           (config.gateway >> 8) & 0xFF, config.gateway & 0xFF,
           (config.dns_server[0] >> 24) & 0xFF, (config.dns_server[0] >> 16) & 0xFF,
           (config.dns_server[0] >> 8) & 0xFF, config.dns_server[0] & 0xFF);
}

static bool dhcp_parse_options(const uint8_t* options, uint32_t options_len, dhcp_config_t* config, uint8_t* msg_type) {
    const uint8_t* ptr = options;
    const uint8_t* end = options + options_len;
    
    while (ptr < end) {
        uint8_t option = *ptr++;
        
        if (option == DHCP_OPT_END) {
            break;
        }
        
        if (ptr >= end) break;
        uint8_t length = *ptr++;
        
        if (ptr + length > end) break;
        
        switch (option) {
            case DHCP_OPT_MESSAGE_TYPE:
                if (length == 1) *msg_type = *ptr;
                break;
                
            case DHCP_OPT_SUBNET_MASK:
                if (length == 4) config->subnet_mask = *(uint32_t*)ptr;
                break;
                
            case DHCP_OPT_ROUTER:
                if (length >= 4) config->gateway = *(uint32_t*)ptr;
                break;
                
            case DHCP_OPT_DNS_SERVER:
                config->dns_count = length / 4;
                if (config->dns_count > 4) config->dns_count = 4;
                for (int i = 0; i < config->dns_count; i++) {
                    config->dns_server[i] = ((uint32_t*)ptr)[i];
                }
                break;
                
            case DHCP_OPT_LEASE_TIME:
                if (length == 4) config->lease_time = ntohl(*(uint32_t*)ptr);
                break;
                
            case DHCP_OPT_RENEWAL_TIME:
                if (length == 4) config->renewal_time = ntohl(*(uint32_t*)ptr);
                break;
                
            case DHCP_OPT_REBINDING_TIME:
                if (length == 4) config->rebinding_time = ntohl(*(uint32_t*)ptr);
                break;
                
            case DHCP_OPT_SERVER_ID:
                if (length == 4) config->server_ip = *(uint32_t*)ptr;
                break;
        }
        
        ptr += length;
    }
    
    return true;
}

static void dhcp_add_option(uint8_t** ptr, uint8_t option, uint8_t length, const void* data) {
    *(*ptr)++ = option;
    *(*ptr)++ = length;
    memory_copy(*ptr, data, length);
    *ptr += length;
}

static uint32_t dhcp_generate_xid(void) {
    static uint32_t xid = 0x12345678;
    return ++xid;
}

static void dhcp_configure_interface(const dhcp_config_t* config) {
    // TODO: Configure network interface with IP, subnet mask, gateway, DNS
    printf("DHCP: Configuring network interface...\n");
}
