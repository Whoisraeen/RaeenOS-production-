#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

// ACPI RSDP (Root System Description Pointer) structure
#pragma pack(push, 1)
typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} acpi_rsdp_t;
#pragma pack(pop)

// ACPI SDT Header (System Description Table)
#pragma pack(push, 1)
typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_sdt_header_t;
#pragma pack(pop)

// Initialize ACPI driver
void acpi_init(void);

// Set power state (placeholder)
void acpi_set_power_state(uint8_t state);

#endif // ACPI_H
