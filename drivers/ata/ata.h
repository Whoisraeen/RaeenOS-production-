#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the ATA driver
void ata_init(void);

// Read sectors from an ATA drive
int ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, uint8_t* buffer);

// Write sectors to an ATA drive
int ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, const uint8_t* buffer);

#endif // ATA_H