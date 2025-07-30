#include "e1000.h"
#include "network.h"
#include "../pci/pci.h"
#include "../../kernel/vga.h"
#include "../../kernel/memory.h"
#include "../../kernel/include/driver.h"

// E1000 MMIO registers (simplified for illustration)
#define E1000_REG_CTRL      0x00000
#define E1000_REG_STATUS    0x00008
#define E1000_REG_EERD      0x00014
#define E1000_REG_ICR       0x000C0
#define E1000_REG_IMS       0x000D0
#define E1000_REG_RCTL      0x00100
#define E1000_REG_RDBAL     0x02800
#define E1000_REG_RDBAH     0x02804
#define E1000_REG_RDLEN     0x02808
#define E1000_REG_RDH       0x02810
#define E1000_REG_RDT       0x02818
#define E1000_REG_TCTL      0x00400
#define E1000_REG_TDBAL     0x03800
#define E1000_REG_TDBAH     0x03804
#define E1000_REG_TDLEN     0x03808
#define E1000_REG_TDH       0x03810
#define E1000_REG_TDT       0x03818

// E1000 descriptor structures
typedef struct {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) e1000_rx_desc_t;

typedef struct {
    uint64_t addr;
    uint16_t length;
    uint8_t cmd;
    uint8_t dtyp;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) e1000_tx_desc_t;

#define E1000_NUM_RX_DESC 128
#define E1000_NUM_TX_DESC 128

static volatile uint32_t* e1000_mmio_base = NULL;
static e1000_rx_desc_t* rx_descs = NULL;
static e1000_tx_desc_t* tx_descs = NULL;
static uint8_t* rx_buffers[E1000_NUM_RX_DESC];
static uint8_t* tx_buffers[E1000_NUM_TX_DESC];

// Helper function to read from MMIO register
static uint32_t e1000_read_reg(uint32_t reg) {
    return e1000_mmio_base[reg / 4];
}

// Helper function to write to MMIO register
static void e1000_write_reg(uint32_t reg, uint32_t val) {
    e1000_mmio_base[reg / 4] = val;
}

// E1000 driver initialization
static int e1000_driver_init(void) {
    vga_puts("E1000 driver initialized (placeholder).\n");
    return 0;
}

