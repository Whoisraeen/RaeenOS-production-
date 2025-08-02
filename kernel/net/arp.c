#include "arp.h"
#include "../vga.h"
#include "../libs/libc/include/string.h"
#include "../../drivers/network/ethernet.h"

// Placeholder ARP cache
typedef struct {
    ipv4_addr_t ip_address;
    uint8_t mac_address[6];
    uint32_t timestamp; // For cache aging
} arp_cache_entry_t;

static arp_cache_entry_t arp_cache[16]; // Small cache for now
static uint32_t arp_cache_count = 0;

void arp_init(void) {
    debug_print("ARP module initialized (placeholder).\n");
    memset(arp_cache, 0, sizeof(arp_cache));
}

bool arp_resolve(ipv4_addr_t ip_address, uint8_t* mac_address) {
    debug_print("ARP: Resolving IP ");
    vga_put_hex(ip_address);
    debug_print(" to MAC (simulated).\n");

    // Check cache first
    for (uint32_t i = 0; i < arp_cache_count; i++) {
        if (arp_cache[i].ip_address == ip_address) {
            memcpy(mac_address, arp_cache[i].mac_address, 6);
            debug_print("ARP: Found in cache.\n");
            return true;
        }
    }

    // Simulate ARP request (broadcast)
    // In a real implementation, this would send an ARP request packet
    // and wait for a reply.
    debug_print("ARP: Sending ARP request (simulated).\n");

    // For now, just return a dummy MAC address for any request
    uint8_t dummy_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    memcpy(mac_address, dummy_mac, 6);

    // Add to cache (placeholder for actual reply handling)
    if (arp_cache_count < 16) {
        arp_cache[arp_cache_count].ip_address = ip_address;
        memcpy(arp_cache[arp_cache_count].mac_address, dummy_mac, 6);
        arp_cache[arp_cache_count].timestamp = 0; // TODO: Use actual time
        arp_cache_count++;
    }

    return true;
}

void arp_handle_packet(const uint8_t* packet, uint32_t size) {
    debug_print("ARP: Received packet (simulated).\n");
    // In a real implementation, this would parse the ARP packet
    // and update the ARP cache or respond to requests.
    (void)packet;
    (void)size;
}
