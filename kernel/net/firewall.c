#include "firewall.h"
#include "../vga.h"

void firewall_init(void) {
    vga_puts("Firewall initialized (placeholder).\n");
}

int firewall_add_rule(firewall_rule_t* rule) {
    (void)rule;
    vga_puts("Adding firewall rule (placeholder).\n");
    return 0; // Success
}

bool firewall_check_packet(uint32_t src_ip, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, uint8_t protocol) {
    (void)src_ip;
    (void)dest_ip;
    (void)src_port;
    (void)dest_port;
    (void)protocol;
    // For now, always allow
    return true;
}