// E1000 driver probe function (called by PCI bus driver)
void e1000_init(uint8_t bus, uint8_t device, uint8_t function) {
    vga_puts("Probing E1000 device...\n");

    // Read BAR0 to get MMIO base address
    uint32_t bar0 = pci_read_config_dword(bus, device, function, PCI_BAR0);
    uint32_t mmio_phys_addr = bar0 & 0xFFFFFFF0; // Mask out flags

    // Map MMIO region (simplified, needs proper virtual memory mapping)
    // For now, assume identity mapping or direct access if kernel is high enough
    e1000_mmio_base = (volatile uint32_t*)mmio_phys_addr;

    // Enable bus mastering
    uint16_t command_reg = pci_read_config_word(bus, device, function, PCI_COMMAND);
    command_reg |= (1 << 2); // Set Bus Master Enable bit
    pci_write_config_word(bus, device, function, PCI_COMMAND, command_reg);

    // Perform a simple read to verify MMIO access
    uint32_t ctrl_reg_val = e1000_read_reg(E1000_REG_CTRL);
    vga_puts("E1000 CTRL register: ");
    vga_put_hex(ctrl_reg_val);
    vga_puts("\n");

    // Allocate Rx and Tx descriptor rings and buffers
    rx_descs = (e1000_rx_desc_t*)kmalloc(E1000_NUM_RX_DESC * sizeof(e1000_rx_desc_t));
    tx_descs = (e1000_tx_desc_t*)kmalloc(E1000_NUM_TX_DESC * sizeof(e1000_tx_desc_t));

    if (!rx_descs || !tx_descs) {
        vga_puts("Failed to allocate E1000 descriptor rings!\n");
        return;
    }

    // Allocate Rx/Tx buffers
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_buffers[i] = (uint8_t*)kmalloc(2048); // Typical buffer size
        if (!rx_buffers[i]) {
            vga_puts("Failed to allocate E1000 Rx buffer!\n");
            return;
        }
        rx_descs[i].addr = (uint64_t)rx_buffers[i];
    }

    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        tx_buffers[i] = (uint8_t*)kmalloc(2048); // Typical buffer size
        if (!tx_buffers[i]) {
            vga_puts("Failed to allocate E1000 Tx buffer!\n");
            return;
        }
        tx_descs[i].addr = (uint64_t)tx_buffers[i];
    }

    // Set up Rx/Tx descriptor base addresses and lengths
    e1000_write_reg(E1000_REG_RDBAL, (uint32_t)rx_descs);
    e1000_write_reg(E1000_REG_RDBAH, 0); // Assuming 32-bit physical addresses for now
    e1000_write_reg(E1000_REG_RDLEN, E1000_NUM_RX_DESC * sizeof(e1000_rx_desc_t));
    e1000_write_reg(E1000_REG_RDH, 0);
    e1000_write_reg(E1000_REG_RDT, E1000_NUM_RX_DESC - 1);

    e1000_write_reg(E1000_REG_TDBAL, (uint32_t)tx_descs);
    e1000_write_reg(E1000_REG_TDBAH, 0); // Assuming 32-bit physical addresses for now
    e1000_write_reg(E1000_REG_TDLEN, E1000_NUM_TX_DESC * sizeof(e1000_tx_desc_t));
    e1000_write_reg(E1000_REG_TDH, 0);
    e1000_write_reg(E1000_REG_TDT, 0);

    // Enable receiver and transmitter (simplified)
    e1000_write_reg(E1000_REG_RCTL, 0x00000002); // Enable receiver
    e1000_write_reg(E1000_REG_TCTL, 0x00000002); // Enable transmitter

    vga_puts("E1000 device initialized.\n");
}

// Send a packet using the E1000 driver
int e1000_send_packet(const uint8_t* data, uint32_t size) {
    if (!e1000_mmio_base || !tx_descs) return -1;

    uint32_t tdt = e1000_read_reg(E1000_REG_TDT);
    if (tdt >= E1000_NUM_TX_DESC) tdt = 0; // Wrap around

    // Copy data to the transmit buffer
    memcpy(tx_buffers[tdt], data, size);

    // Update descriptor
    tx_descs[tdt].length = size;
    tx_descs[tdt].cmd = 0x09; // EOP | RS
    tx_descs[tdt].status = 0; // Clear status

    // Advance TDT
    e1000_write_reg(E1000_REG_TDT, (tdt + 1) % E1000_NUM_TX_DESC);

    return 0;
}

// Receive a packet using the E1000 driver
int e1000_receive_packet(uint8_t* buffer, uint32_t buffer_size) {
    if (!e1000_mmio_base || !rx_descs) return -1;

    uint32_t rdh = e1000_read_reg(E1000_REG_RDH);
    uint32_t rdt = e1000_read_reg(E1000_REG_RDT);

    if (rdh == rdt) {
        return 0; // No packets received
    }

    uint32_t next_rdh = (rdh + 1) % E1000_NUM_RX_DESC;
    e1000_rx_desc_t* desc = &rx_descs[next_rdh];

    if (!(desc->status & 0x01)) { // Descriptor Done bit not set
        return 0; // Packet not ready
    }

    uint32_t packet_size = desc->length;
    if (packet_size > buffer_size) {
        packet_size = buffer_size; // Truncate if buffer is too small
    }

    memcpy(buffer, rx_buffers[next_rdh], packet_size);

    // Clear status and advance RDH
    desc->status = 0;
    e1000_write_reg(E1000_REG_RDH, next_rdh);

    return packet_size;
}

// Register the E1000 driver
static driver_t e1000_driver = {
    .name = "E1000 Network Driver",
    .init = e1000_driver_init,
    .probe = NULL // E1000 is probed by the generic network driver
};

void e1000_register(void) {
    register_driver(&e1000_driver);
}
