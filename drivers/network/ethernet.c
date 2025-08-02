#include "ethernet.h"
#include "../../kernel/vga.h"
#include "../../kernel/include/driver.h"
#include "../../kernel/string.h"

// Placeholder MAC address
static uint8_t current_mac_address[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

// Ethernet driver structure
static driver_t ethernet_driver = {
    .name = "Generic Ethernet Driver",
    .init = ethernet_init,
    .probe = NULL // Not a bus driver
};

void ethernet_init(void) {
    debug_print("Ethernet driver initialized (placeholder).\n");
    debug_print("MAC Address: ");
    for (int i = 0; i < 6; i++) {
        vga_put_hex(current_mac_address[i]);
        if (i < 5) debug_print(":");
    }
    debug_print("\n");
}

int ethernet_send_packet(const uint8_t* data, uint32_t size) {
    (void)data;
    (void)size;
    debug_print("Ethernet: Sending packet (simulated).\n");
    return 0; // Success
}

int ethernet_receive_packet(uint8_t* buffer, uint32_t buffer_size) {
    (void)buffer;
    (void)buffer_size;
    debug_print("Ethernet: Receiving packet (simulated).\n");
    return 0; // Success
}

void ethernet_get_mac_address(uint8_t* mac_address) {
    if (mac_address) {
        memcpy(mac_address, current_mac_address, 6);
    }
}

