#include "usb.h"
#include "../pci/pci.h"
#include "../../kernel/vga.h"
#include "../../kernel/include/driver.h"

// USB driver structure
static driver_t usb_driver = {
    .name = "USB Host Controller Driver",
    .init = usb_init,
    .probe = NULL // USB is not a bus driver
};

void usb_init(void) {
    vga_puts("USB Host Controller Initialization (placeholder):\n");

    // Enumerate PCI devices to find USB host controllers
    // This is a simplified example and would need proper PCI enumeration
    // and device identification based on class/subclass codes.
    for (uint8_t bus = 0; bus < 1; bus++) { // Limit bus for quick test
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint8_t class_code = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 24) & 0xFF);
                uint8_t subclass = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 16) & 0xFF);

                // USB Host Controller class code is 0x0C, subclass 0x03
                if (class_code == 0x0C && subclass == 0x03) {
                    uint8_t prog_if = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_PROG_IF) >> 8) & 0xFF);
                    vga_puts("  Found USB Host Controller: ");
                    if (prog_if == 0x00) {
                        vga_puts("UHCI");
                    } else if (prog_if == 0x10) {
                        vga_puts("OHCI");
                    } else if (prog_if == 0x20) {
                        vga_puts("EHCI");
                    } else if (prog_if == 0x30) {
                        vga_puts("XHCI");
                    } else {
                        vga_puts("Unknown");
                    }
                    vga_puts(" (Bus ");
                    vga_put_hex(bus);
                    vga_puts(", Device ");
                    vga_put_hex(device);
                    vga_puts(", Function ");
                    vga_put_hex(function);
                    vga_puts(")\n");
                }
            }
        }
    }
}