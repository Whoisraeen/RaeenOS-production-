#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include <stdbool.h>

// NVMe Command structure (simplified placeholder)
typedef struct {
    uint32_t opcode;
    uint32_t command_id;
    uint64_t lba;
    uint32_t num_sectors;
    uint64_t data_ptr;
    // More fields for metadata, PRP entries, etc.
} nvme_command_t;

// NVMe Completion Queue Entry (simplified placeholder)
typedef struct {
    uint32_t command_id;
    uint32_t status;
    // More fields for result, etc.
} nvme_completion_t;

// Initialize the NVMe driver
void nvme_init(void);

// Read sectors from an NVMe drive
int nvme_read_sectors(uint8_t drive, uint64_t lba, uint32_t num_sectors, uint8_t* buffer);

// Write sectors to an NVMe drive
int nvme_write_sectors(uint8_t drive, uint64_t lba, uint32_t num_sectors, const uint8_t* buffer);

// Submit an NVMe command (placeholder)
int nvme_submit_command(uint8_t drive, const nvme_command_t* cmd);

// Poll for NVMe completion (placeholder)
int nvme_poll_completion(uint8_t drive, nvme_completion_t* completion);

#endif // NVME_H
