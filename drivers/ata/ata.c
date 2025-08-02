#include "ata.h"
#include "../../kernel/vga.h"
#include "../../kernel/ports.h"
#include "../../kernel/string.h"
#include "../../kernel/include/driver.h"

// ATA I/O Ports
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE_SELECT 0x1F6
#define ATA_PRIMARY_COMMAND     0x1F7
#define ATA_PRIMARY_STATUS      0x1F7

// ATA Commands
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_IDENTIFY        0xEC

// ATA Status Register Bits
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive Ready
#define ATA_SR_DF      0x20    // Drive Write Fault
#define ATA_SR_ERR     0x01    // Error

// ATA driver structure
static driver_t ata_driver = {
    .name = "ATA Driver",
    .init = ata_init,
    .probe = NULL // Not a bus driver
};

static void ata_wait_busy(void) {
    while (inb(ATA_PRIMARY_STATUS) & ATA_SR_BSY);
}

static void ata_wait_drdy(void) {
    while (!(inb(ATA_PRIMARY_STATUS) & ATA_SR_DRDY));
}

void ata_init(void) {
    debug_print("ATA driver initialized (placeholder).\n");
    // Basic check for ATA presence
    outb(ATA_PRIMARY_DRIVE_SELECT, 0xA0); // Master drive
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status != 0xFF) { // 0xFF means no device
        debug_print("ATA Primary Master detected.\n");
    } else {
        debug_print("No ATA Primary Master detected.\n");
    }
}

int ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, uint8_t* buffer) {
    ata_wait_busy();
    outb(ATA_PRIMARY_DRIVE_SELECT, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECTOR_COUNT, num_sectors);
    outb(ATA_PRIMARY_LBA_LOW, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);

    for (int i = 0; i < num_sectors; i++) {
        ata_wait_drdy();
        for (int j = 0; j < 256; j++) { // 256 words per sector
            ((uint16_t*)buffer)[i * 256 + j] = inw(ATA_PRIMARY_DATA);
        }
    }
    debug_print("ATA: Read ");
    vga_put_dec(num_sectors);
    debug_print(" sectors from LBA ");
    vga_put_hex(lba);
    debug_print("\n");
    return 0;
}

int ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, const uint8_t* buffer) {
    ata_wait_busy();
    outb(ATA_PRIMARY_DRIVE_SELECT, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECTOR_COUNT, num_sectors);
    outb(ATA_PRIMARY_LBA_LOW, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);

    for (int i = 0; i < num_sectors; i++) {
        ata_wait_drdy();
        for (int j = 0; j < 256; j++) { // 256 words per sector
            outw(ATA_PRIMARY_DATA, ((uint16_t*)buffer)[i * 256 + j]);
        }
    }
    debug_print("ATA: Written ");
    vga_put_dec(num_sectors);
    debug_print(" sectors to LBA ");
    vga_put_hex(lba);
    debug_print("\n");
    return 0;
}