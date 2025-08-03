#include "pci.h"
#include "../kernel/ports.h"
#include "../kernel/vga.h"

// Read a 32-bit value from PCI configuration space
uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunction << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

// Write a 32-bit value to PCI configuration space
void pci_write_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunction << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

// Read a 16-bit value from PCI configuration space
uint16_t pci_read_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t lbus  = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;
    uint32_t address;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunction << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    uint32_t data = inl(PCI_CONFIG_DATA);
    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

// Write a 16-bit value to PCI configuration space
void pci_write_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
    uint32_t lbus  = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;
    uint32_t address;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunction << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    uint32_t data = inl(PCI_CONFIG_DATA);
    uint32_t shift = (offset & 2) * 8;
    data = (data & ~(0xFFFF << shift)) | ((uint32_t)value << shift);
    outl(PCI_CONFIG_DATA, data);
}

// Check if a PCI device exists
static uint16_t pci_check_vendor(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t vendor_device = pci_read_config_dword(bus, device, function, PCI_VENDOR_ID);
    uint16_t vendor_id = (uint16_t)(vendor_device & 0xFFFF);
    if (vendor_id == 0xFFFF) return 0xFFFF; // Device does not exist
    return vendor_id;
}

// Enumerate PCI devices
void pci_init(void) {
    vga_puts("PCI Bus Enumeration:\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t device = 0; device < 32; device++) {
            for (uint16_t function = 0; function < 8; function++) {
                if (pci_check_vendor(bus, device, function) != 0xFFFF) {
                    uint32_t class_rev = pci_read_config_dword(bus, device, function, PCI_REVISION_ID);
                    uint8_t class_code = (uint8_t)((class_rev >> 24) & 0xFF);
                    uint8_t subclass = (uint8_t)((class_rev >> 16) & 0xFF);
                    uint16_t vendor_id = (uint16_t)(pci_read_config_dword(bus, device, function, PCI_VENDOR_ID) & 0xFFFF);
                    uint16_t device_id = (uint16_t)((pci_read_config_dword(bus, device, function, PCI_VENDOR_ID) >> 16) & 0xFFFF);

                    vga_puts("  Found PCI device: Bus ");
                    vga_put_hex(bus);
                    vga_puts(", Device ");
                    vga_put_hex(device);
                    vga_puts(", Function ");
                    vga_put_hex(function);
                    vga_puts(" - Vendor ID: ");
                    vga_put_hex(vendor_id);
                    vga_puts(", Device ID: ");
                    vga_put_hex(device_id);
                    vga_puts(", Class: ");
                    vga_put_hex(class_code);
                    vga_puts(", Subclass: ");
                    vga_put_hex(subclass);
                    vga_puts("\n");
                }
            }
        }
    }
}
