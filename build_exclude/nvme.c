#include "nvme.h"
#include "../drivers/pci/pci.h"
#include "../kernel/vga.h"
#include "../kernel/include/driver.h"

// NVMe driver structure
static driver_t nvme_driver = {
    .name = "NVMe Driver",
    .init = nvme_init,
    .probe = NULL // NVMe is not a bus driver
};

void nvme_init(void) {
    vga_puts("NVMe driver initialized (placeholder):\n");

    // Enumerate PCI devices to find NVMe controllers
    // NVMe controller class code is 0x01, subclass 0x08, prog_if 0x02
    for (uint8_t bus = 0; bus < 1; bus++) { // Limit bus for quick test
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint8_t class_code = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 24) & 0xFF);
                uint8_t subclass = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_CLASS) >> 16) & 0xFF);
                uint8_t prog_if = (uint8_t)((pci_read_config_dword(bus, device, function, PCI_PROG_IF) >> 8) & 0xFF);

                if (class_code == 0x01 && subclass == 0x08 && prog_if == 0x02) {
                    vga_puts("  Found NVMe Controller (Bus ");
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