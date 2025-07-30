#include "pci_driver.h"
#include "pci.h"
#include "../../kernel/vga.h"
#include "../../kernel/include/driver.h"

// Forward declarations for specific device drivers
extern void e1000_init(uint8_t bus, uint8_t device, uint8_t function);

// PCI driver structure
static driver_t pci_bus_driver = {
    .name = "PCI Bus Driver",
    .init = pci_driver_init,
    .probe = NULL // PCI is a bus driver, it doesn't probe itself
};

void pci_driver_init(void) {
    vga_puts("PCI Bus Driver initialized.\n");
    register_driver(&pci_bus_driver);
}

void pci_enumerate_devices(void) {
    vga_puts("PCI: Enumerating devices...\n");
    for (uint8_t bus = 0; bus < 255; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t vendor_id = pci_read_config_dword(bus, device, function, PCI_VENDOR_ID);
                if (vendor_id == 0xFFFF) continue; // Device doesn't exist

                uint32_t device_id = pci_read_config_dword(bus, device, function, PCI_DEVICE_ID);
                uint8_t class_code = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 24) & 0xFF);
                uint8_t subclass = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 16) & 0xFF);

                vga_puts("  PCI Device: Bus="); vga_put_hex(bus);
                vga_puts(", Device="); vga_put_hex(device);
                vga_puts(", Function="); vga_put_hex(function);
                vga_puts(", VendorID="); vga_put_hex(vendor_id);
                vga_puts(", DeviceID="); vga_put_hex(device_id);
                vga_puts(", Class="); vga_put_hex(class_code);
                vga_puts(", Subclass="); vga_put_hex(subclass);
                vga_puts("\n");

                // Check for specific device types and initialize their drivers
                if (class_code == 0x02 && subclass == 0x00) { // Ethernet controller
                    if (vendor_id == 0x8086 && device_id == 0x100E) { // Intel 82540EM (E1000)
                        vga_puts("    Found Intel E1000 Ethernet Controller. Initializing...\n");
                        e1000_init(bus, device, function);
                    }
                    // Add other Ethernet controllers here
                }
                // Add other PCI device types (e.g., USB, Audio, GPU) here
            }
        }
    }
    vga_puts("PCI: Device enumeration complete.\n");
}
