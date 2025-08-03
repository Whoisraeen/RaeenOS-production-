#include "network.h"
#include "../drivers/pci/pci.h"
#include "../kernel/vga.h"
#include "e1000.h" // Include E1000 driver header

nic_driver_type_t active_nic_driver = NIC_DRIVER_NONE;

void network_init(void) {
    vga_puts("Generic Network driver initialized (placeholder):\n");

    // Enumerate PCI devices to find network controllers
    // Network controller class code is 0x02 (Ethernet controller)
    // Subclass 0x00 (Ethernet controller)
    // Prog IF 0x00 (Ethernet controller)
    // Vendor ID for Intel is 0x8086
    // Device ID for E1000 (82540EM) is 0x100E

    bool found_nic = false;
    for (uint8_t bus = 0; bus < 255; bus++) { 
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t vendor_id = pci_read_config_dword(bus, device, function, PCI_VENDOR_ID);
                if (vendor_id == 0xFFFF) continue; // Device doesn't exist

                uint32_t device_id = pci_read_config_dword(bus, device, function, PCI_DEVICE_ID);
                uint8_t class_code = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 24) & 0xFF);
                uint8_t subclass = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 16) & 0xFF);

                if (class_code == 0x02 && subclass == 0x00) { // Ethernet controller
                    vga_puts("  Found Network Controller (Bus ");
                    vga_put_hex(bus);
                    vga_puts(", Device ");
                    vga_put_hex(device);
                    vga_puts(", Function ");
                    vga_put_hex(function);
                    vga_puts(")\n");
                    vga_puts("    Vendor ID: "); vga_put_hex(vendor_id); vga_puts("\n");
                    vga_puts("    Device ID: "); vga_put_hex(device_id); vga_puts("\n");
                    found_nic = true;

                    // Example: e1000_init(bus, device, function);
                    if (vendor_id == 0x8086 && device_id == 0x100E) { // Intel 82540EM (E1000)
                        vga_puts("    Initializing E1000 driver...\n");
                        e1000_init(bus, device, function);
                        active_nic_driver = NIC_DRIVER_E1000;
                        return; // For now, just find one and return
                    }
                    // Add other NIC initializations here
                    return; // For now, just find one and return
                }
            }
        }
    }

    if (!found_nic) {
        vga_puts("  No Ethernet controller found.\n");
    }
}

int network_send_packet(network_packet_t* packet) {
    switch (active_nic_driver) {
        case NIC_DRIVER_E1000:
            return e1000_send_packet(packet->data, packet->size);
        case NIC_DRIVER_NONE:
        default:
            vga_puts("No active NIC driver to send packet.\n");
            return -1;
    }
}

int network_receive_packet(network_packet_t* packet) {
    switch (active_nic_driver) {
        case NIC_DRIVER_E1000:
            return e1000_receive_packet(packet->data, packet->size);
        case NIC_DRIVER_NONE:
        default:
            return 0; // No packet received
    }
}


