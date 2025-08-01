// RaeenOS ACPI (Advanced Configuration and Power Interface) Support
// --------------------------------------------------------------

#ifndef ACPI_H
#define ACPI_H

#include "include/types.h"
// Using types.h for kernel build

// ACPI table signatures
#define ACPI_RSDP_SIGNATURE     0x2052545020445352ULL  // "RSD PTR "
#define ACPI_RSDT_SIGNATURE     0x54445352  // "RSDT"
#define ACPI_XSDT_SIGNATURE     0x54445358  // "XSDT"
#define ACPI_MADT_SIGNATURE     0x43495041  // "APIC"
#define ACPI_FADT_SIGNATURE     0x50434146  // "FACP"
#define ACPI_HPET_SIGNATURE     0x54455048  // "HPET"
#define ACPI_MCFG_SIGNATURE     0x4746434D  // "MCFG"

// RSDP structure for ACPI 1.0
typedef struct __attribute__((packed)) {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} rsdp_descriptor_t;

// RSDP structure for ACPI 2.0+
typedef struct __attribute__((packed)) {
    rsdp_descriptor_t rsdp1;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} rsdp_descriptor_2_t;

// Standard ACPI table header
typedef struct __attribute__((packed)) {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_sdt_header_t;

// Root System Description Table
typedef struct __attribute__((packed)) {
    acpi_sdt_header_t header;
    uint32_t sdt_pointers[];
} rsdt_t;

// Extended System Description Table
typedef struct __attribute__((packed)) {
    acpi_sdt_header_t header;
    uint64_t sdt_pointers[];
} xsdt_t;

// MADT (Multiple APIC Description Table) entry types
#define MADT_TYPE_LOCAL_APIC            0
#define MADT_TYPE_IO_APIC               1
#define MADT_TYPE_INTERRUPT_OVERRIDE    2
#define MADT_TYPE_NMI_SOURCE            3
#define MADT_TYPE_LOCAL_APIC_NMI        4
#define MADT_TYPE_LOCAL_APIC_OVERRIDE   5
#define MADT_TYPE_IO_SAPIC              6
#define MADT_TYPE_LOCAL_SAPIC           7
#define MADT_TYPE_PLATFORM_INTERRUPT    8
#define MADT_TYPE_LOCAL_X2APIC          9
#define MADT_TYPE_LOCAL_X2APIC_NMI      10

// MADT table structure
typedef struct __attribute__((packed)) {
    acpi_sdt_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
    uint8_t entries[];
} madt_t;

// MADT entry header
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t length;
} madt_entry_header_t;

// Local APIC entry
typedef struct __attribute__((packed)) {
    madt_entry_header_t header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} madt_local_apic_t;

// I/O APIC entry
typedef struct __attribute__((packed)) {
    madt_entry_header_t header;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address;
    uint32_t global_system_interrupt_base;
} madt_io_apic_t;

// CPU information structure
typedef struct {
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
    int is_enabled;
} cpu_info_t;

// I/O APIC information
typedef struct {
    uint8_t io_apic_id;
    uint32_t io_apic_address;
    uint32_t global_interrupt_base;
} io_apic_info_t;

// System information gathered from ACPI
typedef struct {
    int cpu_count;
    cpu_info_t cpus[256];  // Support up to 256 CPUs
    int io_apic_count;
    io_apic_info_t io_apics[16];  // Support up to 16 I/O APICs
    uint32_t local_apic_address;
    int acpi_version;
} acpi_system_info_t;

// Function declarations
int acpi_init(void);
rsdp_descriptor_t* acpi_find_rsdp(void);
int acpi_validate_checksum(void* table, size_t length);
acpi_sdt_header_t* acpi_find_table(uint32_t signature);
int acpi_parse_madt(void);
acpi_system_info_t* acpi_get_system_info(void);
void acpi_print_system_info(void);

// Power management functions
void acpi_shutdown(void);
void acpi_reboot(void);

#endif // ACPI_H