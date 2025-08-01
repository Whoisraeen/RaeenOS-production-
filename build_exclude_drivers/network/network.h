#ifndef NETWORK_H
#define NETWORK_H

#include "../../kernel/include/types.h"

// Network packet structure (simplified)
typedef struct {
    uint8_t* data;
    uint32_t size;
} network_packet_t;

typedef enum {
    NIC_DRIVER_NONE,
    NIC_DRIVER_E1000,
    // Add other NIC driver types here
} nic_driver_type_t;

extern nic_driver_type_t active_nic_driver;

// Initialize generic network driver
void network_init(void);

// Send a network packet
int network_send_packet(network_packet_t* packet);

// Receive a network packet
int network_receive_packet(network_packet_t* packet);

#endif // NETWORK_H
