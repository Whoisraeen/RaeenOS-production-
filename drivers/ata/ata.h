#ifndef ATA_H
#define ATA_H

#include "include/types.h"

// ATA PIO mode commands
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_IDENTIFY        0xEC

// ATA registers
#define ATA_REG_DATA            0x00
#define ATA_REG_ERROR           0x01
#define ATA_REG_FEATURES        0x01
#define ATA_REG_SECTOR_COUNT    0x02
#define ATA_REG_LBA_LOW         0x03
#define ATA_REG_LBA_MID         0x04
#define ATA_REG_LBA_HIGH        0x05
#define ATA_REG_DEVICE          0x06
#define ATA_REG_STATUS          0x07
#define ATA_REG_COMMAND         0x07

// ATA status bits
#define ATA_SR_BSY              0x80    // Busy
#define ATA_SR_DRDY             0x40    // Drive ready
#define ATA_SR_DF               0x20    // Device fault
#define ATA_SR_SRV              0x10    // Service
#define ATA_SR_DRQ              0x08    // Data request ready
#define ATA_SR_CORR             0x04    // Corrected data
#define ATA_SR_IDX              0x02    // Index
#define ATA_SR_ERR              0x01    // Error

// ATA error bits
#define ATA_ER_AMNF             0x01    // Address mark not found
#define ATA_ER_TK0NF            0x02    // Track 0 not found
#define ATA_ER_ABRT             0x04    // Aborted command
#define ATA_ER_MCR              0x08    // Media change request
#define ATA_ER_IDNF             0x10    // ID not found
#define ATA_ER_MC               0x20    // Media changed
#define ATA_ER_UNC              0x40    // Uncorrectable data
#define ATA_ER_BBK              0x80    // Bad block

// ATA drive types
#define ATA_MASTER              0x00
#define ATA_SLAVE               0x01

// ATA primary and secondary bus I/O ports
#define ATA_PRIMARY_IO          0x1F0
#define ATA_SECONDARY_IO        0x170

// ATA primary and secondary control ports
#define ATA_PRIMARY_DCR         0x3F6
#define ATA_SECONDARY_DCR       0x376

// Initialize ATA driver
void ata_init(void);

// Read sectors from ATA drive
int ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, uint16_t* buf);

// Write sectors to ATA drive
int ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t num_sectors, uint16_t* buf);

#endif // ATA_H
