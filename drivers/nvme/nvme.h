#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the NVMe driver
void nvme_init(void);

// Read sectors from an NVMe drive
int nvme_read_sectors(uint8_t drive, uint64_t lba, uint32_t num_sectors, uint8_t* buffer);

// Write sectors to an NVMe drive
int nvme_write_sectors(uint8_t drive, uint64_t lba, uint32_t num_sectors, const uint8_t* buffer);

#endif // NVME_H