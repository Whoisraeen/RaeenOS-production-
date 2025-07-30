#include "ata.h"
#include "../../kernel/ports.h"
#include "../../kernel/vga.h"
#include "../../kernel/timer.h"
#include "../../kernel/include/driver.h"

// Driver structure for ATA
static driver_t ata_driver = {
    .name = "ATA Driver",
    .init = ata_init,
    .probe = NULL // ATA is not a bus driver, so no probe function
};

void ata_init(void) {
    // For now, just a placeholder. Real initialization would involve
    // probing for drives, identifying them, etc.
    vga_puts("ATA driver initialized (placeholder).\n");
    register_driver(&ata_driver);
}


int ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, uint16_t* buf) {
    uint16_t base_port = (drive == ATA_MASTER) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t control_port = (drive == ATA_MASTER) ? ATA_PRIMARY_DCR : ATA_SECONDARY_DCR;

    ata_wait_ready(base_port);

    outb(control_port, 0x00); // Disable interrupts

    outb(base_port + ATA_REG_FEATURES, 0x00); // PIO mode
    outb(base_port + ATA_REG_SECTOR_COUNT, num_sectors);
    outb(base_port + ATA_REG_LBA_LOW, (uint8_t)lba);
    outb(base_port + ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
    outb(base_port + ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(base_port + ATA_REG_DEVICE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(base_port + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    for (int i = 0; i < num_sectors; i++) {
        ata_wait_drq(base_port);
        for (int j = 0; j < 256; j++) { // 256 words per sector (512 bytes)
            buf[j] = inw(base_port + ATA_REG_DATA);
        }
        buf += 256;
    }

    return 0;
}

int ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, uint16_t* buf) {
    uint16_t base_port = (drive == ATA_MASTER) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
    uint16_t control_port = (drive == ATA_MASTER) ? ATA_PRIMARY_DCR : ATA_SECONDARY_DCR;

    ata_wait_ready(base_port);

    outb(control_port, 0x00); // Disable interrupts

    outb(base_port + ATA_REG_FEATURES, 0x00); // PIO mode
    outb(base_port + ATA_REG_SECTOR_COUNT, num_sectors);
    outb(base_port + ATA_REG_LBA_LOW, (uint8_t)lba);
    outb(base_port + ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
    outb(base_port + ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(base_port + ATA_REG_DEVICE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(base_port + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    for (int i = 0; i < num_sectors; i++) {
        ata_wait_drq(base_port);
        for (int j = 0; j < 256; j++) { // 256 words per sector (512 bytes)
            outw(base_port + ATA_REG_DATA, buf[j]);
        }
        buf += 256;
    }

    return 0;
}
