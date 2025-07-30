#ifndef FIREWALL_H
#define FIREWALL_H

#include <stdint.h>
#include <stdbool.h>

// Firewall rule action
typedef enum {
    FIREWALL_ACTION_ALLOW,
    FIREWALL_ACTION_DENY
} firewall_action_t;

// Firewall rule structure (simplified)
typedef struct {
    firewall_action_t action;
    uint32_t src_ip; // 0 for any
    uint32_t dest_ip; // 0 for any
    uint16_t src_port; // 0 for any
    uint16_t dest_port; // 0 for any
    uint8_t protocol; // 0 for any
} firewall_rule_t;

// Initialize the firewall
void firewall_init(void);

// Add a firewall rule
int firewall_add_rule(firewall_rule_t* rule);

// Check if a packet is allowed by firewall rules
bool firewall_check_packet(uint32_t src_ip, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, uint8_t protocol);

#endif // FIREWALL_H
