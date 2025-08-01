#ifndef NVME_H
#define NVME_H

#include "include/types.h"

// NVMe Controller Registers (simplified)
// These are typically memory-mapped, not I/O port mapped
typedef struct {
    uint32_t cap;   // Controller Capabilities
    uint32_t vs;    // Version
    uint32_t intms; // Interrupt Mask Set
    uint32_t intmc; // Interrupt Mask Clear
    uint32_t ccfg;  // Controller Configuration
    uint32_t csts;  // Controller Status
    uint32_t nssr;  // NVM Subsystem Reset
    uint32_t aqa;   // Admin Queue Attributes
    uint32_t asq;   // Admin Submission Queue Base Address
    uint32_t acq;   // Admin Completion Queue Base Address
    // ... many more registers
} nvme_registers_t;

// NVMe Command and Completion Queue Entry (simplified)
typedef struct {
    uint32_t cdw0;
    uint32_t cdw1;
    uint32_t cdw2;
    uint32_t cdw3;
    uint32_t mptr;
    uint32_t dptr[2];
    // ... many more fields
} nvme_sq_entry_t;

typedef struct {
    uint32_t cdw0;
    uint32_t cdw1;
    uint32_t cdw2;
    uint32_t cdw3;
} nvme_cq_entry_t;

// Initialize NVMe driver
void nvme_init(void);

#endif // NVME_H
